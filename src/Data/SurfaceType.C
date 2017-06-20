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

#include "SurfaceType.h"
#include <QDebug>


namespace IQmol {
namespace Data {

QString SurfaceType::toString() const 
{
   QString label;
   switch (m_kind) {
      case Custom:                 label = m_label;                   break;
      case AlphaOrbital:           label = "Alpha";                   break;
      case BetaOrbital:            label = "Beta";                    break;
      case TotalDensity:           label = "Total Density";           break;
      case SpinDensity:            label = "Spin Density";            break;
      case AlphaDensity:           label = "Alpha Density";           break;
      case BetaDensity:            label = "Beta Density";            break;
      case DensityCombo:           label = "User Defined Density";    break;
      case CubeData:               label = "Cube Data";               break;
      case VanDerWaals:            label = "van der Waals";           break;
      case Promolecule:            label = "Promolecule";             break;
      case SolventExcluded:        label = "Solvent Excluded";        break;
      case SID:                    label = "SID";                     break;
      case ElectrostaticPotential: label = "Electrostatic Potential"; break;
      case Geminal:                label = "Geminal";                 break;
      case Correlation:            label = "Correlated Density";      break;
      case CustomDensity:          label = "Custom Density";          break;
      case BasisFunction:          label = "Basis Function";          break;
   }

   if (isIndexed()) label += " " + QString::number(m_index);
   return label;
}


bool SurfaceType::operator==(SurfaceType const& that) const
{
   bool same(m_kind == that.m_kind && m_index == that.m_index);
   if (m_kind == Custom) same = same && m_label == that.m_label;

   return same;
}


bool SurfaceType::isIndexed() const
{
   return (m_kind == AlphaOrbital   || m_kind == BetaOrbital  || 
           m_kind == Geminal        || m_kind == Correlation) ||
           m_kind == BasisFunction;
}


bool SurfaceType::isOrbital() const
{
   return (m_kind == AlphaOrbital) || (m_kind == BetaOrbital);
}


bool SurfaceType::isBasis() const
{
   return (m_kind == BasisFunction);
}

bool SurfaceType::isDensity() const
{
   return (m_kind == AlphaDensity) || (m_kind == BetaDensity) ||
          (m_kind == TotalDensity) || (m_kind == SpinDensity) ||
          (m_kind == DensityCombo) || (m_kind == Correlation);
}


bool SurfaceType::isRegularDensity() const
{
   return (m_kind == AlphaDensity) || (m_kind == BetaDensity) ||
          (m_kind == TotalDensity) || (m_kind == SpinDensity);
}




bool SurfaceType::isSigned() const
{
   return (m_kind == AlphaOrbital) || (m_kind == BetaOrbital)  ||
          (m_kind == SpinDensity)  || (m_kind == DensityCombo) ||
          (m_kind == Geminal)      || (m_kind == Correlation)  ||
          (m_kind == BasisFunction);
}


void SurfaceType::dump() const 
{
   qDebug() << toString();
}

} } // end namespace IQmol::Data
