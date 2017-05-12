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

#include "MolecularOrbitals.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Data {


MolecularOrbitals::MolecularOrbitals(
   unsigned const nAlpha, 
   unsigned const nBeta, 
   QList<double> const& alphaCoefficients, 
   QList<double> const& alphaEnergies,  
   QList<double> const& betaCoefficients, 
   QList<double> const& betaEnergies,
   ShellList const& shells) : 
   Orbitals(nAlpha, nBeta, shells, alphaCoefficients, betaCoefficients),
   m_alphaEnergies(alphaEnergies),  m_betaEnergies(betaEnergies)
{
   m_restricted = (m_nAlpha == m_nBeta) && (m_alphaEnergies == m_betaEnergies);
}


bool MolecularOrbitals::consistent() const
{
   bool ok(true);
   ok = ok && m_nOrbitals > 0;
   if (orbitalType() != NaturalTransition) {
      ok = ok && m_nAlpha <= m_nOrbitals;
      ok = ok && m_nBeta  <= m_nOrbitals;
   }
   ok = ok && m_alphaEnergies.size() == (int)m_nOrbitals;
   ok = ok && m_betaEnergies.size()  == (int)m_nOrbitals;

   unsigned nBasis(0);
   ShellList::const_iterator iter;
   for (iter = m_shellList.begin(); iter != m_shellList.end(); ++iter) {
       nBasis += (*iter)->nBasis();
   }

   ok = ok && m_nBasis == nBasis;

   return ok; 
}


void MolecularOrbitals::dump() const
{
   Orbitals::dump();

   SurfaceList::const_iterator surface;
   for (surface = m_surfaceList.begin(); surface != m_surfaceList.end(); ++surface) {
       (*surface)->dump();
   }
}

} } // end namespace IQmol::Data
