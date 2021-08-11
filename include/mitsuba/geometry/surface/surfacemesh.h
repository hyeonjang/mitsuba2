#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/geometry/fwd.h>

NAMESPACE_BEGIN(mitsuba)
NAMESPACE_BEGIN(geometry)

template<typename Float, typename Spectrum>
class MTS_EXPORT_GEOMETRY SurfaceMesh : public Object {
public:
    MTS_GEOMETRY_IMPORT_TYPES()

    SurfaceMesh(){};
    // SurfaceMesh(const DynamicBuffer<size_t>& polygons);

    // MTS_INLINE vertices() { return }

    MTS_DECLARE_CLASS()
protected:
    // SurfaceMesh(const Properties &);
    // virtual ~SurfaceMesh();

    // size_t n_faces_count = 0;
};

MTS_EXTERN_CLASS_GEOMETRY(SurfaceMesh)
NAMESPACE_END(geometry)
NAMESPACE_END(mitsuba)