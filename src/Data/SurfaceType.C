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


SurfaceType::SurfaceType(int const kind)
{
   switch (kind) {  
      case Custom:                  m_kind = Custom;                  break;
      case AlphaOrbital:            m_kind = AlphaOrbital;            break;
      case BetaOrbital:             m_kind = BetaOrbital;             break;
      case TotalDensity:            m_kind = TotalDensity;            break;
      case SpinDensity:             m_kind = SpinDensity;             break;
      case AlphaDensity:            m_kind = AlphaDensity;            break;
      case BetaDensity:             m_kind = BetaDensity;             break;
      case DensityCombo:            m_kind = DensityCombo;            break;
      case CubeData:                m_kind = CubeData;                break;
      case VanDerWaals:             m_kind = VanDerWaals;             break;
      case Promolecule:             m_kind = Promolecule;             break;
      case SolventExcluded:         m_kind = SolventExcluded;         break;
      case SID:                     m_kind = SID;                     break;
      case ElectrostaticPotential:  m_kind = ElectrostaticPotential;  break;
      case Geminal:                 m_kind = Geminal;                 break;
      case Correlation:             m_kind = Correlation;             break;
      case CustomDensity:           m_kind = CustomDensity;           break;
      case BasisFunction:           m_kind = BasisFunction;           break;
      case DysonLeft:               m_kind = DysonLeft;               break;
      case DysonRight:              m_kind = DysonRight;              break;
      case MullikenAtomic:          m_kind = MullikenAtomic;          break;
      case MullikenDiatomic:        m_kind = MullikenDiatomic;        break;
      case GenericOrbital:          m_kind = GenericOrbital;          break;
      default:
         qDebug() << "Unknown surface type" << kind;
         m_kind = Custom;
         
   }
}



SurfaceType::Units SurfaceType::units() const
{
   Units units(Orbital);

   if (isDensity() || 
       m_kind == SID || 
       m_kind == Promolecule ||
       m_kind == SolventExcluded || 
       m_kind == ElectrostaticPotential || 
       m_kind == VanDerWaals) {
       units = Volume;
   }

   return units;
}




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
      case DysonLeft:              label = "Dyson (left)";            break;
      case DysonRight:             label = "Dyson (right)";           break;
      case MullikenAtomic:         label = "Mulliken Atomic";         break;
      case MullikenDiatomic:       label = "Mulliken Diatomic";       break;
      case GenericOrbital:         label = "Orbital";                 break;
   }

   if (isIndexed()) label += " " + QString::number(m_index+1);
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
   return m_kind == AlphaOrbital   ||  m_kind == BetaOrbital  || 
          m_kind == DysonLeft      ||  m_kind == DysonRight   ||
          m_kind == Geminal        ||  m_kind == Correlation  ||
          m_kind == BasisFunction  ||  m_kind == GenericOrbital;
}


bool SurfaceType::isOrbital() const
{
   return m_kind == AlphaOrbital || 
          m_kind == BetaOrbital  ||
          m_kind == DysonLeft    || 
          m_kind == DysonRight   ||
          m_kind == GenericOrbital;
}


bool SurfaceType::isBasis() const
{
   return (m_kind == BasisFunction);
}

bool SurfaceType::isDensity() const
{
   return (m_kind == AlphaDensity)   || (m_kind == BetaDensity)      ||
          (m_kind == TotalDensity)   || (m_kind == SpinDensity)      ||
          (m_kind == MullikenAtomic) || (m_kind == MullikenDiatomic) ||
          (m_kind == DensityCombo)   || (m_kind == Correlation)      ||
          (m_kind == CustomDensity);
}


bool SurfaceType::isRegularDensity() const
{
   return (m_kind == AlphaDensity) || (m_kind == BetaDensity) ||
          (m_kind == TotalDensity) || (m_kind == SpinDensity);
}


bool SurfaceType::isSigned() const
{
   return (m_kind == AlphaOrbital)   || (m_kind == BetaOrbital)      ||
          (m_kind == DysonLeft)      || (m_kind == DysonRight)       ||
          (m_kind == SpinDensity)    || (m_kind == DensityCombo)     ||
          (m_kind == MullikenAtomic) || (m_kind == MullikenDiatomic) ||
          (m_kind == Geminal)        || (m_kind == Correlation)      ||
          (m_kind == BasisFunction)  || (m_kind == GenericOrbital)   ||
          (m_kind == CustomDensity) ;
}


void SurfaceType::dump() const 
{
   qDebug() << toString();
}

} } // end namespace IQmol::Data
