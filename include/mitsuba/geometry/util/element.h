#pragma once

#include <mitsuba/core/struct.h>

#include <functional>
#include <iterator>

// namespace std {
//     template<typename Float> 
//     struct hash<mitsuba::geometry::Vertex<Float>> 
//     { 
//         std::size_t operator()(const mitsuba::geometry::Vertex<Float> e) const 
//         { 
//             return std::hash<size_t>{}(e.get_index());
//         }
//     };
// }

namespace mitsuba {
namespace geometry {

// same structure as geometry-central
template <typename Float, typename Spectrum>
struct Element {
    MTS_GEOMETRY_IMPORT_TYPES(SurfaceMesh)

    Element(SurfaceMesh* m, const Index& i):mesh(m), index(i){};

    bool operator==(const Element<Float, Spectrum>& other) const { return mesh==other.mesh&&index==other.index; };
    bool operator!=(const Element<Float, Spectrum>& other) const { return mesh!=other.mesh&&index!=other.index; };

    MTS_INLINE Index get_index(Mask mask=true) const 
    {
        MTS_MASK_ARGUMENT(mask);
        return gather<Index>(index, arange<Index>(slices(index)), mask); 
    }
    SurfaceMesh* get_mesh() const { return mesh; }

    Mask is_valid() const {
        return neq(index, math::Infinity<Index>);
    }

    // temporally testing function
    Element<Float, Spectrum> test_iter() 
    {
        Element<Float, Spectrum> z = zero<Element<Float, Spectrum>>(10);

        // std::cout << z.m_index.begin() << std::endl;
        // std::cout << z.m_index.end()   << std::endl;

        return z;
    }

    SurfaceMesh* mesh = nullptr;
    Index        index;

    ENOKI_STRUCT(Element, mesh, index);
};

//@@todo
template <typename T>
struct IteratorTraits
{

};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba ///
/////////////////////////////////////
ENOKI_STRUCT_SUPPORT(mitsuba::geometry::Element, mesh, index);