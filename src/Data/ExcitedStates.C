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

#include "ExcitedStates.h"
#include <QDebug>


namespace IQmol {
namespace Data {


QString ExcitedStates::typeLabel() const
{
   QString type("Unknown");
   switch (m_type) {
      case CIS:    type = "CIS/TDA";   break;
      case CISD:   type = "CIS(D)";    break;
      case TDDFT:  type = "RPA/TDDFT"; break;
      case EOM:    type = "EOM";       break;
   }
   return type;
}

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


QList<Amplitude> ExcitedStates::amplitudes(unsigned const transition) const
{
   QList<Amplitude> amplitudes;

   if ((int)transition < m_transitions.size()) {
      amplitudes = m_transitions[transition]->amplitudes();

//      unsigned nAlpha(m_orbitalSymmetries.nAlpha());
//      unsigned nBeta(m_orbitalSymmetries.nBeta());
      
// off by one
      QList<Amplitude>::iterator iter;
      for (iter = amplitudes.begin(); iter != amplitudes.end(); ++iter) {
          unsigned i((*iter).m_i);
          unsigned a((*iter).m_a);
          Spin spin((*iter).m_spin);
//          a += (spin == Alpha) ? nAlpha : nBeta;
          (*iter).m_ei = m_orbitalSymmetries.energy(spin, i-1);
          (*iter).m_ea = m_orbitalSymmetries.energy(spin, a-1);
      }
   } 

   return amplitudes;
}


void ExcitedStates::setCisdEnergies(QList<double> const& singlets, 
   QList<double> const& triplets) 
{
   if (triplets.isEmpty() && singlets.size() == m_transitions.size()) {
      // Unrestricted case states should be in the same order
      for (int i = 0; i < m_transitions.size(); ++i) {
          m_transitions[i]->setEnergy(singlets[i]);
      }
      m_type = CISD;
   }else if (singlets.size() + triplets.size() == m_transitions.size()) {
      int singlet(0);
      int triplet(0);
      double s2;
      for (int i = 0; i < m_transitions.size(); ++i) {
          // Restricted case, S^2 is either 0.0 or 2.0
          s2 = m_transitions[i]->spinSquared();
          if (s2 < 1.0) {
             m_transitions[i]->setEnergy(singlets[singlet]);
             ++singlet;
          }else {
             m_transitions[i]->setEnergy(triplets[triplet]);
             ++triplet;
          }
      }
      m_type = CISD;
   }
}


void ExcitedStates::dump() const
{
   qDebug() << "  Electronic States:";
   m_transitions.dump();
   m_orbitalSymmetries.dump();
}

} } // end namespace IQmol::Data
