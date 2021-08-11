#pragma once

#include <enoki/array_traits.h>
#include <mitsuba/core/fwd.h>
#include <mitsuba/core/spectrum.h>
#include <mitsuba/core/traits.h>

NAMESPACE_BEGIN(mitsuba)

struct Chunk;
template <typename Float, typename Spectrum> class SurfaceMesh;

template <typename Float_, typename Spectrum_> struct GeometryAliases {
    using Float                  = Float_;
    using Spectrum               = Spectrum_;

    /// Strip away any masking-related wrappers from 'Float' and 'Spectrum'
    using FloatU                 = underlying_t<Float>;
    using SpectrumU              = underlying_t<Spectrum>;

    using Chunk                  = mitsuba::Chunk;

    using SurfaceMesh            = mitsuba::SurfaceMesh<FloatU, SpectrumU>;
};

#define MTS_GEOMETRY_IMPORT_BASIC_TYPES()                                                \
    MTS_IMPORT_CORE_TYPES()                                                              \
    using GeometryAliases      = mitsuba::GeometryAliases<Float, Spectrum>;              

// This macro is needed to get this to compile across all compilers
#define MTS_GEOMETRY_IMPORT_TYPES_HELPER(...) GeometryAliases, ##__VA_ARGS__

#define MTS_GEOMETRY_IMPORT_TYPES(...)                                                   \
    MTS_GEOMETRY_IMPORT_BASIC_TYPES()                                                    \
    using Chunk = typename GeometryAliases::Chunk;                                       \
    ENOKI_USING_TYPES(MTS_GEOMETRY_IMPORT_TYPES_HELPER(__VA_ARGS__))

#define MTS_GEOMETRY_IMPORT_OBJECT_TYPES()                                               \
    using SurfaceMesh            = typename GeometryAliases::SurfaceMesh;                

// -----------------------------------------------------------------------------

NAMESPACE_END(mitsuba)
