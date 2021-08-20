#pragma once

#include <mitsuba/core/fwd.h>
#include <mitsuba/geometry/fwd.h>
#include <mitsuba/geometry/surface/surfacemesh.h>

namespace mitsuba {
namespace geometry {

// @@Todo : implement function, output is geometry and surface mesh
// almost same as original mitsuba::mesh
template<typename Float>
class MTS_EXPORT_GEOMETRY PolygonMesh : public Object
{
public:
    MTS_GEOMETRY_IMPORT_TYPES()

    template <typename Index>
    MTS_INLINE auto face_indices(Index index, mask_t<Index> active = true) const {
        using Result = Array<replace_scalar_t<Index, size_t>, 3>;
        return gather<Result>(m_polygons, index, active);
    }

    PolygonMesh(){};
    PolygonMesh(const IStorage& polygons, const FStorage& vertices)
    :m_polygons(polygons), m_vertices_positions(vertices){}
    PolygonMesh(const IStorage& polygons, const FStorage& vertices, const FStorage& normals, const FStorage& texcoords)
    :m_polygons(polygons), m_vertices_positions(vertices), m_vertices_normals(normals), m_vertices_texcoords(texcoords){}
    
    SurfaceMesh<Float> to_surface_mesh()
    {
        SurfaceMesh<Float> mesh(m_polygons);
        return mesh;
    }

    // std::string to_string() {
    //     std::ostringstream oss;
    //     oss << class_()->name() << "[" << std::endl
    //         << "polygons = " << m_polygons << std::endl;
    // }

protected:

    IStorage    m_polygons;
    FStorage    m_vertices_positions;
    FStorage    m_vertices_normals;
    FStorage    m_vertices_texcoords;
};

}
}