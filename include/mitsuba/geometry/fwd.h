#pragma once

#include <enoki/array_traits.h>
#include <mitsuba/core/fwd.h>
#include <mitsuba/core/spectrum.h>
#include <mitsuba/core/traits.h>

NAMESPACE_BEGIN(mitsuba)

NAMESPACE_BEGIN(geometry)
template <typename Float> struct Element;
template <typename Float> struct Vertex;
template <typename Float> struct Halfedge;
template <typename Float> struct Edge;

template <typename Float, typename Spectrum> class PolygonMesh;
template <typename Float, typename Spectrum> class SurfaceMesh;
NAMESPACE_END(geometry)

template <typename Float_, typename Spectrum_> struct GeometryAliases {
    using Float                  = Float_;
    using Spectrum               = Spectrum_;

    /// Strip away any masking-related wrappers from 'Float' and 'Spectrum'
    using FloatU                 = underlying_t<Float>;
    using SpectrumU              = underlying_t<Spectrum>;

    using Vertex                 = mitsuba::geometry::Vertex<FloatU>;
    using Halfedge               = mitsuba::geometry::Halfedge<FloatU>;
    using Edge                   = mitsuba::geometry::Edge<FloatU>;

    using PolygonMesh            = mitsuba::geometry::PolygonMesh<FloatU, SpectrumU>;
    using SurfaceMesh            = mitsuba::geometry::SurfaceMesh<FloatU, SpectrumU>;
};

#define MTS_GEOMETRY_IMPORT_BASIC_TYPES()                                       \
    MTS_IMPORT_CORE_TYPES()                                                     \
    using GeometryAliases      = mitsuba::GeometryAliases<Float, Spectrum>;     \
    using Vertex               = typename GeometryAliases::Vertex;              \
    using Halfedge             = typename GeometryAliases::Halfedge;            \
    using Edge                 = typename GeometryAliases::Edge;

// This macro is needed to get this to compile across all compilers
#define MTS_GEOMETRY_IMPORT_TYPES_HELPER(...) GeometryAliases, ##__VA_ARGS__

#define MTS_GEOMETRY_IMPORT_TYPES(...)                                                   \
    MTS_GEOMETRY_IMPORT_BASIC_TYPES()                                                    \
    ENOKI_USING_TYPES(MTS_GEOMETRY_IMPORT_TYPES_HELPER(__VA_ARGS__))

#define MTS_GEOMETRY_IMPORT_OBJECT_TYPES()                                \
    using PolygonMesh            = typename GeometryAliases::PolygonMesh; \
    using SurfaceMesh            = typename GeometryAliases::SurfaceMesh;
    // -----------------------------------------------------------------------------

NAMESPACE_END(mitsuba)
