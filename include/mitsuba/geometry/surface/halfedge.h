#pragma once

#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/util/element.h>

namespace mitsuba {
namespace geometry {

template<typename Float>
struct Vertex : Element<SurfaceMesh<Float>, Vertex<Float>>
{
    using Base = Element<SurfaceMesh<Float>, Vertex<Float>>;

    Vertex(){};
    Vertex(SurfaceMesh<Float>* mesh, size_t index):Base(mesh, index){};

    // Halfedge halfedge() const { return Halfedge(mesh, mesh->vHalfedge(index)); };
    // Corner corner() const;
    bool is_dead() const;
};

template<typename Float>
struct MTS_EXPORT_GEOMETRY Halfedge : Element<SurfaceMesh<Float>, Halfedge<Float>>
{
    using Base = Element<SurfaceMesh<Float>, Halfedge<Float>>;

    Halfedge(){};
    Halfedge(SurfaceMesh<Float>* mesh, size_t index):Base(mesh, index){};

    Halfedge    next() const;
    Halfedge    twin() const;
    // Vertex      vertex() const;
    // Edge        edge() const;
    // Face        face() const;
};

template<typename Float>
struct MTS_EXPORT_GEOMETRY Edge : Element<SurfaceMesh<Float>, Halfedge<Float>>
{
    using Base = Element<SurfaceMesh<Float>, Halfedge<Float>>;

    Edge(){};
    Edge(SurfaceMesh<Float>* mesh, size_t index):Base(mesh, index){};

};


/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba //
////////////////////////////////////