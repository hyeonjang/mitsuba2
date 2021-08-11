#pragma once

#include <mitsuba/core/fwd.h>
#include<mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/surface/surfacemesh.h>

namespace mitsuba {
namespace geometry {

// @@Todo : implement function, output is geometry and surface mesh
template<typename Float, typename Spectrum>
class MTS_EXPORT_GEOMETRY PolygonMesh : public Object
{
public:
    MTS_GEOMETRY_IMPORT_TYPES()

    PolygonMesh();
    PolygonMesh(const DynamicBuffer<UInt32>& polygons, const DynamicBuffer<Float>& vertices)
    :m_polygons(polygons), m_vertices_positions(vertices)
    {
        std::cout << m_polygons << std::endl;
        std::cout << vertices << std::endl;
    }

    SurfaceMesh<Float, Spectrum> to_surface_mesh();
    friend class Mesh<Float, Spectrum>;

protected:

    DynamicBuffer<UInt32> m_polygons;
    DynamicBuffer<Float>  m_vertices_positions;
    DynamicBuffer<Float>  m_vertices_normals;
    DynamicBuffer<Float>  m_vertices_texcoords;
};

}
}