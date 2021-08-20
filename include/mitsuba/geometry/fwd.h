#pragma once

#include <enoki/array_traits.h>
#include <mitsuba/core/fwd.h>
#include <mitsuba/core/spectrum.h>
#include <mitsuba/core/traits.h>

NAMESPACE_BEGIN(mitsuba)

NAMESPACE_BEGIN(geometry)
template <typename Float, typename M, typename Derived> struct Element;
template <typename Float> struct Halfedge;
template <typename Float> struct Vertex;
template <typename Float> struct Edge;
template <typename Float> struct Face;

template <typename Float> class PolygonMesh;
template <typename Float> class SurfaceMesh;
NAMESPACE_END(geometry)

template <typename Float_> struct GeometryAliases {

    using Float                  = Float_;
    using Index                  = size_array_t<Float>;
    // using Spectrum               = Spectrum_;

    /// Strip away any masking-related wrappers from 'Float' and 'Spectrum'
    using FloatU                 = underlying_t<Float>;
    using IndexU                 = underlying_t<Index>;
    // using SpectrumU              = underlying_t<Spectrum>;

    using FStorage               = mitsuba::DynamicBuffer<FloatU>; 
    using IStorage               = mitsuba::DynamicBuffer<IndexU>;

    using Halfedge               = mitsuba::geometry::Halfedge<FloatU>;
    using Vertex                 = mitsuba::geometry::Vertex<FloatU>;
    using Edge                   = mitsuba::geometry::Edge<FloatU>;
    using Face                   = mitsuba::geometry::Face<FloatU>;

    using PolygonMesh            = mitsuba::geometry::PolygonMesh<FloatU>;
    using SurfaceMesh            = mitsuba::geometry::SurfaceMesh<FloatU>;
};

#define MTS_GEOMETRY_IMPORT_BASIC_TYPES()                                       \
    MTS_IMPORT_CORE_TYPES()                                                     \
    using GeometryAliases      = mitsuba::GeometryAliases<Float>;               \
    using Index                = typename GeometryAliases::Index;               \
    using FStorage             = typename GeometryAliases::FStorage;            \
    using IStorage             = typename GeometryAliases::IStorage;            \
    using Halfedge             = typename GeometryAliases::Halfedge;            \
    using Vertex               = typename GeometryAliases::Vertex;              \
    using Edge                 = typename GeometryAliases::Edge;                \
    using Face                 = typename GeometryAliases::Face;

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
