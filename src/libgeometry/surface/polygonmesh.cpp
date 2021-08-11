#include "mitsuba/geometry/surface/polygonmesh.h"
#include "mitsuba/geometry/surface/surfacemesh.h"

namespace mitsuba {
namespace geometry {

MTS_VARIANT SurfaceMesh<Float, Spectrum> PolygonMesh<Float, Spectrum>::to_surface_mesh() 
{
    std::unique_ptr<SurfaceMesh<Float, Spectrum>, Spectrum> mesh;
    mesh.reset(new SurfaceMesh(this->m_faces_buf));

    return mesh;
}  

}
}