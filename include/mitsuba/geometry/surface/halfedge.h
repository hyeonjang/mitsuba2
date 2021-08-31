#pragma once

#include <mitsuba/core/stream.h>
#include <mitsuba/core/vector.h>
#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/util/element.h>
#include <mitsuba/geometry/surface/surfacemesh.h>

namespace mitsuba {
namespace geometry {

template<typename Float, typename Spectrum>
struct Halfedge : public Element<Float, Spectrum> {
    MTS_GEOMETRY_IMPORT_TYPES(Vertex, Face, Edge, SurfaceMesh, SurfaceMeshPtr)
    
    // make visible the parant class
    MTS_GEOMETRY_IMPORT_BASE(Element)

    Halfedge():Base(){};
    Halfedge(SurfaceMesh* mesh, const Index& index):Base(mesh, index){};

    MTS_INLINE Halfedge next()   const { return Halfedge(this->m_mesh, this->m_mesh->henext(this->m_index)); };
    MTS_INLINE Halfedge next_incoming() const;
    MTS_INLINE Halfedge next_outgoing() const;
    MTS_INLINE Halfedge twin()   const { return Halfedge(this->m_mesh, this->m_mesh->hetwin(this->m_index)); };
    MTS_INLINE Vertex   vertex() const { return Vertex(this->m_mesh, this->m_mesh->hevertex(this->m_index)); }
    MTS_INLINE Vertex   headvertex() const { return next().vertex(); }
    MTS_INLINE Vertex   tailvertex() const { return vertex(); }
    MTS_INLINE Face     face() const { return Face(this->m_mesh, this->m_mesh->heface(this->m_index));}
    MTS_INLINE Edge     edge() const { return Edge(this->m_mesh, this->m_mesh->heedge(this->m_index));}

    MTS_INLINE Halfedge prev_orbit_face();

};

// circular to face, finally indicate last one
template<typename Float, typename Spectrum> 
Halfedge<Float, Spectrum> Halfedge<Float, Spectrum>::prev_orbit_face(){
    Halfedge currHe = *this;
    while(true) {
        Halfedge nextHe = currHe.next();
        if(nextHe == *this) break;
        currHe = nextHe;
    }
  return currHe;
}

template <typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Halfedge<Float, Spectrum> &h)
{
    os << "Halfedge: " << h.get_index();
    return os;
}

template <typename Float, typename Spectrum>
struct Vertex : public Element<Float, Spectrum>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Face, Edge, Corner, SurfaceMesh, SurfaceMeshPtr)
    MTS_GEOMETRY_IMPORT_BASE(Element)

    Vertex(){};
    Vertex(SurfaceMesh* mesh, const Index& index):Base(mesh, index){};

    Halfedge halfedge() const { return Halfedge(this->m_mesh, this->m_mesh->vhalfedge(this->m_index)); };
    Corner   corner()   const;

    MTS_INLINE Halfedge incoming_halfedges() const;
    MTS_INLINE Halfedge outgoing_halfedges() const;
    MTS_INLINE Vertex adj_vertices() const;
    MTS_INLINE Face adj_faces() const;
    MTS_INLINE Edge adj_edges() const;
    MTS_INLINE Corner adj_corners() const;
};

template <typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Vertex<Float, Spectrum> &v) 
{
    os << "Vertex: " << v.get_index();
    return os;
}

template <typename Float, typename Spectrum>
struct Face : public Element<Float, Spectrum>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Vertex, Edge, SurfaceMesh, SurfaceMeshPtr)
    MTS_GEOMETRY_IMPORT_BASE(Element);

    Face():Base(){};
    Face(SurfaceMesh* mesh, const Index& index):Base(mesh, index){};

    Halfedge halfedge() const { return Halfedge(this->m_mesh, this->m_mesh->fhalfedge(this->m_index)); };
};

template<typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Face<Float, Spectrum> &f)
{
    os << "Face: " << f.get_index();
    return os;
}

template<typename Float, typename Spectrum>
struct Edge : public Element<Float, Spectrum>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Vertex, Face, SurfaceMesh, SurfaceMeshPtr)
    MTS_GEOMETRY_IMPORT_BASE(Element);

    Edge():Base(){};
    Edge(SurfaceMesh* mesh, const Index& index):Base(mesh, index){};

    Halfedge halfedge() const { return Halfedge(this->m_mesh, this->m_mesh->ehalfedge(this->m_index)); }
};

template<typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Edge<Float, Spectrum> &e)
{
    os << "Edge: " << e.get_index();
    return os;
}

template<typename Float, typename Spectrum>
struct Corner : public Element<Float, Spectrum>
{

};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba //
////////////////////////////////////