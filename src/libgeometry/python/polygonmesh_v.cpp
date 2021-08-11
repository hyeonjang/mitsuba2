#include <mitsuba/python/python.h>
#include <mitsuba/geometry/surface/polygonmesh.h>

#include "docstr.h"

MTS_PY_EXPORT(PolygonMesh) {
    MTS_PY_GEOMETRY_IMPORT_TYPES(PolygonMesh)
    MTS_PY_CLASS(PolygonMesh, Object)
        .def(py::init<const DynamicBuffer<UInt32> &, const DynamicBuffer<Float> &>())
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
