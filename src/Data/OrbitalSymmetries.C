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

#include "OrbitalSymmetries.h"
#include <QDebug>


namespace IQmol {
namespace Data {


void OrbitalSymmetries::setOccupied(Spin const spin, unsigned const nOrbs)
{
   switch (spin) {
      case Alpha:  
         m_nAlpha = nOrbs;
         m_nBeta  = nOrbs;
         break;
      case Beta:   
         m_nBeta  = nOrbs;  
         break;
   }
}


void OrbitalSymmetries::append(Spin const spin, double const energy, QString const& symmetry)
{
   switch (spin) {
      case Alpha:
         m_alphaEnergies.append(energy);
         m_alphaSymmetries.append(symmetry);
         break;
      case Beta:
         m_betaEnergies.append(energy);
         m_betaSymmetries.append(symmetry);
         break;
   }
}


double OrbitalSymmetries::energy(Spin const spin, unsigned const n) const 
{ 
   double nrg(0.0);;

   switch (spin) {
      case Alpha:  
         if ((int)n < m_alphaEnergies.size()) { nrg = m_alphaEnergies.at(n); }
         break;

      case Beta:
         if ((int)n < m_betaEnergies.size()) {
            nrg = m_betaEnergies.at(n);
          }else if (m_betaEnergies.isEmpty() && (int)n < m_alphaEnergies.size()) {
             nrg = m_alphaEnergies.at(n);  
         }
         break;
   } 

   return nrg;
}


QString OrbitalSymmetries::symmetry(Spin const spin, unsigned const n) const 
{ 
   QString sym("?");

   switch (spin) {
      case Alpha:  
          if ((int)n < m_alphaSymmetries.size()) sym = m_alphaSymmetries.at(n);  
          break;

      case Beta:   
          if ((int)n < m_betaSymmetries.size()) {
             sym = m_betaSymmetries.at(n);  
          }else if (m_betaSymmetries.isEmpty() && (int)n < m_alphaSymmetries.size()) {
             sym = m_alphaSymmetries.at(n);  
          }
          break; 
   } 

   return sym;
}


void OrbitalSymmetries::dump() const 
{
   qDebug() << "(alpha, beta) = " << m_nAlpha << m_nBeta;
   for (int i = 0; i < m_alphaEnergies.size(); ++i) {
       //qDebug() << energy(Alpha, i) << symmetry(Alpha, i);
       qDebug() << energy(Alpha, i);
   }
}

} } // end namespace IQmol::Data
