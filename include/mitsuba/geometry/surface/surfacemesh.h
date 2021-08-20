#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/surface/halfedge.h>
#include <mitsuba/geometry/util/hash_function.h>

#include <unordered_map>
#include <unordered_set>

NAMESPACE_BEGIN(mitsuba)
NAMESPACE_BEGIN(geometry)

template<typename Float>
class MTS_EXPORT_GEOMETRY SurfaceMesh : public Object {
public:
    MTS_GEOMETRY_IMPORT_TYPES()
    using ScalarSize = scalar_t<Index>;
    using Storage = IStorage;
    using HeStorage = HalfedgeStorage<Float>; // more thinking

    template <typename Index>
    MTS_INLINE auto face_indices(const Storage& polygons, Index index, mask_t<Index> active = true) const {
        using Result = Array<replace_scalar_t<Index, size_t>, 3>;
        return gather<Result>(polygons, index, active);
    }

    SurfaceMesh(const Storage& polygons)
    {
        // now assume that all input polyons are triangle
        n_edges_count = n_halfedges_count = slices(polygons);
        n_faces_count = n_edges_count/n_face_degree;

        // counting the number of vertices
        n_vertices_count += scalar_cast(hmax_nested(polygons));
        n_vertices_count++;

        he = empty<HeStorage>(n_halfedges_count);

        vhalfedges = empty<Storage>(n_vertices_count);
        fhalfedges = empty<Storage>(n_faces_count);
        ehalfedges = empty<Storage>(n_edges_count);


        for(ScalarSize j=0; j<n_faces_count; j++)
        {
            size_t idx = j*n_face_degree;
            Index he_idx  = get_new_halfedge(n_face_degree).get_index();

            // not clear
            Index he_next = (gather<Index>(he_idx, arange<Index>(n_face_degree))+1)%n_face_degree+n_face_degree*j;
            Index he_vert = gather<Index>(polygons, he_idx);
            Index he_face = he_idx/3;//;gather<Index>(he_idx, he_idx)/n_face_degree;

            auto he_ptr = slice_ptr(he, idx);
            scatter(he_ptr.next,   he_next, arange<Index>(n_face_degree));
            scatter(he_ptr.vertex, he_vert, arange<Index>(n_face_degree));
            scatter(he_ptr.face,   he_face, arange<Index>(n_face_degree));

            scatter(vhalfedges, he_idx, he_vert);
            scatter(fhalfedges, he_idx, he_face);
        }

        if constexpr (is_cuda_array_v<Float>)
            cuda_eval();


        std::unordered_map<std::tuple<Index, Index>, Index> edge_history;
        // Index ihe = 0;
        for(ScalarSize j=0; j<n_faces_count; j++)
        {
            for(ScalarSize i=0; i<n_face_degree; i++)
            {
                
            }
        }


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

    void initialize_halfedge_neighbors() {

    }


    Halfedge get_new_halfedge(size_t size=1) {

        size_t start = n_halfedges_fill_count;
        size_t end   = start + size;

        n_halfedges_fill_count = end;
        n_halfedges_count = end;

        return Halfedge(this, arange<Index>(start, end));
    }

    Edge get_new_edge() {

        n_edges_fill_count++;
        n_edges_count++;

        return Edge(this, n_edges_fill_count-1);
    }

    MTS_INLINE Index henext(Index he)        const { return he.next[he]; }
    MTS_INLINE Index hetwin(Index he)        const { return he.twin[he]; }
    MTS_INLINE Index hevertex(Index he)      const { return he.next[he]; }
    MTS_INLINE Index heface(Index he)        const { return he.face[he]; }
    MTS_INLINE Index heedge(Index he)        const { return he.edge[he]; }

    MTS_INLINE Index vhalfedge(Index idx)   const { return gather<Index>(vhalfedges, idx); }
    MTS_INLINE Index fhalfedge(Index idx)   const { return gather<Index>(fhalfedges, idx); }
    MTS_INLINE Index ehalfedge(Index idx)   const { return gather<Index>(ehalfedges, idx); }

    MTS_INLINE auto vertices()  { return Vertex(this, he.vertex); }
    MTS_INLINE auto edges()     { return Edge(this, he.edge); }


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

    Storage vhalfedges;
    Storage fhalfedges;
    Storage ehalfedges;

public:
    ENOKI_STRUCT(SurfaceMesh, he, vhalfedges, fhalfedges, ehalfedges);
};

// MTS_EXTERN_CLASS_GEOMETRY(SurfaceMesh)
NAMESPACE_END(geometry)
NAMESPACE_END(mitsuba)