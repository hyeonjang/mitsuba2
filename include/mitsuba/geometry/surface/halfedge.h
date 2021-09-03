#pragma once

#include <mitsuba/core/struct.h>
#include <mitsuba/core/stream.h>
#include <mitsuba/core/vector.h>
#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/util/util.h>
#include <mitsuba/geometry/util/element.h>
#include <mitsuba/geometry/surface/surfacemesh.h>

namespace mitsuba {
namespace geometry {

template<typename Float, typename Spectrum>
struct Halfedge : public Element<Float, Spectrum> {
    MTS_GEOMETRY_IMPORT_TYPES(Vertex, Face, Edge, SurfaceMesh, SurfaceMeshPtr)
    
    // make visible the parant class
    MTS_GEOMETRY_IMPORT_BASE(Element, mesh, index)

    Halfedge():Base(){};
    Halfedge(SurfaceMesh* m, const Index& i):Base(m, i){};

    MTS_INLINE Halfedge next()   const { return Halfedge(mesh, mesh->henext(index)); };
    MTS_INLINE Halfedge next_incoming() const;
    MTS_INLINE Halfedge next_outgoing() const;
    MTS_INLINE Halfedge twin()   const { return Halfedge(mesh, mesh->hetwin(index)); };
    MTS_INLINE Vertex   vertex() const { return Vertex(mesh, mesh->hevertex(index)); }
    MTS_INLINE Vertex   headvertex() const { return next().vertex(); }
    MTS_INLINE Vertex   tailvertex() const { return vertex(); }
    MTS_INLINE Face     face() const { return Face(mesh, mesh->heface(index));}
    MTS_INLINE Edge     edge() const { return Edge(mesh, mesh->heedge(index));}

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

#define nothing
#define ENOKI_SS(p, o, q) ENOKI_DERIVED_STRUCT(p, o, q, nothing)

template <typename Float, typename Spectrum>
struct Vertex : public Element<Float, Spectrum>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Face, Edge, Corner, SurfaceMesh, SurfaceMeshPtr)
    MTS_GEOMETRY_IMPORT_BASE(Element, mesh, index, get_index, get_mesh)

    Vertex(){};
    Vertex(SurfaceMesh* m, const Index& i):Base(m, i){};

    Halfedge halfedge() const { return Halfedge(mesh, mesh->vhalfedge(index)); };
    Corner   corner()   const;

    MTS_INLINE Halfedge incoming_halfedges() const;
    MTS_INLINE Halfedge outgoing_halfedges() const;
    MTS_INLINE Vertex adj_vertices() const;
    MTS_INLINE Face adj_faces() const;
    MTS_INLINE Edge adj_edges() const;
    MTS_INLINE Corner adj_corners() const;

};

template <typename Float, typename Spectrum>
struct Face : public Element<Float, Spectrum>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Vertex, Edge, SurfaceMesh, SurfaceMeshPtr)
    MTS_GEOMETRY_IMPORT_BASE(Element, mesh, index, get_index)

    Face():Base(){};
    Face(SurfaceMesh* m, const Index& i):Base(m, i){};

    Halfedge halfedge() const { return Halfedge(mesh, mesh->fhalfedge(index)); };

    Vertex adj_vertices() const {

        // using IndexD = Array<Index, 1>;

        // IndexD result = halfedge().get_index();
        // Halfedge currHe = halfedge();
        // while(true) {
        //     Halfedge nextHe = currHe.next();
        //     if(nextHe == halfedge()) break;
        //     currHe = nextHe;

        //     std::cout << typeid(decltype(currHe.get_index())).name() << std::endl;

        //     IndexD next = nextHe.get_index();
        //     IndexD curr = currHe.get_index();

        //     result = util::concat(result, curr);
        //     std::cout << result << std::endl;
        // }
        // return currHe.vertex();
    }

    Halfedge adj_halfedges() const {
        std::cout << this->get_index() << std::endl;
        std::cout << halfedge() << std::endl;
        std::cout << halfedge().next() << std::endl;
        std::cout << halfedge().next().next() << std::endl;
        std::cout << halfedge().next().next().next() << std::endl;
    }

};


template<typename Float, typename Spectrum>
struct Edge : public Element<Float, Spectrum>
{
    MTS_GEOMETRY_IMPORT_TYPES(Halfedge, Vertex, Face, SurfaceMesh, SurfaceMeshPtr)
    MTS_GEOMETRY_IMPORT_BASE(Element, mesh, index)

    Edge():Base(){};
    Edge(SurfaceMesh* m, const Index& i):Base(m, i){};

    Halfedge halfedge() const { return Halfedge(mesh, mesh->ehalfedge(index)); }
};

template<typename Float, typename Spectrum>
struct Corner : public Element<Float, Spectrum>
{

};


// operator<< implementation
template <typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Halfedge<Float, Spectrum> &h)
{
    os << "Halfedge: " << h.get_index();
    return os;
}

template <typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Vertex<Float, Spectrum> &v) 
{
    os << "Vertex: " << v.get_index();
    return os;
}

template<typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Face<Float, Spectrum> &f)
{
    os << "Face: " << f.get_index();
    return os;
}

template<typename Float, typename Spectrum>
std::ostream &operator<<(std::ostream &os, const Edge<Float, Spectrum> &e)
{
    os << "Edge: " << e.get_index();
    return os;
}

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba //
////////////////////////////////////