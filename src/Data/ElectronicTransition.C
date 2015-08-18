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


bool ElectronicTransition::addAmplitude(QStringList const& list)
{
   if (list.size() < 3) return false;

   bool ok;

   unsigned i(list[0].toUInt(&ok));
   if (!ok) return false;
   unsigned a(list[1].toUInt(&ok));
   if (!ok) return false;
   double amplitude(list[2].toDouble(&ok));
   if (!ok) return false;
      
   Spin spin(Alpha);
   if (list.size() == 4 && list[3] == "b") spin = Beta;
   m_amplitudes.append(Amplitude(spin, i, a, amplitude, spin));
   
   return true;
}


void ElectronicTransition::dump() const
{
   qDebug() << "  Transition:" << m_energy << "eV   " << m_strength;
   qDebug() << "    " << m_transitionMoment.x << m_transitionMoment.y << m_transitionMoment.z;
   qDebug() << "    Amplitudes size" << m_amplitudes.size();

   QList<Amplitude>::const_iterator iter;
   for (iter = m_amplitudes.begin(); iter != m_amplitudes.end(); ++iter) {
       (*iter).dump();
   }
}


void Amplitude::dump() const
{
   qDebug() << SpinLabel(m_spin) << m_i << "-->" << m_a << "  (" << m_amplitude << ")"; 
}

} } // end namespace IQmol::Data
