import mitsuba

mitsuba.set_variant('gpu_autodiff_rgb')

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