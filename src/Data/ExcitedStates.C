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

#include "ExcitedStates.h"
#include <QDebug>


namespace IQmol {
namespace Data {


double ExcitedStates::maxEnergy() const
{
   double max(0.0);

   ElectronicTransitionList::const_iterator iter;
   for (iter = m_transitions.begin(); iter != m_transitions.end(); ++iter) {
       if ((*iter)->energy() > max) max = (*iter)->energy();
   }

   return max; 
}


double ExcitedStates::maxIntensity() const
{
   double max(0.0);

   ElectronicTransitionList::const_iterator iter;
   for (iter = m_transitions.begin(); iter != m_transitions.end(); ++iter) {
       if ((*iter)->strength() > max) max = (*iter)->strength();
   }

   return max; 
}


void ExcitedStates::dump() const
{
   qDebug() << "  Electronic States:";
   m_transitions.dump();
}

} } // end namespace IQmol::Data
