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

#include "ElectronicTransition.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID ElectronicTransitionList::TypeID = Type::ElectronicTransitionList;

void ElectronicTransition::dump() const
{
   qDebug() << "  Transition:" << m_energy << "eV   " << m_strength;
   qDebug() << "    " << m_transitionMoment.x << m_transitionMoment.y << m_transitionMoment.z;
}

} } // end namespace IQmol::Data
