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

    Element(SurfaceMesh* mesh, const Index& index):m_mesh(mesh), m_index(index){};

    Mask operator==(const Element<Float, Spectrum>& other) const { return eq(m_index, other.m_index); };
    Mask operator!=(const Element<Float, Spectrum>& other) const { return neq(m_index, other.m_index); };

    MTS_INLINE Index get_index(Mask mask=true) const 
    {
        MTS_MASK_ARGUMENT(mask);
        return gather<Index>(m_index, arange<Index>(slices(m_index)), mask); 
    }
    SurfaceMesh* get_mesh() const { return m_mesh; }

    Mask is_valid() const {
        return neq(index, math::Infinity<Index>);
    }

    // temporally testing function
    Element<Float, Spectrum> test_iter() 
    {
        Element<Float, Spectrum> z = zero<Element<Float, Spectrum>>(10);

        // std::cout << z.m_index.begin() << std::endl;
        // std::cout << z.m_index.end()   << std::endl;

        std::cout << z.m_mesh  << std::endl; 
        std::cout << z.m_index << std::endl;

        return z;
    }

    SurfaceMesh* m_mesh = nullptr;
    Index        m_index;

    ENOKI_STRUCT(Element, m_mesh, m_index);
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
ENOKI_STRUCT_SUPPORT(mitsuba::geometry::Element, m_mesh, m_index);