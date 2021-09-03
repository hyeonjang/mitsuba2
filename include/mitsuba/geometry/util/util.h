#pragma once

#include <enoki/stl.h>

NAMESPACE_BEGIN(mitsuba)
NAMESPACE_BEGIN(geometry)
NAMESPACE_BEGIN(util)

template <typename T1, typename T2, enable_if_array_any_t<T1, T2> = 0>
auto concat(const T1 &a1, const T2 &a2) {
    static_assert(is_array_any_v<T1, T2>,
                  "concat(): at least one of the inputs must be an array!");
    static_assert(std::is_same_v<scalar_t<T1>, scalar_t<T2>>,
                  "concat(): Scalar types must be identical");

    constexpr size_t Size1 = array_size_v<T1>,
                     Size2 = array_size_v<T2>;

    using Result = Array<value_t<expr_t<T1, T2>>, Size1 + Size2>;

    if constexpr (Result::Size1 == T1::Size && Result::Size2 == T2::Size) {
        return Result(a1, a2);
    } else {
        Result result;

        if constexpr (is_array_v<T1>) {
            if constexpr (Result::Size == T1::Size) {
                result = a1.derived();
            } else {
                for (size_t i = 0; i < Size1; ++i)
                    result.coeff(i) = a1.derived().coeff(i);
            }
        } else {
            result.coeff(0) = a1;
        }

        if constexpr (is_array_v<T2>) {
            for (size_t i = 0; i < Size2; ++i)
                result.coeff(i + Size1) = a2.derived().coeff(i);
        } else {
            result.coeff(Size1) = a2;
        }

        return result;
    }
}


NAMESPACE_END(util)
NAMESPACE_END(geometry)
NAMESPACE_END(mitsuba)