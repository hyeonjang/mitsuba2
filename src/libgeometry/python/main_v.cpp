#include <mitsuba/geometry/surface/polygonmesh.h>
#include <mitsuba/geometry/surface/surfacemesh.h>
#include <mitsuba/python/python.h>

#define MODULE_NAME MTS_MODULE_NAME(geometry, MTS_VARIANT_NAME)

#define PY_TRY_CAST(Type)                                         \
    if (auto tmp = dynamic_cast<Type *>(o); tmp)                  \
        return py::cast(tmp);

/// Helper routine to cast Mitsuba plugins to their underlying interfaces
static py::object caster(Object *o) {
    MTS_PY_GEOMETRY_IMPORT_TYPES();
    // MTS_PY_IMPORT_TYPES();

    // Try casting, starting from the most precise types
    // auto tmp0 = dynamic_cast<Sampler*>(o);
    // py::cast(tmp0);

    // PY_TRY_CAST(Sampler);
    PY_TRY_CAST(Halfedge);
    PY_TRY_CAST(Vertex);
    PY_TRY_CAST(Face);
    PY_TRY_CAST(Edge);
    PY_TRY_CAST(PolygonMesh);
    PY_TRY_CAST(SurfaceMesh);

    return py::object();
}

// MTS_PY_DECLARE(util);
// MTS_PY_DECLARE(surface);
MTS_PY_DECLARE(Halfedge);
MTS_PY_DECLARE(Vertex);
MTS_PY_DECLARE(Face);
MTS_PY_DECLARE(Edge);
MTS_PY_DECLARE(PolygonMesh);
MTS_PY_DECLARE(SurfaceMesh);
 
PYBIND11_MODULE(MODULE_NAME, m) {
    // Temporarily change the module name (for pydoc)
    m.attr("__name__") = "mitsuba.geometry";

    // Create sub-modules
    py::module util     = create_submodule(m, "util");
    py::module surface  = create_submodule(m, "submodule");

    util.doc()     = "including sparsematrix";
    surface.doc()  = "Process mesh (like geometry-central)";

    MTS_PY_IMPORT(Halfedge);
    MTS_PY_IMPORT(Vertex);
    MTS_PY_IMPORT(Face);
    MTS_PY_IMPORT(Edge);
    MTS_PY_IMPORT(PolygonMesh);
    MTS_PY_IMPORT(SurfaceMesh);

    auto casters = (std::vector<void *> *) (py::capsule)(
        py::module::import("mitsuba.core_ext").attr("casters"));
    casters->push_back((void *) caster);

    // Change module name back to correct value
    m.attr("__name__") = "mitsuba." ENOKI_TOSTRING(MODULE_NAME);
}
