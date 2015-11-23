#ifndef IQMOL_UTIL_NUMERICAL_H
#define IQMOL_UTIL_NUMERICAL_H
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

#include <cmath>


namespace IQmol {
namespace Util {

   inline int round(double d) {
      return (int)floor(d+0.5);
   }

   inline bool isEven(int i) {
      return ( (i % 2) == 0);
   }

   inline bool isOdd(int i) {
      return ( (i % 2) == 1);
   }

} } // end namespace IQmol::Util

#endif
