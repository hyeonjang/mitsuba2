#pragma once

#if defined(ENOKI_CUDA)
#include <enoki/cuda.h>
#endif
#if defined(ENOKI_AUTODIFF)
#include <enoki/autodiff.h>
#endif
#include <enoki/dynamic.h>
#include <enoki/transform.h>
#include <enoki/special.h>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <sstream>

using namespace enoki;
namespace py = pybind11;
using namespace py::literals;

using Float32 = float;
using Float64 = double;
using Int32   = int32_t;
using UInt32  = uint32_t;
using Int64   = int64_t;
using UInt64  = uint64_t;