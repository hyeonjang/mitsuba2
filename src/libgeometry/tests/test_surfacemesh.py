import enoki as ek
import pytest
import mitsuba

mitsuba.set_variant("gpu_autodiff_rgb")

def test01_basics():
    from mitsuba.core import BoundingBox3f
    from mitsuba.render import BSDF
    from mitsuba.render import Mesh
    from mitsuba.geometry import SurfaceMesh