#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/surface/halfedge.h>
#include <mitsuba/geometry/util/halfedge_storage.h>
#include <mitsuba/geometry/util/hash_function.h>

#include <unordered_map>
#include <unordered_set>

NAMESPACE_BEGIN(mitsuba)
NAMESPACE_BEGIN(geometry)

template<typename Float>
class MTS_EXPORT_GEOMETRY SurfaceMesh : public Object {
public:
    MTS_GEOMETRY_IMPORT_TYPES(SurfaceMeshPtr, Halfedge, Vertex, Edge, Face)

    using ScalarSize = scalar_t<Index>;
    using HeStorage  = HalfedgeStorage<Float>; // more thinking


    // core
    SurfaceMesh(const IStorage& polygons){
        static_assert(!is_dynamic_array_v<HeStorage>, "Not a dynamic array");

        // now assume that all input polyons are triangle
        n_edges_count = n_halfedges_count = slices(polygons);
        n_faces_count = n_edges_count/n_face_degree;

        // counting the number of vertices
        n_vertices_count += scalar_cast(hmax_nested(polygons));
        n_vertices_count++;

        // memory allocation to member variables
        he = empty<HeStorage>(n_halfedges_count);
        vhalfedges = empty<IStorage>(n_vertices_count);
        fhalfedges = empty<IStorage>(n_faces_count);
        ehalfedges = empty<IStorage>(n_edges_count);

        set_halfedge_main(polygons);
        set_halfedge_twin(polygons);

        // slice(he, 0) = HeStorage(Storage(0), Storage(0), Storage(0), Storage(0), Storage(0));
        std::cout << "v.halfedge() " << vhalfedges << std::endl;
        std::cout << "f.halfedge() " << fhalfedges << std::endl;
        std::cout << "e.halfedge() " << ehalfedges << std::endl;

        std::cout << "he.next() "   << he.next << std::endl;
        std::cout << "he.twin() "   << he.twin << std::endl;
        std::cout << "he.vertex() " << he.vertex << std::endl;
        std::cout << "he.face() "   << he.face << std::endl;
        std::cout << "he.edge() "   << he.edge << std::endl;
    }

    MTS_INLINE void set_halfedge_main(const IStorage& polygons) {

        for(ScalarSize j=0; j<n_faces_count; j++) 
        {
            Index he_idx  = new_halfedge(n_face_degree).get_index(); // danger

            // very unclear
            Index he_next = (gather<Index>(he_idx, arange<Index>(n_face_degree))+1)%n_face_degree+n_face_degree*j;
            Index he_vert = gather<Index>(polygons, he_idx);
            Index he_face = he_idx/3;//;gather<Index>(he_idx, he_idx)/n_face_degree;

            size_t idx = j*n_face_degree;
            auto he_ptr = slice_ptr(he, idx);
            
            scatter(he_ptr.next,   he_next, arange<Index>(n_face_degree));
            scatter(he_ptr.vertex, he_vert, arange<Index>(n_face_degree));
            scatter(he_ptr.face,   he_face, arange<Index>(n_face_degree));
            scatter(vhalfedges, he_idx, he_vert);
            scatter(fhalfedges, he_idx, he_face);
        }

        if constexpr (is_cuda_array_v<Float>) cuda_eval();
    }

    MTS_INLINE void set_halfedge_twin(const IStorage& polygons) {

        using Bool = bool_array_t<Index>;
        using IndexSet = std::pair<Index, Index>;

        Index ihe = 0;
        auto he_ptr = slice_ptr(he, 0);
        std::unordered_map<IndexSet, Index> edge_history;

        const Index INVALID = Index(std::numeric_limits<value_t<Index>>::max()); // todo forwarding
        for(size_t j=0; j<n_faces_count; j++) 
        {
            Index tail = gather<Index>(polygons, arange<Index>(j*n_face_degree, (j+1)*n_face_degree));
            Index head = gather<Index>(polygons, 
                (arange<Index>(j*n_face_degree, (j+1)*n_face_degree)+1)%n_face_degree+n_face_degree*j);

            for(size_t i=0; i<n_face_degree; i++)
            {
                auto itail = slice(tail, i);
                auto ihead = slice(head, i);

                IndexSet key {min(itail, ihead), max(itail, ihead)};

                if(edge_history.find(key)==edge_history.end())
                {
                    Index edge = new_edge().get_index();

                    scatter(he_ptr.edge, edge, ihe);
                    scatter(he_ptr.twin, INVALID, ihe);
                    scatter(he_ptr.orient, Bool(true), ihe);

                    scatter(ehalfedges, ihe, edge);

                    edge_history[key] = ihe;

                } else {
                    Index prev_he = edge_history[key];
                    Index i_edge = gather<Index>(he_ptr.edge, prev_he);

                    scatter(he_ptr.edge, i_edge, ihe);
                    scatter(he_ptr.twin, prev_he, ihe);
                    scatter(he_ptr.orient, Bool(true), ihe);

                    edge_history[key] = ihe;

                }
                ihe += Index(1);
            }
        }

        std::cout << he.twin << std::endl;

        // set halfedge twin based on edge information

        for(auto& entry : edge_history)
        {
            Index last = entry.second;

            if (gather<Index>(he.twin, last)==INVALID) {
                scatter(he_ptr.twin, last, last);

                std::cout << gather<Index>(he.twin, last) << std::endl;
                std::cout << "last: " << last << std::endl;

                continue;
            }

            Index curr = last;
            std::cout << "curr0: " << curr << std::endl;
            while(gather<Index>(he.twin, curr)!=INVALID) {
                curr = gather<Index>(he.twin, curr);

                std::cout << "curr1: " << curr << std::endl;
            }
            std::cout << "c: " << curr << std::endl;
            std::cout << "l: " << last << std::endl;
            scatter(he_ptr.twin, last, curr);

        }
        if constexpr (is_cuda_array_v<Float>) cuda_eval();
    }

