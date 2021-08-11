#include <mitsuba/python/python.h>

// MTS_PY_DECLARE(SurfaceMesh);

PYBIND11_MODULE(geometry_ext, m){
    
    m.attr("__name__") = "mitsuba.geometry";

    // MTS_PY_IMPORT(SurfaceMesh);

    m.attr("__name__") = "mitsuba.geometry_ext";
}