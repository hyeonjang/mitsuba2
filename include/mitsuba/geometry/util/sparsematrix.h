/*
    2021

    Sparse data structure (dynamic memory type)

    Eigen like enoki SparseMatrix implementation

*/

#pragma once

#include <enoki/array.h>
#include <enoki/dynamic.h>
#include <enoki/autodiff.h>

#include "storage.h"

namespace geometry
{

using namespace enoki;

// Value trait to access the column type of a matrix same as static matrix
template <typename T> using column_t = typename std::decay_t<T>::Column;

// Value trait to access the entry type of a matrix
template <typename T> using entry_t = value_t<column_t<T>>;

// CCS Sparse Matrix
template <typename Value_>
struct SparseMatrix 
{
    using Value = Value_;
    using Storage = internal::Storage<Value>;
    using StorageIndex = size_array_t<Value>;

    static constexpr bool IsSparseMatrix = true;
    // static constexpr bool IsDynamic = true;
    static constexpr bool IsDiff = Value::IsDiff;

    size_t m_rows_size; 
    size_t m_cols_size;
    
    StorageIndex m_ptr;
    Storage m_data;

public:
    // constructors
    ENOKI_STRUCT(SparseMatrix, m_rows_size, m_cols_size, m_ptr, m_data);
    ENOKI_INLINE SparseMatrix(const size_t m, const size_t n):m_rows_size(m),m_cols_size(n){};

    ENOKI_INLINE decltype(auto) n_rows() const { return m_rows_size; }
    ENOKI_INLINE decltype(auto) n_cols() const { return m_cols_size; }
    ENOKI_INLINE Storage& data() { return m_data; }
    ENOKI_INLINE const Storage& data() const { return m_data; }

    //@@todo add more fluency constructor??

    ENOKI_INLINE decltype(auto) coeff(size_t index) const 
    {
        using Index = uint32_array_t<Value>;

        Value result = zero<Value>(m_cols_size);

        size_t i = m_ptr[index];
        size_t j = m_ptr[index+1];
        Index end = arange<Index>(i, j);

        auto data_value = gather<Value>(m_data.value, end);
        auto data_index = gather<Index>(m_data.index, end);

        scatter(result, data_value, data_index);
        return result;
    }
    ENOKI_INLINE decltype(auto) coeff(size_t i, size_t j) { return coeff(i)[j]; }
    ENOKI_INLINE decltype(auto) coeff(size_t i, size_t j) const { return coeff(i)[j]; }
    ENOKI_INLINE decltype(auto) operator[](size_t i) { return coeff(i); }
    ENOKI_INLINE decltype(auto) operator[](size_t i) const { return coeff(i); }
    ENOKI_INLINE decltype(auto) operator()(size_t i, size_t j) { return coeff(j, i); }
    ENOKI_INLINE decltype(auto) operator()(size_t i, size_t j) const { return coeff(j, i); }

    template<typename Input>
    ENOKI_INLINE size_t set(const Input& input, size_t index)
    {
        using Value = Input;
        using Index = uint32_array_t<Input>; // check type is okay?
   
        // 1. copy value and indexing of column (row)
        Value values = input;
        Index arangs = arange<Index>(input.size());

        // 2.  check nonzero values
        auto active = neq(input, 0.f);

        // 3. accumlate nonzero value and make it to index
        auto accums = (psum(full<Index>(1)&&active) - 1) + index;

        // 4. scatter to storage value and index
        scatter(m_data.value, values, accums, active);
        scatter(m_data.index, arangs, accums, active);

        // 5. return counting
        return scalar_cast(count(active));
    }

    template<typename InputInterator>
    ENOKI_INLINE void set_from_vectors(const InputInterator& begin, const InputInterator& end)
    {
        using Index = StorageIndex;

        // 1. reserve memory
        reserve(begin, end);

        // 2. iteratively store nonzero values
        size_t count = 0, index = 0;
        for(InputInterator iter(begin); iter!=end; ++iter)
        {
            auto values = (*iter);
            scatter(m_ptr, full<Index>(count), full<Index>(index));
            count += set(values, count);
            index++;
        }

        // 3. save the last one @@todo optimize
        scatter(m_ptr, full<Index>(count), full<Index>(index));
    }

    template<typename... Column>
    ENOKI_INLINE void from_cols(const Column&... cols) 
    {
        using Index = StorageIndex;

        // 1. reserve memory
        reserve(cols...);

        // 2. iteratively store nonzero values
        size_t count = 0, index = 0;
        for(auto iter:{cols...})
        {
            auto values = (iter);
            scatter(m_ptr, full<Index>(count), full<Index>(index));
            count += set(values, count);
            index++;
        }

        // 3. save the last one @@todo optimize
        scatter(m_ptr, full<Index>(count), full<Index>(index));
    }

private:
    template<typename InputInterator>
    ENOKI_INLINE void reserve(const InputInterator& begin, const InputInterator& end)
    {
        // 0. counting
        size_t n_reserve = 0;
        for(InputInterator iter(begin); iter!=end; ++iter)
        {
            auto nnz = neq(*iter, 0.f);
            n_reserve += scalar_cast(count_nested(nnz));
        }
        // 1. allocate data values and indices
        set_slices(m_data, n_reserve);
        m_data = empty<Storage>(n_reserve); 

        // 2. allocate starting nonzero indices
        set_slices(m_ptr, m_rows_size+1);
        m_ptr = empty<StorageIndex>(m_rows_size+1);
    }

