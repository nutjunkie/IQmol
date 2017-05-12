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


// Restricted
Orbitals::Orbitals(
   unsigned const nAlpha, 
   unsigned const nBeta, 
   ShellList const& shells,
   QList<double> const& alphaCoefficients, 
   QString const& label)
 : m_nAlpha(nAlpha), m_nBeta(nBeta), m_nBasis(0), m_nOrbitals(0), m_restricted(true),
   m_label(label), m_shellList(shells)
{
   if (m_shellList.isEmpty()) {
      QLOG_WARN() << "Empty shell list in Orbitals constructor";  return;
   }

   m_nBasis    = m_shellList.nBasis();
   m_nOrbitals = alphaCoefficients.size() / m_nBasis;
   m_shellList.boundingBox(m_bbMin, m_bbMax);

   m_alphaCoefficients.resize(m_nOrbitals, m_nBasis);
   unsigned ka(0);
   for (unsigned i = 0; i < m_nOrbitals; ++i) {
       for (unsigned j = 0; j < m_nBasis; ++j, ++ka) {
           m_alphaCoefficients(i,j) = alphaCoefficients[ka];
       }
   }
}


// Unrestricted
Orbitals::Orbitals(
   unsigned const nAlpha, 
   unsigned const nBeta, 
   ShellList const& shells,
   QList<double> const& alphaCoefficients, 
   QList<double> const& betaCoefficients,
   QString const& label)
 : m_nAlpha(nAlpha), m_nBeta(nBeta), m_nBasis(0), m_nOrbitals(0), m_restricted(false),
   m_label(label), m_shellList(shells)
{
   if (m_shellList.isEmpty()) {
      QLOG_WARN() << "Empty shell list in Orbitals constructor";  return;
   }else if (alphaCoefficients.size() != betaCoefficients.size()) {
      QLOG_WARN() << "Inconsistent coefficient matrix dimensions in Orbitals constructor:";  
      QLOG_WARN() << "  Alpha:" << alphaCoefficients.size() 
                  << "  Beta:"  << betaCoefficients.size();
      return;
   }

   m_nBasis    = m_shellList.nBasis();
   m_nOrbitals = alphaCoefficients.size() / m_nBasis;
   m_shellList.boundingBox(m_bbMin, m_bbMax);

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
}


bool Orbitals::consistent() const 
{ 
   return
   m_nOrbitals > 0 && 
   m_nBasis    > 0 && 
   m_nOrbitals <= m_nBasis     && 
   m_nAlpha    <= m_nOrbitals  &&
   m_nBeta     <= m_nOrbitals;
}


Matrix const& Orbitals::alphaCoefficients() const 
{ 
   return m_alphaCoefficients; 
}


Matrix const& Orbitals::betaCoefficients()  const 
{ 
   return restricted() ? m_alphaCoefficients :  m_betaCoefficients;
}


ShellList& Orbitals::shellList() 
{ 
   return m_shellList; 
}


void Orbitals::dump() const 
{
   qDebug() << "There are  " << m_nAlpha << "alpha and" << m_nBeta << "beta electrons";
   qDebug() << "There are  " << m_nBasis << "basis functions and" << m_nOrbitals << "orbitals";
   qDebug() << "There ares " << m_shellList.size() << "shells";

   m_shellList.dump();
}

} } // end namespace IQmol::Data
