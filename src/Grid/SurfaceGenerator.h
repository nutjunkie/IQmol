#ifndef IQMOL_GRID_SURFACEGENERATOR_H
#define IQMOL_GRID_SURFACEGENERATOR_H
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

#include "Task.h"


namespace IQmol {

namespace Data {
   class GridData;
   class SurfaceInfo;
   class Surface;
}

namespace Grid {


   class SurfaceGenerator : public Task {

      Q_OBJECT

      public:
         SurfaceGenerator(Data::GridData const& grid, Data::SurfaceInfo const&);
         
         Data::Surface* getSurface() const;

      protected:
         void run();

      private:
         Data::GridData const&     m_grid;
         Data::SurfaceInfo const&  m_surfaceInfo;
         Data::Surface*            m_surface;
   };

} } // end namespace IQmol::Grid

#endif
