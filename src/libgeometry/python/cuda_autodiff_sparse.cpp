#include "common.h"
#include "ekgeo/util/sparsematrix.h"

// using SparseMatrix = SparseMatrix<Float32>;
// using SparseMatrix = SparseMatrix<Float32>;


void bind_cuda_autodiff_sparsematrix(py::module& m , py::module& s) 
{
    bind_sparse_matrix<SparseMatrix>(m, s, "SparseMatrixf");
}
