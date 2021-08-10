#pragma once

#include <mitsuba/geometry/util/element.h>

namespace mitsuba {
namespace geometry {

class SurfaceMesh;

struct Vertex;
struct Halfedge;
struct Corner;

struct Vertex : Element<SurfaceMesh, Vertex>
{
    Vertex();
    Vertex(SurfaceMesh* mesh, size_t index);

    // Halfedge halfedge() const { return Halfedge(mesh, mesh->vHalfedge(index)); };
    Corner corner() const;
    bool is_dead() const;
};

struct Halfedge : Element<SurfaceMesh, Halfedge>
{
    Halfedge();
    Halfedge(SurfaceMesh* mesh, size_t index);

    Halfedge    next() const;
    Halfedge    twin() const;
    // Vertex      vertex() const;
    // Edge        edge() const;
    // Face        face() const;
};

struct Corner
{

};

struct Edge
{

};

struct Face
{

};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba //
////////////////////////////////////