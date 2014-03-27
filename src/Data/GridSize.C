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

#include "GridSize.h"
#include "QsLog.h"
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace Data {

double GridSize::stepSize(unsigned const quality)
{
   // These spacings are chosen so that each step uses roughly four times as
   // many points as the previous one.
   double stepSize(0.0);
   switch (quality) {
      case 0:   stepSize = 1.000000;  break;  // not used
      case 1:   stepSize = 0.629961;  break;
      case 2:   stepSize = 0.396850;  break;
      case 3:   stepSize = 0.250000;  break;  // default
      case 4:   stepSize = 0.157490;  break;
      case 5:   stepSize = 0.099213;  break;
      case 6:   stepSize = 0.062500;  break;
      case 7:   stepSize = 0.039373;  break;
      default:  stepSize = 0.250000;  break;
   }
   return stepSize;
}


GridSize::GridSize(qglviewer::Vec const& origin, qglviewer::Vec const& delta, 
   unsigned const nx, unsigned const ny, unsigned const nz) : m_origin(origin), 
   m_delta(delta), m_nx(nx), m_ny(ny), m_nz(nz)
{
}
            

GridSize::GridSize(qglviewer::Vec const& min, qglviewer::Vec const& max, unsigned const quality)
{
   double d(stepSize(quality));
   m_origin = min;
   m_delta.setValue(d, d, d);

   qglviewer::Vec delta(max-min);
   delta /= d;

   m_nx = std::ceil(delta.x);
   m_ny = std::ceil(delta.y);
   m_nz = std::ceil(delta.z);
}


qglviewer::Vec GridSize::max() const
{
   qglviewer::Vec ofSpecies((m_nx-1)*m_delta.x, (m_ny-1)*m_delta.y, (m_nz-1)*m_delta.z);
   return m_origin + ofSpecies;
}


bool GridSize::operator==(GridSize const& that) const
{
   if (m_nx != that.m_nx  ||  m_ny != that.m_ny  ||  m_nz != that.m_nz) return false;
   
   double thresh(10e-6);
   qglviewer::Vec delta (m_delta  - that.m_delta );
   qglviewer::Vec origin(m_origin - that.m_origin);

   return delta.norm() < thresh && origin.norm() < thresh;
}


bool GridSize::operator!=(GridSize const& that) const
{
   return  !(*this == that);
}


bool GridSize::operator<(GridSize const& that) const
{
   return m_delta.norm() < that.m_delta.norm();
}


void GridSize::dump() const
{
   qglviewer::Vec max(m_nx*m_delta.x, m_ny*m_delta.y, m_nz*m_delta.z);
   max += m_origin;

   QLOG_TRACE() << "GridSize with" << m_nx*m_ny*m_nz << "points";
   QLOG_TRACE() << "    limits: x" << m_origin.x << "to" << max.x << m_nx << m_delta.x;
   QLOG_TRACE() << "    limits: y" << m_origin.y << "to" << max.y << m_ny << m_delta.y;
   QLOG_TRACE() << "    limits: z" << m_origin.z << "to" << max.z << m_nz << m_delta.z;
}

} } // end namespace IQmol::Data
