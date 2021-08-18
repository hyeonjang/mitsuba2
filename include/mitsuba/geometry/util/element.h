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

template<typename M, typename T>
struct Element {

public:
    using ParantMeshT = M;

    Element();
    Element(ParantMeshT* mesh, size_t idx):m_mesh(mesh), m_index(idx){};

    // MTS_INLINE bool operator==() const;
    // MTS_INLINE bool operator!=() const;

    size_t get_index() const { return m_index; }
    ParantMeshT* get_mesh() const { return m_mesh; }

    bool is_dead() const;

protected:
    ParantMeshT* m_mesh = nullptr;
    size_t m_index;

    friend struct std::hash<Element<M, T>>;
};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba ///
/////////////////////////////////////