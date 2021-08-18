import enoki as ek
import pytest
import mitsuba

mitsuba.set_variant("gpu_autodiff_rgb")

def test01_create_polygon_mesh():
    from mitsuba.core import Float, UInt32
    from mitsuba.render import Mesh
    from mitsuba.geometry import PolygonMesh

    m = Mesh("MyMesh", 3, 2)
    m.vertex_positions_buffer()[:] = [0.0, 0.0, 0.0, 1.0, 0.2, 0.0, 0.2, 1.0, 0.0]
    m.faces_buffer()[:] = [0, 1, 2, 1, 2, 0]
    m.parameters_changed()

    vertices = Float([0.0, 0.0, 0.0, 1.0, 0.2, 0.0, 0.2, 1.0, 0.0])
    polygons = UInt32([0, 1, 2, 0, 1, 3])

    mesh = PolygonMesh(m.faces_buffer(), m.vertex_positions_buffer())
    mesh.to_surface_mesh()

test01_create_polygon_mesh()