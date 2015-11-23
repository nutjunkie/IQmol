#ifndef IQMOL_DATA_GRIDSIZE_H
#define IQMOL_DATA_GRIDSIZE_H
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

#include "QGLViewer/vec.h"


namespace IQmol {
namespace Data {

   class GridSize {

      public:

         GridSize(qglviewer::Vec const& origin, qglviewer::Vec const& delta,
            unsigned const nx, unsigned const ny, unsigned const nz);

         GridSize(qglviewer::Vec const& min, qglviewer::Vec const& max, 
            unsigned const quality);

         qglviewer::Vec const& delta() const { return m_delta; }
         qglviewer::Vec const& origin() const { return m_origin; }
         qglviewer::Vec max() const;

         unsigned nx() const { return m_nx; }
         unsigned ny() const { return m_ny; }
         unsigned nz() const { return m_nz; }

         bool operator==(GridSize const& that) const;
         bool operator!=(GridSize const& that) const;
         bool operator<(GridSize const& that) const;

         void dump() const;

         static double stepSize(unsigned const quality);

      private:
         qglviewer::Vec m_origin;
         qglviewer::Vec m_delta;
         unsigned m_nx;
         unsigned m_ny;
         unsigned m_nz;
   };

} } // end namespace IQmol::Data

#endif
