import enoki as ek
import pytest
import mitsuba

mitsuba.set_variant("gpu_autodiff_rgb")

class MeshAsset:
    def load_mesh(self, path):
        import os
        from mitsuba.core.xml import load_string #@@todo
        name, typ = os.path.splitext(path)
        mesh = load_string("""
                    <shape type="{typ}" version="0.5.0">
                        <string name="filename" value="{str}"/>
                    </shape>
                """.format(typ=typ[1:], str=name+typ))
        return mesh

def construct_polygonmesh(str):
    from mitsuba.geometry import PolygonMesh
    assets = MeshAsset()
    m = assets.load_mesh(str)
    return PolygonMesh(m.faces_buffer(), m.vertex_positions_buffer())

def construct_surfacemesh(str):
    return construct_polygonmesh(str).to_surface_mesh()

def test_halfedge():
    surfmesh = construct_surfacemesh("assets/two_poly.ply")

def test_vertex():
    surfmesh = construct_surfacemesh("assets/two_poly.ply")

    vertices = surfmesh.vertices()
    vertex = surfmesh.vertex([0, 1, 2, 3, 4, 5])

    assert vertices.get_index() == vertex.get_index()

def test_face():
    surfmesh = construct_surfacemesh("assets/two_poly.ply")

    faces = surfmesh.faces()
    face = surfmesh.face([0, 1, 2, 3, 4, 5])


def test_edge():
    surfmesh = construct_surfacemesh("assets/two_poly.ply")
    edges = surfmesh.edges()
    edge = surfmesh.edge([0, 1, 2, 3, 4, 5])

test_vertex()