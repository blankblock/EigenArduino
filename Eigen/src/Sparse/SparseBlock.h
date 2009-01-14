// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2008 Gael Guennebaud <g.gael@free.fr>
// Copyright (C) 2008 Daniel Gomez Ferro <dgomezferro@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_SPARSE_BLOCK_H
#define EIGEN_SPARSE_BLOCK_H

template<typename MatrixType>
struct ei_traits<SparseInnerVector<MatrixType> >
{
  typedef typename ei_traits<MatrixType>::Scalar Scalar;
  enum {
    IsRowMajor = (int(MatrixType::Flags)&RowMajorBit)==RowMajorBit,
    Flags = MatrixType::Flags,
    RowsAtCompileTime = IsRowMajor ? 1 : MatrixType::RowsAtCompileTime,
    ColsAtCompileTime = IsRowMajor ? MatrixType::ColsAtCompileTime : 1,
    CoeffReadCost = MatrixType::CoeffReadCost
  };
};

template<typename MatrixType>
class SparseInnerVector : ei_no_assignment_operator,
  public SparseMatrixBase<SparseInnerVector<MatrixType> >
{
    enum {
      IsRowMajor = ei_traits<SparseInnerVector>::IsRowMajor
    };
public:

    EIGEN_SPARSE_GENERIC_PUBLIC_INTERFACE(SparseInnerVector)
    class InnerIterator;

    inline SparseInnerVector(const MatrixType& matrix, int outer)
      : m_matrix(matrix), m_outer(outer)
    {
      ei_assert( (outer>=0) && (outer<matrix.outerSize()) );
    }

    EIGEN_STRONG_INLINE int rows() const { return IsRowMajor ? 1 : m_matrix.rows(); }
    EIGEN_STRONG_INLINE int cols() const { return IsRowMajor ? m_matrix.cols() : 1; }

  protected:

    const typename MatrixType::Nested m_matrix;
    int m_outer;

};

template<typename MatrixType>
class SparseInnerVector<MatrixType>::InnerIterator : public MatrixType::InnerIterator
{
public:
  inline InnerIterator(const SparseInnerVector& xpr, int outer=0)
    : MatrixType::InnerIterator(xpr.m_matrix, xpr.m_outer)
  {
    ei_assert(outer==0);
  }
};

/** \returns the \a outer -th column (resp. row) of the matrix \c *this if \c *this
  * is col-major (resp. row-major).
  */
template<typename Derived>
SparseInnerVector<Derived> SparseMatrixBase<Derived>::innerVector(int outer)
{ return SparseInnerVector<Derived>(derived(), outer); }

/** \returns the \a outer -th column (resp. row) of the matrix \c *this if \c *this
  * is col-major (resp. row-major). Read-only.
  */
template<typename Derived>
const SparseInnerVector<Derived> SparseMatrixBase<Derived>::innerVector(int outer) const
{ return SparseInnerVector<Derived>(derived(), outer); }

# if 0
template<typename MatrixType, int BlockRows, int BlockCols, int PacketAccess>
class Block<MatrixType,BlockRows,BlockCols,PacketAccess,IsSparse>
  : public SparseMatrixBase<Block<MatrixType,BlockRows,BlockCols,PacketAccess,IsSparse> >
{
public:

    _EIGEN_GENERIC_PUBLIC_INTERFACE(Block, SparseMatrixBase<Block>)
    class InnerIterator;

    /** Column or Row constructor
      */
    inline Block(const MatrixType& matrix, int i)
      : m_matrix(matrix),
        // It is a row if and only if BlockRows==1 and BlockCols==MatrixType::ColsAtCompileTime,
        // and it is a column if and only if BlockRows==MatrixType::RowsAtCompileTime and BlockCols==1,
        // all other cases are invalid.
        // The case a 1x1 matrix seems ambiguous, but the result is the same anyway.
        m_startRow( (BlockRows==1) && (BlockCols==MatrixType::ColsAtCompileTime) ? i : 0),
        m_startCol( (BlockRows==MatrixType::RowsAtCompileTime) && (BlockCols==1) ? i : 0),
        m_blockRows(matrix.rows()), // if it is a row, then m_blockRows has a fixed-size of 1, so no pb to try to overwrite it
        m_blockCols(matrix.cols())  // same for m_blockCols
    {
      ei_assert( (i>=0) && (
          ((BlockRows==1) && (BlockCols==MatrixType::ColsAtCompileTime) && i<matrix.rows())
        ||((BlockRows==MatrixType::RowsAtCompileTime) && (BlockCols==1) && i<matrix.cols())));
    }

    /** Fixed-size constructor
      */
    inline Block(const MatrixType& matrix, int startRow, int startCol)
      : m_matrix(matrix), m_startRow(startRow), m_startCol(startCol),
        m_blockRows(matrix.rows()), m_blockCols(matrix.cols())
    {
      EIGEN_STATIC_ASSERT(RowsAtCompileTime!=Dynamic && RowsAtCompileTime!=Dynamic,THIS_METHOD_IS_ONLY_FOR_FIXED_SIZE)
      ei_assert(startRow >= 0 && BlockRows >= 1 && startRow + BlockRows <= matrix.rows()
          && startCol >= 0 && BlockCols >= 1 && startCol + BlockCols <= matrix.cols());
    }

    /** Dynamic-size constructor
      */
    inline Block(const MatrixType& matrix,
          int startRow, int startCol,
          int blockRows, int blockCols)
      : m_matrix(matrix), m_startRow(startRow), m_startCol(startCol),
                          m_blockRows(blockRows), m_blockCols(blockCols)
    {
      ei_assert((RowsAtCompileTime==Dynamic || RowsAtCompileTime==blockRows)
          && (ColsAtCompileTime==Dynamic || ColsAtCompileTime==blockCols));
      ei_assert(startRow >= 0 && blockRows >= 1 && startRow + blockRows <= matrix.rows()
          && startCol >= 0 && blockCols >= 1 && startCol + blockCols <= matrix.cols());
    }

    inline int rows() const { return m_blockRows.value(); }
    inline int cols() const { return m_blockCols.value(); }

    inline int stride(void) const { return m_matrix.stride(); }

    inline Scalar& coeffRef(int row, int col)
    {
      return m_matrix.const_cast_derived()
               .coeffRef(row + m_startRow.value(), col + m_startCol.value());
    }

    inline const Scalar coeff(int row, int col) const
    {
      return m_matrix.coeff(row + m_startRow.value(), col + m_startCol.value());
    }

    inline Scalar& coeffRef(int index)
    {
      return m_matrix.const_cast_derived()
             .coeffRef(m_startRow.value() + (RowsAtCompileTime == 1 ? 0 : index),
                       m_startCol.value() + (RowsAtCompileTime == 1 ? index : 0));
    }

    inline const Scalar coeff(int index) const
    {
      return m_matrix
             .coeff(m_startRow.value() + (RowsAtCompileTime == 1 ? 0 : index),
                    m_startCol.value() + (RowsAtCompileTime == 1 ? index : 0));
    }

  protected:

    const typename MatrixType::Nested m_matrix;
    const ei_int_if_dynamic<MatrixType::RowsAtCompileTime == 1 ? 0 : Dynamic> m_startRow;
    const ei_int_if_dynamic<MatrixType::ColsAtCompileTime == 1 ? 0 : Dynamic> m_startCol;
    const ei_int_if_dynamic<RowsAtCompileTime> m_blockRows;
    const ei_int_if_dynamic<ColsAtCompileTime> m_blockCols;

};
#endif

#endif // EIGEN_SPARSE_BLOCK_H
