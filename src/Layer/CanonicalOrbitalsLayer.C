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

#include "CanonicalOrbitalsLayer.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {

CanonicalOrbitals::CanonicalOrbitals(Data::CanonicalOrbitals& canonicalOrbitals)
 : Orbitals(canonicalOrbitals), m_canonicalOrbitals(canonicalOrbitals)
{
   if (orbitalType() == Data::Orbitals::Canonical) {
      computeDensityVectors();
   }
   m_availableDensities.append(m_canonicalOrbitals.densityList());
   qDebug() << "Number of available densities" << m_availableDensities.size();
}


double CanonicalOrbitals::alphaOrbitalEnergy(unsigned const i) const 
{ 
   return m_canonicalOrbitals.alphaOrbitalEnergy(i);
}


double CanonicalOrbitals::betaOrbitalEnergy(unsigned const i) const 
{ 
   return m_canonicalOrbitals.betaOrbitalEnergy(i);
}


void CanonicalOrbitals::computeDensityVectors()
{
   using namespace boost::numeric::ublas;

   unsigned N(nBasis());
   unsigned Na(nAlpha());
   unsigned Nb(nBeta());

   Matrix const& alphaCoefficients(m_orbitals.alphaCoefficients());
   Matrix const& betaCoefficients(m_orbitals.betaCoefficients());

   Matrix coeffs(Na, N);
   Matrix Pa(N, N);
   Matrix Pb(N, N);

   for (unsigned i = 0; i < Na; ++i) {
       for (unsigned j = 0; j < N; ++j) {
           coeffs(i,j) = alphaCoefficients(i,j);  
       }
   }

   noalias(Pa) = prod(trans(coeffs), coeffs);

   Data::SurfaceType alpha(Data::SurfaceType::AlphaDensity);
   m_availableDensities.append(new Data::Density(alpha, Pa, "Alpha Density"));

   coeffs.resize(Nb, N);

   for (unsigned i = 0; i < Nb; ++i) {
       for (unsigned j = 0; j < N; ++j) {
           coeffs(i,j) = betaCoefficients(i,j);
       }
   }

   noalias(Pb) = prod(trans(coeffs), coeffs);

   Data::SurfaceType beta(Data::SurfaceType::BetaDensity);
   m_availableDensities.append(new Data::Density(beta, Pb, "Beta Density"));

   Data::SurfaceType total(Data::SurfaceType::TotalDensity);
   m_availableDensities.append(new Data::Density(total, Pa+Pb, "Total Density"));

   Data::SurfaceType spin(Data::SurfaceType::SpinDensity);
   m_availableDensities.append(new Data::Density(spin, Pa-Pb, "Spin Density"));
}


QString CanonicalOrbitals::description(Data::SurfaceInfo const& info, bool const tooltip)
{
   Data::SurfaceType const& type(info.type());
   QString label(type.toString());

   if (type.isOrbital()) {
      Data::SurfaceType::Kind kind(type.kind());

      unsigned nElectrons = Data::SurfaceType::AlphaOrbital ? nAlpha() : nBeta();
      unsigned index(type.index());
      double   orbitalEnergy(0.0);

      if (kind == Data::SurfaceType::AlphaOrbital) {
         orbitalEnergy = m_canonicalOrbitals.alphaOrbitalEnergy(index-1);
      }else {
         orbitalEnergy = m_canonicalOrbitals.betaOrbitalEnergy(index-1);
      }

      if (index == nElectrons-1) {
         label += " (HOMO-1)";
      }else if (index == nElectrons) {
         label += " (HOMO)";
      }else if (index == nElectrons+1) {
         label += " (LUMO)";
      }else if (index == nElectrons+2) {
         label += " (LUMO+1)";
      }

      if (tooltip) label += "\nEnergy   = " + QString::number(orbitalEnergy, 'f', 3);
   }else {
      // density
   }

   if (tooltip) label += "\nIsovalue = " + QString::number(info.isovalue(), 'f', 3);
 
   return label;
}

} } // end namespace IQmol::Layer
