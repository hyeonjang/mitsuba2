#pragma once

#include <enoki/array.h>

using namespace enoki;

namespace geometry {
namespace internal {

template<typename Value_> 
struct Storage
{
    using Value = Value_;
    using Index = uint32_array_t<Value_>;

    Value value;
    Index index;

    using ArrayType = Storage;
    using MaskType = Mask<mask_t<Value>>;

    // constructor
    ENOKI_STRUCT(Storage, value, index);

    ENOKI_INLINE auto operator-() const { return Storage(-this->value, index); }
    // template<typename T>
    // ENOKI_INLINE auto operator*(const T& other)
    // {
    //     return Storage(value*other, index);
    // }

    // template<typename T>
    // ENOKI_INLINE auto operator/(const T& other)
    // {
    //     return Storage(value*rcp(other), index);
    // }
};
}
}
ENOKI_STRUCT_SUPPORT(ekgeo::internal::Storage, value, index);