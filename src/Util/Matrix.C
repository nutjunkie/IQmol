/*******************************************************************************
       
  Copyright (C) 2011 Andrew 2015ert
           
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

#include "Matrix.h"
#include <cmath>


namespace IQmol {

QString PrintVector(Vector const& vector)
{
   QString output;
   unsigned n(vector.size());
   for (unsigned i = 0; i < n; ++i) {
       output += QString::number(vector(i), 'f', 8);       
       output += " ";
   }
   output.chop(1);
   return output;
}


QStringList PrintMatrix(Matrix const& matrix, unsigned const columns)
{
   unsigned nColumnsPerLine(columns);
   unsigned width(12);
   unsigned digits(7);
   QString space;

   switch (columns) {
      case 3:
      case 4:
         width  = 16;
         digits = 10;
         break;
      case 5:
         width  = 14;
         digits =  8;
         break;
      case 6:
         width  = 12;
         digits =  7;
         break;
      case 7:
         width  = 10;
         digits =  5;
         space  = " ";
         break;
      case 8:
         width  =  9;
         digits =  4;
         space  = " ";
         break;
      case 9:
         width  =  8;
         digits =  3;
         space  = "  ";
         break;
      default:
         nColumnsPerLine = 6;
         break;
   }

   unsigned nRows(matrix.size1()); 
   unsigned nCols(matrix.size2()); 

   QString output;
   QStringList list;


   for (unsigned firstCol = 0; firstCol < nCols; firstCol += nColumnsPerLine) {
       unsigned lastCol(std::min(firstCol+nColumnsPerLine, nCols));

       // Column headers
       output += space;
       for (unsigned j = firstCol; j < lastCol; ++j) {
           output += QString::number(j+1).rightJustified(width);
       }

       list << output;
       output.clear();

       // Rows for the current columns
       for (unsigned i = 0; i < nRows; ++i) {
           output += QString::number(i+1).rightJustified(4);
           for (unsigned j = firstCol; j < lastCol; ++j) {
               output += QString::number(matrix(i,j), 'f', digits).rightJustified(width);
           }
           list << output;
           output.clear();
       }
   }

   return list;
}


} // end namespace IQmol
