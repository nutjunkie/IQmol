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
#include <cmath>


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
      case Dyson:              s = "Dyson Orbitals";               break;
   }

   return s;
}


Orbitals::Orbitals(
   OrbitalType const orbitalType,
   ShellList const& shellList,
   QList<double> const& alphaCoefficients, 
   QList<double> const& betaCoefficients,
   QString const& title)
 : m_orbitalType(orbitalType), m_title(title), m_nBasis(0), m_nOrbitals(0),
   m_shellList(shellList)
{
   if (m_shellList.isEmpty() || alphaCoefficients.isEmpty()) {
      QLOG_WARN() << "Empty data in Orbitals constructor";  
      return;
   }

   if (m_title.isEmpty()) m_title = toString(orbitalType);

   m_nBasis     = m_shellList.nBasis();
   m_nOrbitals  = alphaCoefficients.size() / m_nBasis;
   m_restricted = (betaCoefficients.size() != alphaCoefficients.size());

   m_alphaCoefficients.resize(m_nOrbitals, m_nBasis);

   unsigned ka(0);
   for (unsigned i = 0; i < m_nOrbitals; ++i) {
       for (unsigned j = 0; j < m_nBasis; ++j, ++ka) {
           m_alphaCoefficients(i,j) = alphaCoefficients[ka];
       }
   }

   if (m_restricted) return;

   m_betaCoefficients.resize(m_nOrbitals, m_nBasis);
   unsigned kb(0);
   for (unsigned i = 0; i < m_nOrbitals; ++i) {
       for (unsigned j = 0; j < m_nBasis; ++j, ++kb) {
           m_betaCoefficients(i,j) = betaCoefficients[kb];
       }
   }
}


bool Orbitals::areOrthonormal() const
{
   Vector const&  overlap(m_shellList.overlapMatrix());
   if (overlap.size() == 0) return true;

   Matrix S(m_nBasis, m_nBasis);
   Matrix T;

   unsigned k(0);
   for (unsigned i = 0; i < m_nBasis; ++i) {
       for (unsigned j = 0; j <=i; ++j, ++k) {
           S(i,j) = S(j,i) = overlap[k];
       }   
   }   

   T = prod(S, trans(m_alphaCoefficients));
   T = prod(m_alphaCoefficients, T);
   
   bool pass(true);
   double thresh(1e-8);

   for (unsigned i = 0; i < m_nOrbitals; ++i) {
       pass = pass && (std::abs(T(i,i)) < thresh);
       for (unsigned j = 0; j < i; ++j) {
           pass = pass && (std::abs(T(i,j)) < thresh) && (std::abs(T(j,i)) < thresh);
       } 
   }

#if 0
   QStringList matT(PrintMatrix(T,5));
   for (int i = 0; i < matT.size(); ++i) {
       qDebug() << matT[i];
   }
#endif
 
   return pass;
}


QString Orbitals::label(unsigned index, bool) const
{
   return QString::number(index+1);
}


QStringList Orbitals::labels(bool alpha) const
{
   unsigned n(m_alphaCoefficients.size1());
   if (!alpha && !m_restricted) n = m_betaCoefficients.size1();

   QStringList list;
 
   for (unsigned i = 0; i < n; ++i) {
       list.append(label(i,alpha));
   }

   return list;
}


bool Orbitals::consistent() const 
{ 
   bool consistent(m_nBasis == m_shellList.nBasis());

   consistent = consistent  &&
      m_nOrbitals > 0       && 
      m_nBasis    > 0       && 
      m_nOrbitals <= m_nBasis;

   consistent = consistent && areOrthonormal();

   if (!consistent) {
      qDebug() << "Inconsistent orbital information";
      qDebug() << "Orbitals:" << m_nOrbitals;
      qDebug() << "Basis:   " << m_nBasis;
   }

   return consistent;
}


Matrix const& Orbitals::alphaCoefficients() const 
{ 
   return m_alphaCoefficients; 
}


Matrix const& Orbitals::betaCoefficients()  const 
{ 
   return restricted() ? m_alphaCoefficients :  m_betaCoefficients;
}


void Orbitals::dump() const 
{
   qDebug() << "There are  " << m_nBasis << "basis functions and" << m_nOrbitals << "orbitals";
   qDebug() << "There ares " << m_shellList.size() << "shells";
   qDebug() << "Restricted:" << m_restricted;

   int s(0), p(0), d5(0), d6(0), f7(0), f10(0), g9(0), g15(0);

   unsigned nBasis(0);
   ShellList::const_iterator iter;
   for (iter = m_shellList.begin(); iter != m_shellList.end(); ++iter) {
       switch ((*iter)->angularMomentum()) {
          case  Data::Shell::S:    ++s;    nBasis +=  1;  break;
          case  Data::Shell::P:    ++p;    nBasis +=  3;  break;   
          case  Data::Shell::D5:   ++d5;   nBasis +=  5;  break;
          case  Data::Shell::D6:   ++d6;   nBasis +=  6;  break;   
          case  Data::Shell::F7:   ++f7;   nBasis +=  7;  break;
          case  Data::Shell::F10:  ++f10;  nBasis += 10;  break;   
          case  Data::Shell::G9:   ++g9;   nBasis +=  9;  break;
          case  Data::Shell::G15:  ++g15;  nBasis += 15;  break;   
       }   
   }   

   QString check = (consistent() ? "OK" : "NOT OK");
   qDebug() << "Consistency check:    " << check;

   check = (nBasis == m_nBasis) ? "OK" : "NOT OK";
   qDebug() << "Basis function check: " << check << nBasis << "!= " << m_nBasis;

   QString types("   S    P   D5   D6   F7  F10");
   QString tally = QString("%1 %2 %3 %4 %5 %6").arg( s,4).arg( p,4).arg( d5,4)
                                                  .arg(d6,4).arg(f7,4).arg(f10,4);
   qDebug() << "Shell types:           " << types;
   qDebug() << "                       " << tally;

   m_shellList.dump();

/*
   SurfaceList::const_iterator surface;
   for (surface = m_surfaceList.begin(); surface != m_surfaceList.end(); ++surface) {
       (*surface)->dump();
   }
*/
}

} } // end namespace IQmol::Data
