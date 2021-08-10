#pragma once

#include <mitsuba/core/struct.h>

namespace mitsuba {
namespace geometry {

const size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

// same structure as geometry-central

template<typename M, typename T>
class Element {

public:
    using ParantMeshT = M;

    Element();
    Element(ParantMeshT* mesh, size_t idx);

    // MTS_INLINE bool operator==() const;
    // MTS_INLINE bool operator!=() const;

    size_t get_index() const { return index; }
    ParantMeshT* get_mesh() const { return mesh; }

    bool is_dead() const;

protected:
    size_t index = INVALID_INDEX;
    ParantMeshT* mesh = nullptr;
};

/////////////////////////////////////
} // the end of namespace geometry //
/////////////////////////////////////
} // the end of namespace mitsuba ///
/////////////////////////////////////