#include "mitsuba/core/fwd.h"
#include "mitsuba/geometry/surface/surfacemesh.h"

NAMESPACE_BEGIN(mitsuba)
NAMESPACE_BEGIN(geometry)
// namespace geometry {

// MTS_VARIANT SurfaceMesh<Float, Spectrum>::SurfaceMesh(const Properties &props) {
// }

// MTS_VARIANT SurfaceMesh<Float, Spectrum>::~SurfaceMesh() { }

// SurfaceMesh::SurfaceMesh()
// {

// }

// SurfaceMesh::SurfaceMesh(const DynamicBuffer<size_t>& polygons)
// {
//     n_faces_count = slices(polygons);    

// }

MTS_IMPLEMENT_CLASS_VARIANT(SurfaceMesh, Object)
MTS_INSTANTIATE_CLASS(SurfaceMesh)
NAMESPACE_END(geometry)
NAMESPACE_END(mitsuba)