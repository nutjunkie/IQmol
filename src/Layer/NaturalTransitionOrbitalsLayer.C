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

#include "NaturalTransitionOrbitalsLayer.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {

NaturalTransitionOrbitals::NaturalTransitionOrbitals(
   Data::NaturalTransitionOrbitals& ntos)
 : Orbitals(ntos), m_naturalTransitionOrbitals(ntos)
{
}


double NaturalTransitionOrbitals::alphaOrbitalAmplitude(unsigned index) const 
{ 
   return m_naturalTransitionOrbitals.alphaOccupancy(index);
}


double NaturalTransitionOrbitals::betaOrbitalAmplitude(unsigned index) const 
{ 
   return m_naturalTransitionOrbitals.betaOccupancy(index);
}



QString NaturalTransitionOrbitals::description(Data::SurfaceInfo const& info, 
   bool const tooltip)
{
   Data::SurfaceType const& type(info.type());

   unsigned index(type.index());
   QString label(m_naturalTransitionOrbitals.label(index));

   if (tooltip) {
      Data::SurfaceType::Kind kind(type.kind());
      double occupancy(0.0);
      if (kind == Data::SurfaceType::AlphaOrbital) {
         occupancy = m_naturalTransitionOrbitals.alphaOccupancy(index-1);
      }else {
         occupancy = m_naturalTransitionOrbitals.betaOccupancy(index-1);
      }

      label += "\nOccupancy = " + QString::number(occupancy, 'f', 3);
      label += "\nIsovalue = " + QString::number(info.isovalue(), 'f', 3);
   }
 
   return label;
}

} } // end namespace IQmol::Layer
