#pragma once

#include <mitsuba/geometry/fwd.h>

namespace mitsuba {
namespace geometry {

template<typename Float>
struct HalfedgeStorage
{
    MTS_GEOMETRY_IMPORT_TYPES();
    using Bool = bool_array_t<Index>;

    Index next;
    Index twin;
    Index vertex;
    Index face;
    Index edge;
    Bool  orient; 

    ENOKI_STRUCT(HalfedgeStorage, next, twin, vertex, face, edge, orient);
};

}
}
ENOKI_STRUCT_SUPPORT(mitsuba::geometry::HalfedgeStorage, next, twin, vertex, face, edge, orient);
