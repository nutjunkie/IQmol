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

#include "GridData.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID GridList::TypeID = Type::GridList;

bool Grid::isValid() const 
{ 
   bool valid(m_data.size() == m_nx * m_ny * m_nz);
   if (!valid) {
      qDebug() << "Invalid grid dimemsions" << m_nx << m_ny << m_nz << m_nx * m_ny * m_nz
               << m_data.size();
   }
   return valid;
}

void Grid::dump() const
{
   qDebug() << "Grid data:" << m_description;
   qDebug() << "  x = " << m_min.x << m_max.x << m_nx;
   qDebug() << "  y = " << m_min.y << m_max.y << m_ny;
   qDebug() << "  z = " << m_min.z << m_max.z << m_nz;
}

} } // end namespace IQmol::Data
