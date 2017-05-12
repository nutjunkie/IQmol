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

#include "GeminalOrbitals.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Data {

GeminalOrbitals::GeminalOrbitals(unsigned const nAlpha, unsigned const nBeta, 
   unsigned const nBasis,
   QList<double> const& alphaCoefficients, QList<double> const& betaCoefficients, 
   QList<double> const& geminalEnergies, QList<double> const& geminalCoefficients,
   QList<int> const& geminalMoMap, ShellList const& shells) 
 : m_nAlpha(nAlpha), m_nBeta(nBeta), m_nBasis(nBasis), m_geminalEnergies(geminalEnergies), 
   m_geminalCoefficients(geminalCoefficients), m_geminalMoMap(geminalMoMap),
   m_shellList(shells)
{
   m_nGeminals = m_geminalEnergies.size();
   QLOG_DEBUG() << "Number of alpha electrons  :: " << m_nAlpha;
   QLOG_DEBUG() << "Number of beta  electrons  :: " << m_nBeta;
   QLOG_DEBUG() << "Number of basis functions  :: " << m_nBasis;
   QLOG_DEBUG() << "Number of geminals         :: " << m_nGeminals;

   if (m_nBasis == 0) return;

   m_nOrbitals = alphaCoefficients.size()/m_nBasis;
   QLOG_DEBUG() << "Number of orbitals         :: " << m_nOrbitals;
   QLOG_DEBUG() << "Alpha GMO coefficient size :: " << alphaCoefficients.size();
   QLOG_DEBUG() << "Beta  GMO coefficient size :: " << betaCoefficients.size();
   QLOG_DEBUG() << "Geminal energies      size :: " << geminalEnergies.size();

   if (alphaCoefficients.size() != (int)m_nOrbitals*(int)m_nBasis ||
       betaCoefficients.size()  != (int)m_nOrbitals*(int)m_nBasis) {
       m_nOrbitals = 0;
       QLOG_WARN() << "Inconsistent geminal data" 
                   << alphaCoefficients.size() << "!=" <<(int)m_nOrbitals*(int)m_nBasis
                   << betaCoefficients.size()  << "!=" <<(int)m_nOrbitals*(int)m_nBasis;
       return;
   }

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

   computeBoundingBox();
   computeGeminalLimits();
}

void GeminalOrbitals::computeGeminalLimits()
{

   QList<int> const& geminalMoMap(m_geminalMoMap);
   unsigned Na(nAlpha());
   unsigned Nb(nBeta());
   unsigned nGemOrb(nOrbitals());
   unsigned nOS=Na-Nb;
   unsigned nGeminals(nAlpha());
   unsigned i,j,n;
   
   unsigned* GemL = new unsigned[nGeminals+1];
   //Each geminal has a subset of orbitals associated with it, with limits stored in GemL
   // For geminal j it is from GemL[j] to GemL[j+1].  We populate GemL below by looking
   // through MOGem which contains a geminal index of each orbital
   GemL[0] = 0; j = 1;
   for (i = 1; i < nGemOrb; i++) { 
    if ( geminalMoMap[i] == (int)j ) { GemL[j] = i; j++; }
   }
   GemL[nGeminals] = nGemOrb;
   // Open-shells need a separate assignement, as they are lumped toghether in MOGem
   if (nOS > 1) { for (i = nGeminals - nOS  + 1; i < nGeminals + 1; i++) { GemL[i] = GemL[i-1] + 1; } }
   //std::cout<<" Debugging GemL list for "<<nGeminals<<" geminals and "<<nGemOrb<<" orbitals \n";
   //for(i=0; i<nGeminals+1; i++){std::cout<<GemL[i]<<" ";} std::cout<<"\n";
   
   for (n=0; n<nGeminals+1; n++) { m_geminalOrbitalLimits.append(GemL[n]);}

   delete [] GemL;
  
}

bool GeminalOrbitals::consistent() const
{
   bool ok(true);
   ok = ok && m_nOrbitals > 0;
   ok = ok && m_nAlpha <= m_nOrbitals;
   ok = ok && m_nBeta  <= m_nOrbitals;

   unsigned nBasis(0);
   ShellList::const_iterator iter;
   for (iter = m_shellList.begin(); iter != m_shellList.end(); ++iter) {
       nBasis += (*iter)->nBasis();
   }

   ok = ok && m_nBasis == nBasis;

   return ok; 
}


void GeminalOrbitals::computeBoundingBox()
{     
   if (m_shellList.isEmpty()) return;
   m_shellList[0]->boundingBox(m_bbMin, m_bbMax);

   qglviewer::Vec tmin, tmax;
   ShellList::const_iterator iter;
   for (iter = m_shellList.begin(); iter != m_shellList.end(); ++iter) {
       (*iter)->boundingBox(tmin, tmax);
       m_bbMin.x = std::min(tmin.x, m_bbMin.x);
       m_bbMin.y = std::min(tmin.y, m_bbMin.y);
       m_bbMin.z = std::min(tmin.z, m_bbMin.z);
       m_bbMax.x = std::max(tmax.x, m_bbMax.x);
       m_bbMax.y = std::max(tmax.y, m_bbMax.y);
       m_bbMax.z = std::max(tmax.z, m_bbMax.z);
   }
}


void GeminalOrbitals::dump() const
{
   qDebug() << "There are  " << m_nAlpha << "alpha and" << m_nBeta << "beta electrons";
   qDebug() << "There are  " << m_nBasis << "basis functions and" << m_nOrbitals << "orbitals";
   qDebug() << "There ares " << m_shellList.size() << "shells";

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

   QString check("OK");
   if (nBasis != m_nBasis) check.prepend("NOT ");
   qDebug() << "Basis function check:     " << check;

   QString types("   S    P   D5   D6   F7  F10");
   QString tally = QString("%1 %2 %3 %4 %5 %6").arg( s,4).arg( p,4).arg( d5,4)
                                                  .arg(d6,4).arg(f7,4).arg(f10,4);
   qDebug() << "Shell types:              " << types;
   qDebug() << "                          " << tally;

   //m_shellList.dump();

   SurfaceList::const_iterator surface;
   for (surface = m_surfaceList.begin(); surface != m_surfaceList.end(); ++surface) {
       (*surface)->dump();
   }
}

} } // end namespace IQmol::Data
