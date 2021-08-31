#pragma once

#include <enoki/array_traits.h>
#include <mitsuba/core/fwd.h>
#include <mitsuba/core/spectrum.h>
#include <mitsuba/core/traits.h>

NAMESPACE_BEGIN(mitsuba)

NAMESPACE_BEGIN(geometry)
// static constexpr Index INVALID    = mitsuba::math::Max<Index>; // @@tood move to where;

template <typename Float, typename Spectrum> class PolygonMesh;
template <typename Float, typename Spectrum> class SurfaceMesh;
template <typename Float, typename Spectrum> struct Element;
template <typename Float, typename Spectrum> struct Halfedge;
template <typename Float, typename Spectrum> struct Vertex;
template <typename Float, typename Spectrum> struct Edge;
template <typename Float, typename Spectrum> struct Face;
template <typename Float, typename Spectrum> struct Corner;
NAMESPACE_END(geometry)

template <typename Float_, typename Spectrum_> struct GeometryAliases {

    using Float                 = Float_;
    using Spectrum              = Spectrum_;
    using Index                 = size_array_t<Float>;

    /// Strip away any masking-related wrappers from 'Float' and 'Spectrum'
    using FloatU                = underlying_t<Float>;
    using SpectrumU             = underlying_t<Spectrum>;
    using IndexU                = underlying_t<Index>;
    // using SpectrumU              = underlying_t<Spectrum>;

    using FStorage              = mitsuba::DynamicBuffer<Float>; 
    using IStorage              = mitsuba::DynamicBuffer<Index>;

    using PolygonMesh           = mitsuba::geometry::PolygonMesh<FloatU, SpectrumU>;
    using SurfaceMesh           = mitsuba::geometry::SurfaceMesh<FloatU, SpectrumU>;

    using Element               = mitsuba::geometry::Element<FloatU, SpectrumU>;
    using Halfedge              = mitsuba::geometry::Halfedge<FloatU, SpectrumU>;
    using Vertex                = mitsuba::geometry::Vertex<FloatU, SpectrumU>;
    using Edge                  = mitsuba::geometry::Edge<FloatU, SpectrumU>;
    using Face                  = mitsuba::geometry::Face<FloatU, SpectrumU>;
    using Corner                = mitsuba::geometry::Corner<FloatU, SpectrumU>;

    using PolygonMeshPtr        = replace_scalar_t<Float, const PolygonMesh*>; 
    using SurfaceMeshPtr        = replace_scalar_t<Float, const SurfaceMesh*>;
};

#define MTS_GEOMETRY_IMPORT_BASIC_TYPES()                                       \
    MTS_IMPORT_CORE_TYPES()                                                     \
    using GeometryAliases      = mitsuba::GeometryAliases<Float, Spectrum>;     \
    using Index                = typename GeometryAliases::Index;               \
    using FStorage             = typename GeometryAliases::FStorage;            \
    using IStorage             = typename GeometryAliases::IStorage;            

// This macro is needed to get this to compile across all compilers
#define MTS_GEOMETRY_IMPORT_TYPES_HELPER(...) GeometryAliases, ##__VA_ARGS__

#define MTS_GEOMETRY_IMPORT_TYPES(...)                                                   \
    MTS_GEOMETRY_IMPORT_BASIC_TYPES()                                                    \
    ENOKI_USING_TYPES(MTS_GEOMETRY_IMPORT_TYPES_HELPER(__VA_ARGS__))

#define MTS_GEOMETRY_IMPORT_OBJECT_TYPES()                                      \
    using PolygonMesh          = typename GeometryAliases::PolygonMesh;         \
    using SurfaceMesh          = typename GeometryAliases::SurfaceMesh;         \
    using Element              = typename GeometryAliases::Element;             \
    using Halfedge             = typename GeometryAliases::Halfedge;            \
    using Vertex               = typename GeometryAliases::Vertex;              \
    using Edge                 = typename GeometryAliases::Edge;                \
    using Face                 = typename GeometryAliases::Face;                \
    using Corner               = typename GeometryAliases::Corner;              \
    using PolygonMeshPtr       = typename GeometryAliases::PolygonMeshPtr;     \
    using SurfaceMeshPtr       = typename GeometryAliases::SurfaceMeshPtr;     
    
    // -----------------------------------------------------------------------------

// not clear
#define MTS_GEOMETRY_IMPORT_BASE_HELPER(...) Base, ##__VA_ARGS__

#define MTS_GEOMETRY_IMPORT_BASE(Name, ...)         \
    using Base = Name<Float, Spectrum>;                     \
    ENOKI_USING_MEMBERS(MTS_GEOMETRY_IMPORT_BASE_HELPER(__VA_ARGS__))


NAMESPACE_END(mitsuba)
