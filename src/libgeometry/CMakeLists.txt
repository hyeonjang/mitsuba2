set(INC_DIR "../../include/mitsuba/geometry")

set(LIBGEOMETRY_EXTRA_SRC "")

add_library(mitsuba-geometry-obj OBJECT
    ${INC_DIR}/../mitsuba.h
    ${INC_DIR}/util/sparsematrix.h
    ${INC_DIR}/util/element.h

    surface/halfedge.cpp    ${INC_DIR}/surface/halfedge.h

    surface/polygonmesh.cpp ${INC_DIR}/surface/polygonmesh.h
    surface/surfacemesh.cpp ${INC_DIR}/surface/surfacemesh.h
    surface/geometry.cpp    ${INC_DIR}/surface/geometry.h
)

add_library(mitsuba-geometry SHARED $<TARGET_OBJECTS:mitsuba-geometry-obj>)
set_property(TARGET mitsuba-core-obj PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(mitsuba-geometry-obj mitsuba-geometry PROPERTIES FOLDER mitsuba-geometry)
target_compile_definitions(mitsuba-geometry-obj PRIVATE ${PNG_DEFINES} -DMTS_BUILD_MODULE=MTS_MODULE_CORE)

if (CMAKE_CXX_COMPILER_ID MATCHES "^(GNU)$")
  target_link_libraries(mitsuba-geometry PRIVATE -Wl,--no-undefined)
endif()

# Copy to 'dist' directory
add_dist(mitsuba-geometry)

# Python bindings
if (MTS_ENABLE_PYTHON)
  add_subdirectory(python)
endif()

# Register the test directory
add_tests(${CMAKE_CURRENT_SOURCE_DIR}/tests)

if (MTS_ENABLE_OPTIX)
    add_dist(enoki-autodiff enoki-cuda)
    target_link_libraries(mitsuba-geometry PUBLIC enoki-autodiff enoki-cuda cuda)
endif()