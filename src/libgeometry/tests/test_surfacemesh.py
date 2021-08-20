import enoki as ek
import pytest
import mitsuba

mitsuba.set_variant("gpu_autodiff_rgb")

class MeshAsset:
    def load_mesh(self, typ, str):
        from mitsuba.core.xml import load_string #@@todo
        mesh = load_string("""
                    <shape type="{typ}" version="0.5.0">
                        <string name="filename" value="{str}"/>
                    </shape>
                """.format(typ=typ, str=str))
        return mesh


def test01_basics():
    from mitsuba.geometry import PolygonMesh, SurfaceMesh

    assets = MeshAsset()
    m = assets.load_mesh("ply", "assets/two_poly.ply")
    polymesh = PolygonMesh(m.faces_buffer(), m.vertex_positions_buffer())
    surfacemesh = polymesh.to_surface_mesh()
    print(surfacemesh.vertices())


test01_basics()