/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
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

#include "ColorGrid.h"
#include "GridData.h"
#include <cmath>


namespace IQmol {

ColorGrid::ColorGrid(Data::GridData* grid, Gradient::Function const& gradient)
  : m_grid(grid), m_gradient(gradient)
{
   m_grid->getRange(m_min, m_max);

   // For data that straddles zero, we assume that we want the scale 
   // to go the same amount in the positive and negative directions.
   if (m_min < 0.0 && m_max > 0.0) {
      m_max = std::max(-m_min, m_max);
      m_min = -m_max;
   }
}


QColor ColorGrid::operator()(double const x, double const y, double const z) const
{
   double value((*m_grid)(x, y, z, true));
   value = (value - m_min) / (m_max-m_min);
   return m_gradient.colorAt(value);
}

} // end namespace IQmol
