#include <mitsuba/python/python.h>
#include <mitsuba/geometry/surface/halfedge.h>

#include "docstr.h"

// halfedge
MTS_PY_EXPORT(Halfedge) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Halfedge)
    MTS_PY_STRUCT(Halfedge)
        .def(py::init<>())
        ;
        // .def_method(SurfaceMesh, clone)
        // .def_method(SurfaceMesh, sample_count)
        // .def_method(SurfaceMesh, wavefront_size)
        // .def("seed", vectorize(&SurfaceMesh::seed),
        //      "seed_value"_a, D(SurfaceMesh, seed))
        // .def("next_1d", vectorize(&SurfaceMesh::next_1d),
        //      "active"_a = true, D(SurfaceMesh, next_1d))
        // .def("next_2d", vectorize(&SurfaceMesh::next_2d),
        //      "active"_a = true, D(SurfaceMesh, next_2d));
}

// vertex
MTS_PY_EXPORT(Vertex) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Vertex)
    MTS_PY_STRUCT(Vertex)
        .def(py::init<>())
        ;
}

MTS_PY_EXPORT(Face) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Face)
    MTS_PY_STRUCT(Face)
        .def(py::init<>())
        ;
}

MTS_PY_EXPORT(Edge) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Edge)
    MTS_PY_STRUCT(Edge)
        .def(py::init<>())
        ;
}