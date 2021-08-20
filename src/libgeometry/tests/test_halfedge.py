import enoki as ek
import pytest
import mitsuba

from mesh_generation import construct_surfacemesh

surfmesh = construct_surfacemesh("assets/two_poly.ply")
indices = surfmesh.indices()

# Halfedge test
def test_halfedge():
    halfedges = surfmesh.halfedges()
    halfedge = surfmesh.halfedge(indices)

    assert halfedges.get_index() == halfedge.get_index()

# Vertex test
def test_vertex():

    vertices = surfmesh.vertices()
    vertex = surfmesh.vertex(indices)

    assert vertices.get_index() == vertex.get_index()
    assert vertices.halfedge().get_index() == vertex.halfedge().get_index()

test_vertex()

# Face test
def test_face():
    faces = surfmesh.faces()
    face = surfmesh.face(indices)

    assert faces.get_index() == face.get_index()

# Edge test
def test_edge():
    edges = surfmesh.edges()
    edge = surfmesh.edge(indices)

    assert edges.get_index() == edge.get_index()