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

const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

// same structure as geometry-central

template<typename Float, typename T, typename M>
struct Element {
    // using Index = size_array_t<Float>;
    MTS_GEOMETRY_IMPORT_TYPES()
    using ParantMeshT = M;

    Element(){};
    Element(ParantMeshT* mesh, Index idx):m_mesh(mesh), m_index(idx){};

    // MTS_INLINE bool operator==() const;
    // MTS_INLINE bool operator!=() const;

    Index get_index() const { return m_index; }
    ParantMeshT* get_mesh() const { return m_mesh; }

    bool is_dead() const;

protected:
    ParantMeshT* m_mesh = nullptr;
    Index m_index;
};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba ///
/////////////////////////////////////