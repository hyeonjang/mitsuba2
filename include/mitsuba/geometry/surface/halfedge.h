#pragma once

#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/util/element.h>

namespace mitsuba {
namespace geometry {

template<typename Float>
struct HalfedgeStorage
{
    MTS_GEOMETRY_IMPORT_TYPES();
    using Bool = bool_array_t<Index>;

    Index next;
    Index twin;
    Index vertex;
    Index face;
    Index edge;
    Bool  orient; 

    ENOKI_STRUCT(HalfedgeStorage, next, twin, vertex, face, edge, orient);
};

}
}
ENOKI_STRUCT_SUPPORT(mitsuba::geometry::HalfedgeStorage, next, twin, vertex, face, edge, orient);

namespace mitsuba {
namespace geometry {

template<typename Float>
struct Halfedge : public Element<Float, Halfedge<Float>, SurfaceMesh<Float>>
{
    using Base = Element<Float, Halfedge<Float>, SurfaceMesh<Float>>;
    using Index = typename Base::Index;

    using Vertex = typename Base::Vertex;
    using Edge   = typename Base::Edge;
    using Face   = typename Base::Face;

    Halfedge(){};
    Halfedge(SurfaceMesh<Float>* mesh, Index index):Base(mesh, index){};

    MTS_INLINE Halfedge next() const { return Halfedge(this->m_mesh, this->m_mesh->henext(this->m_index)); };
    MTS_INLINE Halfedge twin() const { return Halfedge(this->m_mesh, this->m_mesh->hetwin(this->m_index)); };
    MTS_INLINE Vertex   vertex() const { return Vertex(this->m_mesh, this->m_mesh->hevertex(this->m_index)); }
    MTS_INLINE Vertex   headvertex() const { return next().vertex(); }
    MTS_INLINE Vertex   tailvertex() const { return vertex(); }
    MTS_INLINE Edge     edge() const;
    MTS_INLINE Face     face() const;
};

template<typename Float>
struct Vertex : public Element<Float, Vertex<Float>, SurfaceMesh<Float>>
{
    using Base = Element<Float, Vertex<Float>, SurfaceMesh<Float>>;
    using Index = typename Base::Index;

    Vertex(){};
    Vertex(SurfaceMesh<Float>* mesh, Index index):Base(mesh, index){};

    Halfedge<Float> halfedge(Index idx) const { return Halfedge(this->m_mesh, this->m_mesh->vhalfedge(idx)); };


    std::string t
};

template<typename Float>
struct Face : public Element<Float, Face<Float>, SurfaceMesh<Float>>
{
    using Base = Element<Float, Face<Float>, SurfaceMesh<Float>>;
    using Index = typename Base::Index;

    Face(){};
    Face(SurfaceMesh<Float>* mesh, Index index):Base(mesh, index){};

    Halfedge<Float> halfedge(Index idx) const { return Halfedge(this->m_mesh, this->m_mesh->fhalfedge(idx)); };
};

template<typename Float>
struct Edge : Element<Float, Edge<Float>, SurfaceMesh<Float>>
{
    using Base = Element<Float, Edge<Float>, SurfaceMesh<Float>>;
    using Index = typename Base::Index;

    Edge(){};
    Edge(SurfaceMesh<Float>* mesh, size_t index):Base(mesh, index){};

    Halfedge<Float> halfedge(Index idx) const { return Halfedge(this->m_mesh, this->m_mesh->ehalfedge(idx)); }

};


/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba //
////////////////////////////////////