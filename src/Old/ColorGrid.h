#ifndef IQMOL_COLORGRID_H
#define IQMOL_COLORGRID_H
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

#include "ColorGradient.h"
   

namespace IQmol {

   namespace Data {
      class GridData;
   }
   
   /// Essentially a Grid-based function that returns a color for a given
   /// position.  Used for coloring surfaces.
   class ColorGrid {
   
      public:
         ColorGrid(Data::GridData*, ColorGradient::Function const&);
         void setGradient(ColorGradient::Function const& gradient) { m_gradient = gradient; }
         QColor operator()(double const x, double const y, double const z) const;

      private:
         Data::GridData* m_grid;
         ColorGradient::Function m_gradient;
         double m_min;
         double m_max;
   };

} // end namespace IQmol

#endif
