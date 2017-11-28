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

#include "NaturalTransitionOrbitals.h"
#include <cmath>


namespace IQmol {
namespace Data {


NaturalTransitionOrbitals::NaturalTransitionOrbitals(
   ShellList const& shells, 
   QList<double> const& alphaCoefficients, 
   QList<double> const& alphaOccupancies,  
   QList<double> const& betaCoefficients,  
   QList<double> const& betaOccupancies,
   QString const& label)
 : Orbitals(Orbitals::NaturalTransition, shells, 
      alphaCoefficients, betaCoefficients, label), 
   m_alphaOccupancies(alphaOccupancies), m_betaOccupancies(betaOccupancies)
{
}


double NaturalTransitionOrbitals::alphaOccupancy(unsigned i) const 
{
   return ((int)i < m_alphaOccupancies.size()) ? m_alphaOccupancies[i] : 0.0;
}


double NaturalTransitionOrbitals::betaOccupancy(unsigned i) const 
{
   double energy(0.0);
   if (m_restricted) {
      energy = alphaOccupancy(i);
   }else if ((int)i < m_betaOccupancies.size()) {
      energy = m_betaOccupancies[i];
   }
   return energy;
}


bool NaturalTransitionOrbitals::consistent() const
{
   bool ok(Orbitals::consistent());

   ok = ok && m_alphaOccupancies.size() == (int)m_nOrbitals;
   if (!m_restricted) ok = ok && m_betaOccupancies.size() == (int)m_nOrbitals;

   return ok; 
}


QStringList NaturalTransitionOrbitals::labels(bool const alpha) const
{
   QStringList list(Orbitals::labels(alpha));
   // Half the orbitals correspond to occupieds, half virtuals.
   if (list.size() < 2) return list;

   unsigned n(list.size()/2);
   list[n-1] += " (HONTO)";
   list[n]   += " (LUNTO)";

   if (list.size() < 4) return list;
   list[n-2] += " (HONTO-1)";
   list[n+1] += " (LUNTO+1)";

   //for (unsigned i = 0; i < n-2; ++i)            list[i] += " (occ)";
   //for (unsigned i = n+2; i < list.size(); ++i)  list[i] += " (virt)";
       
   return list;
}

unsigned NaturalTransitionOrbitals::labelIndex(bool const alpha) const
{
   int n(nOrbitals()/2);
   return std::max(0, n-1);
}


void NaturalTransitionOrbitals::dump() const
{
   Orbitals::dump();
}

} } // end namespace IQmol::Data
