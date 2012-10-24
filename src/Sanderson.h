#ifndef IQMOL_SANDERSON_H
#define IQMOL_SANDERSON_H
/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include <QList>
#include "QGLViewer/vec.h"


namespace IQmol {

   /// A three-dimensional cartesian grid containing data.  Two auxiliary
   /// classes, DataType and Size, hold information about the type of data
   /// held by the grid.
   class Sanderson {

      public:
         Sanderson(QList<int> const& atomicNumbers, QList<qglviewer::Vec> const& coordinates,
           int const molecularCharge = 0) : m_atomicNumbers(atomicNumbers), 
           m_coordinates(coordinates), m_molecularCharge(molecularCharge) { }
            
         QList<double> solve();

      private:
         // These are MP2/6-311++G energies
         double hardness(int const Z) const;
         double electronegativity(int const Z) const;
         static const unsigned int s_maxAtomicNumber;
         static const double s_energies[][3];
         static const double s_[][3];
         QList<int> m_atomicNumbers;
         QList<qglviewer::Vec> m_coordinates;
         int m_molecularCharge;
   };


} // end namespace IQmol


#endif
