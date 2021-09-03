#include <mitsuba/core/fwd.h>
#include "mitsuba/geometry/surface/surfacemesh.h"

NAMESPACE_BEGIN(mitsuba)
NAMESPACE_BEGIN(geometry)

MTS_VARIANT SurfaceMesh<Float, Spectrum>::SurfaceMesh(const IStorage& polygons) {
        static_assert(!is_dynamic_array_v<HeStorage>, "Not a dynamic array");

        // now assume that all input polyons are triangle
        n_edges_count = n_halfedges_count = slices(polygons);
        n_faces_count = n_edges_count/n_face_degree;

        // counting the number of vertices
        n_vertices_count += scalar_cast(hmax_nested(polygons));
        n_vertices_count++;

        // memory allocation to member variables
        he = zero<HeStorage>(n_halfedges_count);
        vhalfedges = empty<IStorage>(n_vertices_count);
        fhalfedges = empty<IStorage>(n_faces_count);
        ehalfedges = empty<IStorage>(n_edges_count);

        set_halfedge_main(polygons);
        set_halfedge_twin(polygons);

        // slice(he, 0) = HeStorage(Storage(0), Storage(0), Storage(0), Storage(0), Storage(0));
        // std::cout << "v.halfedge() " << vhalfedges << std::endl;
        // std::cout << "f.halfedge() " << fhalfedges << std::endl;
        // std::cout << "e.halfedge() " << ehalfedges << std::endl;

        // std::cout << "he.next() "   << he.next << std::endl;
        // std::cout << "he.twin() "   << he.twin << std::endl;
        // std::cout << "he.vertex() " << he.vertex << std::endl;
        // std::cout << "he.face() "   << he.face << std::endl;
        // std::cout << "he.edge() "   << he.edge << std::endl;
    }

//@@todo
MTS_VARIANT std::string SurfaceMesh<Float, Spectrum>::to_string() const {
    std::ostringstream oss;
    oss << class_()->name()  << "["  << std::endl
        << "vertices:  " << he.vertex  << std::endl
        << "faces:     " << he.face  << std::endl
        << "edges:     " << he.edge;
    return oss.str();
}


MTS_IMPLEMENT_CLASS_VARIANT(SurfaceMesh, Object)
MTS_INSTANTIATE_CLASS(SurfaceMesh)
NAMESPACE_END(geometry)
NAMESPACE_END(mitsuba)