    void initialize_halfedge_neighbors() {

    }


    Halfedge new_halfedge(size_t size=1) {

        size_t start = n_halfedges_fill_count;
        size_t end   = start + size;

        n_halfedges_fill_count = end;
        n_halfedges_count = end;

        return Halfedge(this, arange<Index>(start, end));
    }

    Edge new_edge(size_t size=1) {

        size_t start = n_edges_fill_count;
        size_t end   = start + size;

        n_edges_fill_count = end;
        n_edges_count = end;

        return Edge(this, arange<Index>(start, end));
    }

    // todo
    MTS_INLINE Index henext(const Index& index) const { return get(he.next, index); }
    MTS_INLINE Index hetwin(const Index& index)   const { return get(he.twin, index); }
    MTS_INLINE Index hevertex(const Index& index) const { return get(he.vertex, index); }
    MTS_INLINE Index heface(const Index& index)   const { return get(he.face, index); }
    MTS_INLINE Index heedge(const Index& index)   const { return get(he.edge, index); }

    MTS_INLINE Index vhalfedge(const Index& idx)  const { return get(vhalfedges, idx); }
    MTS_INLINE Index fhalfedge(const Index& idx)  const { return get(fhalfedges, idx); }
    MTS_INLINE Index ehalfedge(const Index& idx)  const { return get(ehalfedges, idx); }

    MTS_INLINE auto halfedges() { return Halfedge(this, indices()); }
    MTS_INLINE auto vertices()  { return Vertex(this, he.vertex); }
    MTS_INLINE auto faces()     { return Face(this,     he.face); }
    MTS_INLINE auto edges()     { return Edge(this,     he.edge); }

    MTS_INLINE auto halfedge(const Index& index)  { return Halfedge(this,  get(indices(), index)); }
    MTS_INLINE auto vertex(const Index& index)    { return Vertex(this,    get(he.vertex, index)); }
    MTS_INLINE auto face(const Index& index)      { return Face(this,      get(he.face, index)); }
    MTS_INLINE auto edge(const Index& index)      { return Edge(this,      get(he.edge, index)); }

    MTS_INLINE size_t size()    const { return n_halfedges_count; }
    MTS_INLINE Index  indices() const { return arange<Index>(size()); }
    template<typename T> MTS_INLINE Index get(const T& arr, const Index& idx) const { return gather<Index>(arr, idx); } 

    void delete_element();
    
    void compress_halfedge();
    void compress_vertices();
    void compress_faces();
    void compress_edges();

    std::string to_string() const {
        std::ostringstream oss;
        oss << "SurfaceMesh" << std::endl;
        return oss.str();
    }

protected:
    size_t n_face_degree = 3; // more

    size_t n_halfedges_count = 0;
    size_t n_halfedges_capacity_count = 0;
    size_t n_halfedges_fill_count = 0;
   
    size_t n_vertices_count = 0;
    size_t n_vertices_capacity_count = 0;
    size_t n_vertices_fill_count = 0;

    size_t n_faces_count = 0;
    size_t n_faces_capacity_count = 0;
    size_t n_faces_fill_count = 0;

    size_t n_edges_count = 0;
    size_t n_edges_capacity_count = 0;
    size_t n_edges_fill_count = 0;

    HeStorage he;

    IStorage vhalfedges;
    IStorage fhalfedges;
    IStorage ehalfedges;

public:
    ENOKI_CALL_SUPPORT_FRIEND()
    ENOKI_STRUCT(SurfaceMesh, he, vhalfedges, fhalfedges, ehalfedges);
};

// MTS_EXTERN_CLASS_GEOMETRY(SurfaceMesh)
NAMESPACE_END(geometry)
NAMESPACE_END(mitsuba)

// enoki support vectorize
// ENOKI_CALL_SUPPORT_TEMPLATE_BEGIN(mitsuba::geometry::SurfaceMesh)
//     ENOKI_CALL_SUPPORT_METHOD(henext)
//     ENOKI_CALL_SUPPORT_METHOD(hetwin)
//     ENOKI_CALL_SUPPORT_METHOD(hevertex)
//     ENOKI_CALL_SUPPORT_METHOD(heface)
//     ENOKI_CALL_SUPPORT_METHOD(heedge)
//     ENOKI_CALL_SUPPORT_METHOD(vhalfedge)
//     ENOKI_CALL_SUPPORT_METHOD(fhalfedge)
//     ENOKI_CALL_SUPPORT_METHOD(ehalfedge)
// ENOKI_CALL_SUPPORT_TEMPLATE_END(mitsuba::geometry::SurfaceMesh)