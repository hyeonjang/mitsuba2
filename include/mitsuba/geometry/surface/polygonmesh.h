#include <mitsuba/render/mesh.h>

#include <mitsuba/geometry/surface/surfacemesh.h>

namespace mitsuba {
namespace geometry {

// @@Todo : implement function, output is geometry and surface mesh
template<typename Float, typename Spectrum>
class PolygonMesh : Mesh<Float, Spectrum>
{
    SurfaceMesh<Float> to_surface_mesh();

    friend class Mesh<Float, Spectrum>;
};

}
}