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

#include "DysonOrbitalsLayer.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {

DysonOrbitals::DysonOrbitals(Data::DysonOrbitals& dysonOrbitals)
 : Orbitals(dysonOrbitals), m_dysonOrbitals(dysonOrbitals)
{
}


double DysonOrbitals::excitationEnergy(unsigned index) const 
{ 
   return m_dysonOrbitals.excitationEnergy(index);
}


QString DysonOrbitals::description(unsigned index) const 
{ 
   return m_dysonOrbitals.label(index);
}


QString DysonOrbitals::description(Data::SurfaceInfo const& info, 
   bool const tooltip)
{
   Data::SurfaceType const& type(info.type());
   unsigned index(type.index());
   QString label(m_dysonOrbitals.label(index));

   if (tooltip) {
      double energy = excitationEnergy(index);
      label += "\nEx. Energy = " + QString::number(energy, 'f', 3);
      label += " eV\nIsovalue = " + QString::number(info.isovalue(), 'f', 3);
   }
 
   return label;
}

} } // end namespace IQmol::Layer
