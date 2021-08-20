#pragma once

#include <mitsuba/core/stream.h>
#include <mitsuba/core/vector.h>
#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/util/element.h>
#include <mitsuba/geometry/surface/surfacemesh.h>

namespace mitsuba {
namespace geometry {

template<typename Float>
struct Halfedge : public Element<Float>
{
    MTS_GEOMETRY_IMPORT_TYPES(Vertex, Face, Edge)
    MTS_GEOMETRY_IMPORT_BASE(Element)

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

template <typename Float>
std::ostream &operator<<(std::ostream &os, const Halfedge<Float> &h)
{
    os << "Halfedge: " << h.get_index();
    return os;
}

template<typename Float>
struct Vertex : public Element<Float>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Face, Edge)
    MTS_GEOMETRY_IMPORT_BASE(Element)

    Vertex(){};
    Vertex(SurfaceMesh<Float>* mesh, Index index):Base(mesh, index){};

    Halfedge halfedge() const { return Halfedge(this->m_mesh, this->m_mesh->vhalfedge(this->m_index)); };
};

template<typename Float> 
std::ostream &operator<<(std::ostream &os, const Vertex<Float> &v) 
{
    os << "Vertex: " << v.get_index();
    return os;
}

template<typename Float>
struct Face : public Element<Float>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Vertex, Edge)
    MTS_GEOMETRY_IMPORT_BASE(Element);

    Face(){};
    Face(SurfaceMesh<Float>* mesh, Index index):Base(mesh, index){};

    Halfedge halfedge() const { return Halfedge(this->m_mesh, this->m_mesh->fhalfedge(this->m_index)); };
};

template<typename Float>
std::ostream &operator<<(std::ostream &os, const Face<Float> &f)
{
    os << "Face: " << f.get_index();
    return os;
}

template<typename Float>
struct Edge : public Element<Float>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Vertex, Face)
    MTS_GEOMETRY_IMPORT_BASE(Element);

    Edge(){};
    Edge(SurfaceMesh<Float>* mesh, Index index):Base(mesh, index){};

    Halfedge halfedge() const { return Halfedge(this->m_mesh, this->m_mesh->ehalfedge(this->m_index)); }

};

template<typename Float>
std::ostream &operator<<(std::ostream &os, const Edge<Float> &e)
{
    os << "Edge: " << e.get_index();
    return os;
}

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba //
////////////////////////////////////