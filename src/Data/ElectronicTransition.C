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

#include "ElectronicTransition.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID ElectronicTransitionList::TypeID = Type::ElectronicTransitionList;

bool ElectronicTransition::addAmplitude(QList<double> &list,
    QList<int> &indexJ, QList<int> &indexI,
    unsigned const state, unsigned const NO, unsigned const NV, Spin spin)
{
   double thresh(15.0/100.0), amp(0.0);
   int occ(0), vir(0);
   for (int k = indexJ[state-1] ; k < indexJ[state] ; k++) { 
     amp = list[k]; 
     if (std::fabs(amp) >= thresh) {
       vir = (indexI[k] % NV) + 1;
       occ = (int)((indexI[k]+1) / NV);
       //qDebug() << "Adding amplitude:" << occ << "->" << NO+vir << "     " << amp;
       m_amplitudes.append(Amplitude(spin, occ, NO+vir, amp, spin));
     }
   }
}
bool ElectronicTransition::addAmplitude(QList<double> &list, unsigned const state,
    unsigned const NO, unsigned const NV, Spin spin)
{
   double thresh(15.0/100.0);

   QList<double>::iterator amp = list.begin() + (state-1)*NO*NV;
   for (int i = 0 ; i < NO; i++) {
      for (int a = 0; a < NV; a++) {
         if (std::fabs(*amp) >= thresh) {
           //qDebug() << "Adding amplitude:" << i+1 << "->" << NO+a+1 << "     " << *amp;
           m_amplitudes.append(Amplitude(spin, i+1, NO+a+1, *amp, spin));
         }
         ++amp;
      } 
   }
   return true;
}

bool ElectronicTransition::addAmplitude(QStringList const& list, 
   unsigned const nAlpha, unsigned const nBeta)
{
   
   if (list.size() < 3) return false;

   bool ok;

   unsigned i(list[1].toUInt(&ok));
   if (!ok) return false;
   unsigned a(list[3].toUInt(&ok));
   if (!ok) return false;
   double amplitude(list[4].toDouble(&ok));
   if (!ok) return false;
      
   Spin spin(Alpha);
   if (list.size() == 6 && list[5] == "b") spin = Beta;

   int singlyOccupied(nAlpha-nBeta);
   int doublyOccupied(nBeta);

   if (list[0] == "S") i += doublyOccupied;
   if (list[2] == "S") a += doublyOccupied;
   if (list[2] == "V") a += doublyOccupied+singlyOccupied;

   //qDebug() << "Adding amplitude:" << i << "->" << a << "     " << list;
   //("D", "8", "V", "2", " 0.1506", "b")

   m_amplitudes.append(Amplitude(spin, i, a, amplitude, spin));
   
   return true;
}


void ElectronicTransition::dump() const
{
   qDebug() << "  Transition:" << m_energy << "eV   " << m_strength << " <S^2> = " << m_spinSquared;
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