    template<typename... Column>
    ENOKI_INLINE void reserve(const Column&... cols)
    {
        // 0. counting
        size_t n_reserve = 0;
        for(auto& col:{cols...})
        {
            auto nnz = neq(col, 0.f);
            n_reserve += scalar_cast(count_nested(nnz));
        }
        // 1. allocate data values and indices
        set_slices(m_data, n_reserve);
        m_data = empty<Storage>(n_reserve); 

        // 2. allocate starting nonzero indices
        set_slices(m_ptr, m_rows_size+1);
        m_ptr = empty<StorageIndex>(m_rows_size+1);
    }
};

// operator overloading

template <typename T0, typename T1>
SparseMatrix<expr_t<T0, T1>> operator+(const SparseMatrix<T0> &m0, const SparseMatrix<T1> &m1)
{
    using Column = expr_t<T0, T1>;
    using Result = SparseMatrix<expr_t<T0, T1>>;

    assert(m0.n_rows()==m1.n_rows() && m0.n_cols()==m1.n_rows());
    Result result(m0.n_rows(), m1.n_cols());

    std::vector<Column> columns(m0.n_rows(), zero<Column>(m0.n_cols()));
    for(size_t i=0; i<m0.n_rows(); i++)
    {
        auto sum = m0.coeff(i) + m1.coeff(i);
        columns[i] = sum;
    }
    result.set_from_vectors(columns.begin(), columns.end());
    return result;
}

// template <typename T0, typename T1>
// SparseMatrix<expr_t<T0, T1>> operator-(const SparseMatrix<T0> &m0, const SparseMatrix<T1> &m1)
// {
//     return (m0 + (-m1));
// }


template <typename T0, typename T1>
SparseMatrix<expr_t<T0, T1>> operator*(const SparseMatrix<T0> &m0, const SparseMatrix<T1> &m1)
{
    using Column = expr_t<T0, T1>;
    using Result = SparseMatrix<expr_t<T0, T1>>;

    // col, row
    assert(m0.n_rows()==m1.n_cols());
    Result result(m0.n_rows(), m1.n_cols());

    std::vector<Column> columns(m1.n_rows(), zero<Column>(m0.n_cols()));
    for(size_t j=0; j<m0.n_rows(); j++)
    {
        auto sum = m0.coeff(0) * m1(0, j);
        for(size_t i=1; i<m1.n_rows(); i++)
        {
            sum = fmadd(m0.coeff(i), m1(i, j), sum);
        }
        columns[j] = sum;
    }
    result.set_from_vectors(columns.begin(), columns.end());
    return result;
}

//@@todo
template <typename T0, typename T1, enable_if_array_t<T1> = 0>
ENOKI_INLINE auto operator*(const SparseMatrix<T0>& m, const T1& s)
{
    // if constexpr(T1::IsVector)
    // {
    assert(m.n_cols()==s.size());

    auto sum = m.coeff(0) * s.coeff(0);
    for(size_t i=1; i<s.size(); ++i)
    {
        sum = fmadd(m.coeff(i), s.coeff(i), sum);
    }
    return sum;
    // }
}

template <typename T0, typename T1, enable_if_std_type_v<T1> = 0>
ENOKI_INLINE auto operator*(const SparseMatrix<T0> &m, const T1& s)
{
    using EValue = expr_t<T0, T1>;
    using ESparseMatrix = SparseMatrix<EValue>;

    SparseMatrix<EValue> result(m);
    result.m_data.value *= s;
    return result;
}

template <typename T0, typename T1>
ENOKI_INLINE auto operator/(const SparseMatrix<T0> &m, const T1& s)
{
    using EValue = expr_t<T0, T1>;
    using ESparseMatrix = SparseMatrix<EValue>;

    SparseMatrix<EValue> result(m);
    result.m_data.value *= rcp(s);
    return result;
}

// Matrix<FloatD, 6> matrix 
// (
//     10.f, 0.f,0.f, 0.f, -2.f, 0.f,
//     3.f, 9.f, 0.f, 0.f, 0.f, 3.f ,
//     0.f, 7.f, 8.f, 7.f, 0.f, 0.f ,
//     3.f, 0.f, 8.f, 7.f, 5.f, 0.f ,
//     0.f, 8.f, 0.f, 9.f, 9.f, 13.f,
//     0.f, 4.f, 0.f, 0.f, 2.f, -1.f
// );
}
