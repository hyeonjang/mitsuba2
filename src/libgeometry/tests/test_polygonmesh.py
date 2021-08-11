import enoki as ek
import pytest
import mitsuba

mitsuba.set_variant("gpu_autodiff_rgb")

def test01_create_polygon_mesh():
    from mitsuba.render import Mesh
    from mitsuba.geometry import PolygonMesh

    m = Mesh("MyMesh", 3, 2)
    m.vertex_positions_buffer()[:] = [0.0, 0.0, 0.0, 1.0, 0.2, 0.0, 0.2, 1.0, 0.0]
    m.faces_buffer()[:] = [0, 1, 2, 1, 2, 0]
    m.parameters_changed()

    PolygonMesh(m.vertex_positions_buffer(), m.faces_buffer())