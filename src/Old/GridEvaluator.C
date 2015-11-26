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

#include "GridEvaluator.h"
#include "GridData.h"


namespace IQmol {

GridEvaluator::GridEvaluator(Data::GridData& grid, Function3D const& function) 
  : m_grid(grid), m_function(function)
{
   unsigned nx, ny, nz;
   m_grid.getNumberOfPoints(nx, ny, nz);
   m_totalProgress = nx;
}

void GridEvaluator::run()
{
   unsigned nx, ny, nz;
   m_grid.getNumberOfPoints(nx, ny, nz);

   qglviewer::Vec origin(m_grid.origin());
   qglviewer::Vec delta(m_grid.delta());

   double x(origin.x);
   for (unsigned i = 0; i < nx; ++i, x += delta.x) {
       double y(origin.y);
       for (unsigned j = 0; j < ny; ++j, y += delta.y) {
           double z(origin.z);
           for (unsigned k = 0; k < nz; ++k, z += delta.z) {
               m_grid(i, j, k) = m_function(x, y, z);
           }
       }
       progress(i);
       if (m_terminate) break;
   }
}

} // end namespace IQmol
