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

    Element(){};
    Element(SurfaceMesh* mesh, Index idx):m_mesh(mesh), m_index(idx){};

    // MTS_INLINE bool operator==() const;
    // MTS_INLINE bool operator!=() const;

    Index get_index() const { return m_index; }
    SurfaceMesh* get_mesh() const { return m_mesh; }

    bool is_dead() const;

protected:
    SurfaceMesh* m_mesh = nullptr;
    Index m_index;
};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba ///
/////////////////////////////////////