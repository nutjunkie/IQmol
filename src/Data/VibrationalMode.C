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

#include "VibrationalMode.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID VibrationalModeList::TypeID = Type::VibrationalModeList;

void VibrationalMode::dump() const
{
   qDebug() << "  Mode:" << m_frequency << "cm^-1   " << m_intensity 
            << m_irActive << m_ramanActive;
   for (int i = 0; i < m_eigenvector.size(); ++i) {
       qglviewer::Vec v(m_eigenvector[i]); 
       qDebug() << "    " << v.x << v.y << v.z;
   }
}

} } // end namespace IQmol::Data
