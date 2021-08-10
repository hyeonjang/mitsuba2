#pragma once

#include "mitsuba/core/vector.h"

#include <enoki/array.h>
#include <enoki/dynamic.h>

using namespace enoki;

namespace mitsuba {
namespace geometry {

template<typename Float>
class SurfaceMesh 
{
public:
    using Size = size_array_t<Float>;

    SurfaceMesh(const DynamicBuffer<Size>& polygons);

    // MTS_INLINE vertices() { return }

protected:
    n_faces_count;

};

}
}