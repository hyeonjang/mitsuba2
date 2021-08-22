import enoki as ek
import pytest
import mitsuba

from mesh_generation import construct_surfacemesh

surfmesh = construct_surfacemesh("assets/two_poly.ply")
indices = surfmesh.indices()

# Halfedge test
def test_halfedge():
    from mitsuba.geometry import Element

    halfedges = surfmesh.halfedges()
    halfedge = surfmesh.halfedge(indices)
    # halfedge = surfmesh.halfedge(indices)
    
    print(id(halfedges.get_index()))
    print(id(halfedge.get_index()))

    # instance test
    assert halfedge.get_index() == halfedges.get_index()

    # # next() test
    assert halfedge.get_index() == halfedge.next().next().next().get_index()
    
    # # twin() test

    # # vertex() test
    assert halfedges.vertex().get_index() == surfmesh.vertices().get_index()

    # # headvertex() tailvertex() test
    assert halfedges.headvertex().get_index() == halfedges.next().tailvertex().get_index()

    # # face() test
    assert halfedges.face().get_index() == surfmesh.faces().get_index()

    # iter() test

    # edge() test

test_halfedge()

# Vertex test
def test_vertex():

    vertices = surfmesh.vertices()
    vertex = surfmesh.vertex(indices)

    assert vertex.get_index() == vertices.get_index()
    assert vertex.halfedge().get_index() == vertices.halfedge().get_index()


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