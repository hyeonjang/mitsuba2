#include <mitsuba/python/python.h>
#include <mitsuba/geometry/surface/surfacemesh.h>

#include "docstr.h"

MTS_PY_EXPORT(SurfaceMesh) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(SurfaceMesh)
    MTS_PY_CLASS(SurfaceMesh, Object)
        .def(py::init<const DynamicBuffer<UInt32> &>(), D(SurfaceMesh))

        // return set
        .def("halfedges", py::overload_cast<>(&SurfaceMesh::halfedges),
            D(SurfaceMesh, halfedges),
            py::return_value_policy::reference_internal)
        .def("vertices", py::overload_cast<>(&SurfaceMesh::vertices),
            D(SurfaceMesh, vertices),
            py::return_value_policy::reference_internal)
        .def("faces", py::overload_cast<>(&SurfaceMesh::faces),
            D(SurfaceMesh, faces),
            py::return_value_policy::reference_internal)
        .def("edges", py::overload_cast<>(&SurfaceMesh::edges),
            D(SurfaceMesh, edges),
            py::return_value_policy::reference_internal)
        
        // retun element
        .def("halfedge", vectorize(&SurfaceMesh::halfedge),
            "index"_a, D(SurfaceMesh, halfedge))
        .def("vertex", vectorize(&SurfaceMesh::vertex),
            "index"_a, D(SurfaceMesh, vertex))
        .def("face", vectorize(&SurfaceMesh::face),
            "index"_a, D(SurfaceMesh, face))
        .def("edge", vectorize(&SurfaceMesh::edge),
            "index"_a, D(SurfaceMesh, edge))

        .def("size", &SurfaceMesh::size,
            D(SurfaceMesh, size),
            py::return_value_policy::reference_internal) // really ref internal?
        .def("indices", &SurfaceMesh::indices,
            D(SurfaceMesh, indices),
            py::return_value_policy::reference_internal)
        // .def("vertex", py::overload_cast<>(&SurfaceMesh::vertex),
        //     D(SurfaceMesh, vertex),
        //     py::return_value_policy::reference_internal)
        ;
    //    .def_method(SurfaceMesh, clone);
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
