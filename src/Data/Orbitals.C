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

#include "Orbitals.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Data {

QString Orbitals::toString(OrbitalType const type)
{
   QString s;

   switch (type) {
      case Undefined:          s = "Undefined";                    break;
      case Canonical:          s = "Canonical Orbitals";           break;
      case Localized:          s = "Localized Orbitals";           break;
      case NaturalTransition:  s = "Natural Transition Orbitals";  break;
      case NaturalBond:        s = "Natural Bond Orbitals";        break;
   }

   return s;
}


Orbitals::Orbitals() : m_nAlpha(0), m_nBeta(0), m_nBasis(0), m_nOrbitals(0)
{
}


Orbitals::Orbitals(
   unsigned const nAlpha, 
   unsigned const nBeta, 
   QList<double> const& alphaCoefficients, 
   QList<double> const& betaCoefficients, 
   ShellList const& shells, 
   QString const& label) :
   m_label(label), m_nAlpha(nAlpha), m_nBeta(nBeta), m_shellList(shells)
{
   if (m_shellList.isEmpty()) {
      QLOG_WARN() << "Empty shell list in Orbitals constructor";
      return;
   }

   m_nBasis = m_shellList.nBasis();
   
   unsigned nao(alphaCoefficients.size() / m_nBasis);
   unsigned nbo(betaCoefficients.size() / m_nBasis);

   if (nao == 0 || nao != nbo ||
       nao*m_nBasis != alphaCoefficients.size() ||
       nbo*m_nBasis != betaCoefficients.size() ) {
      QLOG_WARN() << "Inconsistent coefficient data in Orbitals constructor:";
      QLOG_WARN() << "Basis functions:" << m_nBasis << "orbitals:" << nao << nbo;
      QLOG_WARN() << "Coefficients:   " << alphaCoefficients.size() << betaCoefficients.size();
      m_nOrbitals = 0;
      return;
   }

   m_nOrbitals = nao;

   m_alphaCoefficients.resize(m_nOrbitals, m_nBasis);
   unsigned ka(0);
   for (unsigned i = 0; i < m_nOrbitals; ++i) {
       for (unsigned j = 0; j < m_nBasis; ++j, ++ka) {
           m_alphaCoefficients(i,j) = alphaCoefficients[ka];
       }
   }

   m_betaCoefficients.resize(m_nOrbitals, m_nBasis);
   unsigned kb(0);
   for (unsigned i = 0; i < m_nOrbitals; ++i) {
       for (unsigned j = 0; j < m_nBasis; ++j, ++kb) {
           m_betaCoefficients(i,j) = betaCoefficients[kb];
       }
   }

   m_shellList.boundingBox(m_bbMin, m_bbMax);
}



void Orbitals::dump() const 
{
   qDebug() << "There are  " << m_nAlpha << "alpha and" << m_nBeta << "beta electrons";
   qDebug() << "There are  " << m_nBasis << "basis functions and" << m_nOrbitals << "orbitals";
   qDebug() << "There ares " << m_shellList.size() << "shells";

   m_shellList.dump();
}

} } // end namespace IQmol::Data
