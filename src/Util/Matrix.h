#ifndef IQMOL_UTIL_MATRIX_H
#define IQMOL_UTIL_MATRIX_H
/*******************************************************************************

  Copyright (C) 2011-2015 Andrew Gilbert

  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.

  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************/

#include "boost/numeric/ublas/matrix.hpp"
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector.hpp"
#include "boost/multi_array.hpp"
//#include <boost/serialization/serialization.hpp>

#include <QStringList>


namespace IQmol {

typedef boost::numeric::ublas::matrix<double>         Matrix;
typedef boost::numeric::ublas::vector<double>         Vector;
typedef boost::numeric::ublas::matrix_column<Matrix const> MatrixColumn;
typedef boost::numeric::ublas::matrix_row<Matrix const> MatrixRow;
typedef boost::multi_array<double, 3> Array3D;
typedef boost::multi_array<double, 4> Array4D;

template <size_t N>
struct Array {
    typedef boost::multi_array<double, N> type;
};

// Invoke:
// Array<size_t>::type  myArray;

QStringList PrintMatrix(Matrix const&, unsigned const columns = 6);
QString PrintVector(Vector const&);

} // end namespace IQmol

#endif
