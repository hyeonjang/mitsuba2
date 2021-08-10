#include "mitsuba/geometry/util/sparsematrix.h"

class Geometry
{
    // operators
    template<typename T>
    SparseMatrix<T> cotan_laplacian;

    template<typename T>
    SparseMatrix<T> face_connection_laplacian;

private:
    void compute_cotan_laplacian();
    void compute_face_connection_laplacian();
};