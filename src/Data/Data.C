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

#include "Data.h"
#include <sstream>

namespace IQmol {
namespace Data {
namespace Type {

QString toString(ID const id)
{
   QString s("Invalid Type");

   switch (id) {
      case Undefined:                s = "Data::Undefined";                 break;
      case Atom:                     s = "Data::Atom";                      break;
      case AtomList:                 s = "Data::List<Atom>";                break;
      case Bank:                     s = "Data::Bank";                      break;
      case PointCharge:              s = "Data::PointCharge";               break;
      case PointChargeList:          s = "Data::List<PointCharge>";         break;
      case EfpFragment:              s = "Data::EfpFragment";               break;
      case EfpFragmentList:          s = "Data::List<EfpFragment>";         break;
      case EfpFragmentLibrary:       s = "Data::EfpFragmentLibrary";        break;
      case CubeData:                 s = "Data::CubeData";                  break;
      case GridData:                 s = "Data::GridData";                  break;
      case GridDataList:             s = "Data::GridDataList";              break;
      case File:                     s = "Data::File";                      break;
      case FileList:                 s = "Data::List<File>";                break;
      case Geometry:                 s = "Data::Geometry";                  break;
      case GeometryList:             s = "Data::List<Geometry>";            break;
      case RemSection:               s = "Data::RemSection";                break;
      case Frequencies:              s = "Data::Frequencies";               break;
      case VibrationalMode:          s = "Data::VibrationalMode";           break;
      case VibrationalModeList:      s = "Data::VibrationalModeList";       break;
      case MolecularOrbitals:        s = "Data::MolecularOrbitals";         break;
      case MolecularOrbitalsList:    s = "Data::MolecularOrbitalsList";     break;

      case Orbitals:                 s = "Data::Orbitals";                  break;
      case OrbitalsList:             s = "Data::OrbitalsList";              break;
      case LocalizedOrbitals:        s = "Data::LocalizedOrbitals";         break;
      case CanonicalOrbitals:        s = "Data::CanonicalOrbitals";         break;

      case Density:                  s = "Data::Density";                   break;
      case DensityList:              s = "Data::DensityList";               break;

      case Shell:                    s = "Data::Shell";                     break;
      case ShellList:                s = "Data::ShellList";                 break;
      case Energy:                   s = "Data::Energy";                    break;
      case Hessian:                  s = "Data::Hessian";                   break;
      case ConformerEnergy:          s = "Data::ConformerEnergy";           break;

      case AtomicProperty:           s = "Data::AtomicProperty";            break;
      case AtomicSymbol:             s = "Data::AtomicSymbol";              break;
      case AtomicNumber:             s = "Data::AtomicNumber";              break;
      case AtomColor:                s = "Data::AtomColor";                 break;
      case NmrShielding:             s = "Data::NmrShielding";              break;
      case NmrShift:                 s = "Data::NmrShift";                  break;
      case Mass:                     s = "Data::Mass";                      break;
      case AtomicCharge:             s = "Data::AtomicCharge";              break;
      case MullikenCharge:           s = "Data::MullikenCharge";            break;
      case GasteigerCharge:          s = "Data::GasteigerCharge";           break;
      case MultipoleDerivedCharge:   s = "Data::MultipoleDerivedCharge";    break;
      case ChelpgCharge:             s = "Data::ChelpgCharge";              break;
      case SpinDensity:              s = "Data::SpinDensity";               break;
      case VdwRadius:                s = "Data::VdwRadius";                 break;

      case DipoleMoment:             s = "Data::DipoleMoment";              break;
      case TotalEnergy:              s = "Data::TotalEnergy";               break;
      case ScfEnergy:                s = "Data::ScfEnergy";                 break;
      case ForceFieldEnergy:         s = "Data::ForceFieldEnergy";          break;
      case PointGroup:               s = "Data::PointGroup";                break;

      case Constraint:               s = "Data::Constraint:";               break;
      case PositionConstraint:       s = "Data::PositionConstraint:";       break;
      case DistanceConstraint:       s = "Data::DistanceConstraint:";       break;
      case AngleConstraint:          s = "Data::AngleConstraint:";          break;
      case TorsionConstraint:        s = "Data::TorsionConstraint:";        break;

      case ChargeMultiplicity:       s = "Data::ChargeMultiplicity";        break;
      case MultipoleExpansion:       s = "Data::MultipoleExpansion";        break;
      case MultipoleExpansionList:   s = "Data::MultipoleExpansionList";    break;

      case Mesh:                     s = "Data::Mesh";                      break;
      case MeshList:                 s = "Data::MeshList";                  break;
      case Surface:                  s = "Data::Surface";                   break;
      case SurfaceType:              s = "Data::SurfaceType";               break;
      case SurfaceList:              s = "Data::SurfaceList";               break;
      case SurfaceInfo:              s = "Data::SurfaceInfo";               break;
      case SurfaceInfoList:          s = "Data::SurfaceInfoList";           break;

      case ElectronicTransition:     s = "Data::ElectronicTransition";      break;
      case ElectronicTransitionList: s = "Data::ElectronicTransitionList";  break;
      case ExcitedStates:            s = "Data::ExcitedStates";             break;
      case NmrReference:             s = "Data::NmrReference";              break;
      case NmrReferenceList:         s = "Data::NmrReferenceList";          break;
      case Nmr:                      s = "Data::Nmr";                       break;

      case OrbitalSymmetries:        s = "Data::OrbitalSymmetries";         break;
      case YamlNode:                 s = "Data::YamlNode";                  break;
      case PovRay:                   s = "Data::PovRay";                    break;
      case GeminalOrbitals:          s = "Data::GeminalOrbitals";           break;
   }

   return s;
}

} // end namespace Type


void Base::copy(Base const& that)
{
   if (typeID() == Type::Undefined) {
      // Base copy ctor, nothing to do.
      return;
   }else if (typeID() != that.typeID()) {
       std::string msg("Invalid attempt to copy ");
       msg += toString(that.typeID()).toStdString();
       msg += " to ";
       msg += toString(typeID()).toStdString();
       throw  std::runtime_error(msg);
    }

    std::stringstream ss;
    {
       OutputArchive archive(ss);
       const_cast<Base&>(that).serialize(archive);
    }
    destroy();
    {
       InputArchive archive(ss);
       serialize(archive);
    }
}


} } // end namespace IQmol::Data
