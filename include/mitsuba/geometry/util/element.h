#pragma once

#include <mitsuba/core/struct.h>

#include <functional>

namespace std {
    template<typename Float> 
    struct hash<mitsuba::geometry::Vertex<Float>> 
    { 
        std::size_t operator()(const mitsuba::geometry::Vertex<Float> e) const 
        { 
            return std::hash<size_t>{}(e.get_index());
        }
    };
}

namespace mitsuba {
namespace geometry {

// same structure as geometry-central
template<typename Float>
struct Element {
    // using Index = size_array_t<Float>;
    MTS_GEOMETRY_IMPORT_TYPES(SurfaceMesh)

    // Element(){};
    Element(SurfaceMesh* mesh, const Index& index):m_mesh(mesh), m_index(index){};

    // MTS_INLINE bool operator==() const;
    // MTS_INLINE bool operator!=() const;

    Index get_index() const { return m_index; }
    SurfaceMesh* get_mesh() const { return m_mesh; }

    bool is_dead() const;

    // temporally testing function
    Element<Float> test_iter() 
    {
        // Element<Float> elem;
        // Element z = zero<Element<Float>>(10);
    }

    SurfaceMesh* m_mesh = nullptr;
    Index          m_index;

    ENOKI_STRUCT(Element, m_mesh, m_index);
};

struct Iterator
{

};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba ///
/////////////////////////////////////
ENOKI_STRUCT_SUPPORT(mitsuba::geometry::Element, m_mesh, m_index);