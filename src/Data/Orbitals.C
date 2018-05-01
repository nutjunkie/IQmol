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

   if (alphaCoefficients.size() != m_nBasis*m_nOrbitals) {
      QLOG_WARN() << "Inconsist alpha orbital data" << toString(m_orbitalType);
      m_nOrbitals = 0;
      return;
   }

   m_alphaCoefficients.resize(m_nOrbitals, m_nBasis);

   unsigned ka(0);
   for (unsigned i = 0; i < m_nOrbitals; ++i) {
       for (unsigned j = 0; j < m_nBasis; ++j, ++ka) {
           m_alphaCoefficients(i,j) = alphaCoefficients[ka];
       }
   }

   if (m_restricted) return;

   if (betaCoefficients.size() != m_nBasis*m_nOrbitals) {
      QLOG_WARN() << "Inconsist beta orbital data" << toString(m_orbitalType);
      m_nOrbitals = 0;
      return;
   }

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

   for (unsigned i = 0; i < m_nOrbitals && pass; ++i) {
       if (std::abs(1.0-std::abs(T(i,i))) > thresh) {
          QLOG_WARN() << "Element (" << i << "," << i << ") =" << T(i,i)
                      << "deviation exceeds threshold" << thresh;
          pass = false;
       }
       for (unsigned j = 0; j < i && pass; ++j) {
           if (std::abs(T(i,j)) > thresh) {
              QLOG_WARN() << "Element (" << i << "," << j << ") =" << T(i,j) 
                          << "exceeds threshold" << thresh;
              pass = false;
           }
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

   bool orthonormal(areOrthonormal());
   // disable this check for the time being as noise can cause problems
   // consistent = consistent && orthonormal;

   if (!consistent || !orthonormal) {
      qDebug() << "Inconsistent orbital information";
      qDebug() << "Orbitals:    " << m_nOrbitals;
      qDebug() << "Basis:       " << m_nBasis;
      qDebug() << "Orthonormal: " << orthonormal;
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


/*
From the Molden page:
The following order of D, F and G functions is expected:

   5D: D 0, D+1, D-1, D+2, D-2
   6D: xx, yy, zz, xy, xz, yz

   7F: F 0, F+1, F-1, F+2, F-2, F+3, F-3
  10F: xxx, yyy, zzz, xyy, xxy, xxz, xzz, yzz, yyz, xyz

   9G: G 0, G+1, G-1, G+2, G-2, G+3, G-3, G+4, G-4
  15G: xxxx yyyy zzzz xxxy xxxz xyyy yyyz xzzz yzzz,
       xxyy xxzz yyzz xxyz xyyz xyzz
*/

void Orbitals::reorderFromQChem(Matrix& C)
{
   unsigned offset(0);
   ShellList::iterator shell;

   for (shell = m_shellList.begin(); shell != m_shellList.end(); ++shell) {
       switch ((*shell)->angularMomentum()) {

          case Shell::S:
             offset += 1;
             break;

          case Shell::P:
             offset += 3;
             break;

          case Shell::D5:
             for (unsigned i = 0; i < m_nOrbitals; ++i) {
                 double d2m( C(i, offset+0) );
                 double d1m( C(i, offset+1) );
                 double d0 ( C(i, offset+2) );
                 double d1p( C(i, offset+3) );
                 double d2p( C(i, offset+4) );
                 C(i, offset+0) = d0;
                 C(i, offset+1) = d1p;
                 C(i, offset+2) = d1m;
                 C(i, offset+3) = d2p;
                 C(i, offset+4) = d2m;
             }
             offset += 5; 
             break;

          case Shell::D6:
             for (unsigned i = 0; i < m_nOrbitals; ++i) {
                 double dxx( C(i, offset+0) );
                 double dxy( C(i, offset+1) );
                 double dyy( C(i, offset+2) );
                 double dxz( C(i, offset+3) );
                 double dyz( C(i, offset+4) );
                 double dzz( C(i, offset+5) );
                 C(i, offset+0) = dxx;
                 C(i, offset+1) = dyy;
                 C(i, offset+2) = dzz;
                 C(i, offset+3) = dxy;
                 C(i, offset+4) = dxz;
                 C(i, offset+5) = dyz;
             }
             offset += 6; 
             break;

          case Shell::F7:
             for (unsigned i = 0; i < m_nOrbitals; ++i) {
                 double f3m( C(i, offset+0) );
                 double f2m( C(i, offset+1) );
                 double f1m( C(i, offset+2) );
                 double f0 ( C(i, offset+3) );
                 double f1p( C(i, offset+4) );
                 double f2p( C(i, offset+5) );
                 double f3p( C(i, offset+6) );
                 C(i, offset+0) = f0;
                 C(i, offset+1) = f1p;
                 C(i, offset+2) = f1m;
                 C(i, offset+3) = f2p;
                 C(i, offset+4) = f2m;
                 C(i, offset+5) = f3p;
                 C(i, offset+6) = f3m;
             }
 
             offset += 7; 
             break;

          case Shell::F10:
             for (unsigned i = 0; i < m_nOrbitals; ++i) {
                 double fxxx( C(i, offset+0) );
                 double fxxy( C(i, offset+1) );
                 double fxyy( C(i, offset+2) );
                 double fyyy( C(i, offset+3) );
                 double fxxz( C(i, offset+4) );
                 double fxyz( C(i, offset+5) );
                 double fyyz( C(i, offset+6) );
                 double fxzz( C(i, offset+7) );
                 double fyzz( C(i, offset+8) );
                 double fzzz( C(i, offset+9) );
                 C(i, offset+0) = fxxx;
                 C(i, offset+1) = fyyy;
                 C(i, offset+2) = fzzz;
                 C(i, offset+3) = fxyy;
                 C(i, offset+4) = fxxy;
                 C(i, offset+5) = fxxz;
                 C(i, offset+6) = fxzz;
                 C(i, offset+7) = fyzz;
                 C(i, offset+8) = fyyz;
                 C(i, offset+9) = fxyz;
             }

             offset += 10; 
             break;

          case Shell::G9:
             for (unsigned i = 0; i < m_nOrbitals; ++i) {
                 double g4m( C(i, offset+0) );
                 double g3m( C(i, offset+1) );
                 double g2m( C(i, offset+2) );
                 double g1m( C(i, offset+3) );
                 double g0 ( C(i, offset+4) );
                 double g1p( C(i, offset+5) );
                 double g2p( C(i, offset+6) );
                 double g3p( C(i, offset+7) );
                 double g4p( C(i, offset+8) );
                 C(i, offset+0) = g0;
                 C(i, offset+1) = g1p;
                 C(i, offset+2) = g1m;
                 C(i, offset+3) = g2p;
                 C(i, offset+4) = g2m;
                 C(i, offset+5) = g3p;
                 C(i, offset+6) = g3m;
                 C(i, offset+7) = g4p;
                 C(i, offset+8) = g4m;
             }
 
             offset += 9; 
             break;

          case Shell::G15:
             for (unsigned i = 0; i < m_nOrbitals; ++i) {
                 double gxxxx( C(i, offset+ 0) );
                 double gxxxy( C(i, offset+ 1) );
                 double gxxyy( C(i, offset+ 2) );
                 double gxyyy( C(i, offset+ 3) );
                 double gyyyy( C(i, offset+ 4) );
                 double gxxxz( C(i, offset+ 5) );
                 double gxxyz( C(i, offset+ 6) );
                 double gxyyz( C(i, offset+ 7) );
                 double gyyyz( C(i, offset+ 8) );
                 double gxxzz( C(i, offset+ 9) );
                 double gxyzz( C(i, offset+10) );
                 double gyyzz( C(i, offset+11) );
                 double gxzzz( C(i, offset+12) );
                 double gyzzz( C(i, offset+13) );
                 double gzzzz( C(i, offset+14) );
                 C(i, offset+ 0) = gxxxx;
                 C(i, offset+ 1) = gyyyy;
                 C(i, offset+ 2) = gzzzz;
                 C(i, offset+ 3) = gxxxy;
                 C(i, offset+ 4) = gxxxz;
                 C(i, offset+ 5) = gxyyy;
                 C(i, offset+ 6) = gyyyz;
                 C(i, offset+ 7) = gxzzz;
                 C(i, offset+ 8) = gyzzz;
                 C(i, offset+ 9) = gxxyy;
                 C(i, offset+10) = gxxzz;
                 C(i, offset+11) = gyyzz;
                 C(i, offset+12) = gxxyz;
                 C(i, offset+13) = gxyyz;
                 C(i, offset+14) = gxyzz;
             }

             offset += 15; 
             break;
       }
   }
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
