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

#include "CanonicalOrbitals.h"
#include <QDebug>


namespace IQmol {
namespace Data {


CanonicalOrbitals::CanonicalOrbitals(
   unsigned const nAlpha, 
   unsigned const nBeta, 
   ShellList const& shells, 
   QList<double> const& alphaCoefficients, 
   QList<double> const& alphaEnergies,  
   QList<double> const& betaCoefficients,  
   QList<double> const& betaEnergies,
   QString const& label)
 : Orbitals(Orbitals::Canonical, shells, alphaCoefficients, betaCoefficients, label), 
   m_nAlpha(nAlpha), m_nBeta(nBeta), 
   m_alphaEnergies(alphaEnergies), m_betaEnergies(betaEnergies)
{
}


double CanonicalOrbitals::alphaOrbitalEnergy(unsigned i) const 
{
   return ((int)i < m_alphaEnergies.size()) ? m_alphaEnergies[i] : 0.0;
}


double CanonicalOrbitals::betaOrbitalEnergy(unsigned i) const 
{
   double energy(0.0);
   if (m_restricted) {
      energy = alphaOrbitalEnergy(i);
   }else if ((int)i < m_betaEnergies.size()) {
      energy = m_betaEnergies[i];
   }
   return energy;
}


bool CanonicalOrbitals::consistent() const
{
   bool ok(Orbitals::consistent());

   ok = ok && m_nAlpha <= m_nOrbitals;
   ok = ok && m_nBeta  <= m_nOrbitals;

   ok = ok && m_alphaEnergies.size() == (int)m_nOrbitals;
   if (!m_restricted) ok = ok && m_betaEnergies.size() == (int)m_nOrbitals;
      
   return ok; 
}


QString CanonicalOrbitals::label(unsigned index, bool alpha) const
{
   unsigned n(alpha ? m_nAlpha : m_nBeta);
   QString  s(alpha ? "Alpha " : "Beta ");
   s += QString::number(index+1);

   if (n == index+2) {
      s += " (HOMO-1)";
   }else if (n == index+1) {
      s += " (HOMO)";
   }else if (n == index) {
      s += " (LUMO)";
   }else if (index == n+1) {
      s += " (LUMO+1)";
   }

   return s;
} 


unsigned CanonicalOrbitals::labelIndex(bool const alpha) const
{
   int n(alpha ? m_nAlpha : m_nBeta);
   return std::max(0,n-1);
}



void CanonicalOrbitals::dump() const
{
   Orbitals::dump();
}

} } // end namespace IQmol::Data
