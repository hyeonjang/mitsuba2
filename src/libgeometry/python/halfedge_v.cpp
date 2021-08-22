#include <mitsuba/python/python.h>
#include <mitsuba/geometry/util/element.h>
#include <mitsuba/geometry/surface/halfedge.h>

#include "docstr.h"

// element
MTS_PY_EXPORT(Element) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Element, SurfaceMesh)
    MTS_PY_STRUCT(Element)
        .def(py::init<>())
        .def(py::init<SurfaceMesh*, Index>())
        .def("get_index", &Element::get_index,D(Element, get_index), 
            py::return_value_policy::reference_internal)
        .def_method(Element, test_iter)
        ;
}

// halfedge
MTS_PY_EXPORT(Halfedge) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Halfedge, Element)
    py::class_<Halfedge, Element>(m, "Halfedge")
        .def(py::init<>())
        .def_repr(Halfedge)
        .def_method(Halfedge, next)
        //.def_method(Halfedge, next_incoming)
        //.def_method(Halfedge, next_outgoing)
        .def_method(Halfedge, twin)
        .def_method(Halfedge, vertex)
        .def_method(Halfedge, headvertex)
        .def_method(Halfedge, tailvertex)
        .def_method(Halfedge, face)
        .def_method(Halfedge, edge)
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
    MTS_PY_GEOMETRY_IMPORT_TYPES(Vertex, Element)
    py::class_<Vertex, Element>(m, "Vertex")
        .def(py::init<>())
        .def("halfedge", vectorize(&Vertex::halfedge), D(Vertex, halfedge))
        .def_repr(Vertex)
        ;
}

// face
MTS_PY_EXPORT(Face) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Face)
    py::class_<Face, Element>(m, "Face")
        .def(py::init<>())
        .def_repr(Face)
        .def("halfedge", vectorize(&Face::halfedge), D(Face, halfedge))
        ;
}

// edge
MTS_PY_EXPORT(Edge) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(Edge)
    py::class_<Edge, Element>(m, "Edge")
        .def(py::init<>())
        .def_repr(Edge)
        .def("halfedge", vectorize(&Edge::halfedge), D(Edge, halfedge))
        ;
}