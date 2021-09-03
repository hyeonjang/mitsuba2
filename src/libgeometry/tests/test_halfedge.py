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
    
    # instance test
    assert halfedge == halfedges

    # next() test
    assert halfedge == halfedge.next().next().next()
    
    # twin() test
    assert halfedge == halfedge.twin().twin()

    # vertex() test
    assert halfedges.vertex() == surfmesh.vertices()

    # headvertex() tailvertex() test
    assert halfedges.headvertex() == halfedges.next().tailvertex()

    # face() test
    assert halfedges.face() == surfmesh.faces()

    # iter() test

    # edge() test

# Vertex test
def test_vertex():

    vertices = surfmesh.vertices()
    vertex = surfmesh.vertex(indices)

    # instance test
    assert vertex == vertices

    # halfedge() test
    assert vertex.halfedge() == vertices.halfedge()

# Face test
def test_face():
    faces = surfmesh.faces()
    face = surfmesh.face(indices)

    assert faces == face

    faces.adj_vertices()

test_face()

# Edge test
def test_edge():
    edges = surfmesh.edges()
    edge = surfmesh.edge(indices)

    assert ek.all(edges == edge)