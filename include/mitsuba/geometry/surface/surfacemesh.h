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

    using Index = uint32_t;
    using Storage = DynamicBuffer<Index>; // currently not differentiable

    template <typename Index>
    MTS_INLINE auto face_indices(Storage polygons, Index index, mask_t<Index> active = true) const {
        using Result = Array<replace_scalar_t<Index, uint32_t>, 3>;
        return gather<Result>(polygons, index, active);
    }

    SurfaceMesh(){};
    template<typename Array_>
    SurfaceMesh(const Array_& polygons)
    {
        using Face = Array<Index, 3>;

        // now assume that all input polyons are triangle
        n_faces_count = slices(polygons);

        // counting the number of vertices
        n_vertices_count += scalar_cast(hmax_nested(polygons));
        n_vertices_count++;

        vhalfedge = empty<Storage>(n_vertices_count);
        fhalfedge = empty<Storage>(n_faces_count);
        ehalfedge = empty<Storage>(n_faces_count*3);

        n_vertices_capacity_count = n_vertices_count;
        n_vertices_fill_count = n_vertices_count;

        n_faces_capacity_count = n_faces_count;
        n_faces_fill_count = n_faces_count;

        // allocation ??
        m_henext = empty<Storage>(n_faces_count*3);
        m_heface = empty<Storage>(n_faces_count*3);
        m_hevertex = empty<Storage>(n_faces_count*3);

        // construct primal
        for(Index cur_face=0; cur_face<n_faces_count; cur_face++)
        {
            Face polygon = slice(polygons, cur_face);

            Index face_degree = polygon.size();
            Index prev_he_idx = 0;
            Index first_he_idx = 0;
            for(Index cur_index=0; cur_index<face_degree; cur_index++)
            {
                Index tail = slice(polygon, cur_index);
                // Index tip  = slice(polygon, (cur_index+1)%face_degree);

                // auto get = get_new_halfedge();
                Index heidx = get_new_halfedge().get_index();

                m_henext[heidx] = 1999;
                m_heface[heidx] = cur_face;
                m_hevertex[heidx] = tail;

                vhalfedge[tail] = heidx;
                if(cur_index == 0) {
                    fhalfedge[cur_face] = heidx;
                    first_he_idx = heidx;
                } else {
                    m_henext[prev_he_idx] = heidx;
                }
                prev_he_idx = heidx;
            }
            m_henext[prev_he_idx] = first_he_idx;
        }



        // construct twins
        m_heedge = empty<Storage>(n_faces_count*3);
        m_hesibling = empty<Storage>(n_faces_count*3);

        std::unordered_map<std::tuple<Index, Index>, Index> edge_history;
        Index ihe = 0;
        for(Index cur_face=0; cur_face<n_faces_count; cur_face++)
        {
            Face polygon = slice(polygons, cur_face);
            
            Index face_degree = polygon.size();
            for(Index cur_index=0; cur_index<face_degree; cur_index++)
            {
                Index tail = slice(polygon, cur_index);
                Index tip  = slice(polygon, (cur_index + 1)%face_degree);

                std::tuple<Index, Index> ekey{std::min(tail, tip), std::max(tail, tip)};

                if(edge_history.find(ekey)==edge_history.end())
                {
                    Index new_edge_idx = get_new_edge().get_index();
                    m_heedge[ihe] = new_edge_idx;
                    m_hesibling[ihe] = 1999;
                    ehalfedge[new_edge_idx] = ihe;
                    edge_history[ekey] = ihe;
                } else { 
                    Index iprevhe = edge_history[ekey];
                    m_hesibling[ihe] = iprevhe;
                    Index ie = m_heedge[iprevhe];
                    m_heedge[ihe] = ie;
                    edge_history[ekey] = ihe;
                }
                ihe++;
            }
        }

        for(auto& entry : edge_history)
        {
            Index lasthe = entry.second;

            if(m_hesibling[lasthe]==1999) {
                m_hesibling[lasthe] = lasthe;
                continue;
            }

            Index cur_he = lasthe;
            while (m_hesibling[cur_he]!=1999)
            {
                cur_he = m_hesibling[cur_he];
            }
            m_hesibling[cur_he] = lasthe;
        }
        std::cout << "v.halfedge() " << vhalfedge << std::endl;
        std::cout << "f.halfedge() " << fhalfedge << std::endl;
        std::cout << "e.halfedge() " << ehalfedge << std::endl;

        std::cout << "he.next() " << m_henext << std::endl;
        std::cout << "he.twin() " << m_hesibling << std::endl;
        std::cout << "he.vertex() " << m_hevertex << std::endl;
        std::cout << "he.face() " << m_heface << std::endl;
        std::cout << "he.edge() " << m_heedge << std::endl;

        std::cout << m_hesibling.begin() << std::endl;
    }

    void initialize_halfedge_neighbors() {

    }


    Halfedge get_new_halfedge() {

        n_halfedges_fill_count++;
        n_halfedges_count++;

        return Halfedge(this, n_halfedges_fill_count-1);
    }

    Edge get_new_edge() {

        n_edges_fill_count++;
        n_edges_count++;

        return Edge(this, n_edges_fill_count-1);
    }

    MTS_INLINE Index henext(size_t he)        const { return m_henext[he]; }
    MTS_INLINE Index hetwin(size_t he)        const { return m_hetwin[he]; }
    MTS_INLINE Index hesibling(size_t he)     const { return m_hesibling[he]; }
    MTS_INLINE Index hevertex(size_t he)      const { return m_henext[he]; }
    MTS_INLINE Index heface(size_t he)        const { return m_heface[he]; }
    MTS_INLINE Index heedge(size_t he)        const { return m_heedge[he]; }

protected:
    Index n_halfedges_count = 0;
    Index n_halfedges_capacity_count = 0;
    Index n_halfedges_fill_count = 0;
   
    Index n_vertices_count = 0;
    Index n_vertices_capacity_count = 0;
    Index n_vertices_fill_count = 0;

    Index n_faces_count = 0;
    Index n_faces_capacity_count = 0;
    Index n_faces_fill_count = 0;

    Index n_edges_count = 0;
    Index n_edges_capacity_count = 0;
    Index n_edges_fill_count = 0;

    Storage m_henext;
    Storage m_hetwin;
    Storage m_hesibling;
    Storage m_hevertex;
    Storage m_heface;
    Storage m_heedge;

    Storage vhalfedge;
    Storage fhalfedge;
    Storage ehalfedge;
};

// MTS_EXTERN_CLASS_GEOMETRY(SurfaceMesh)
NAMESPACE_END(geometry)
NAMESPACE_END(mitsuba)