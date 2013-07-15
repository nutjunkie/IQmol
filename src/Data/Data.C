/*******************************************************************************

  Copyright (C) 2011-2013 Andrew Gilbert

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
      case Undefined:              s = "Data::Undefined";              break;
      case Atom:                   s = "Data::Atom";                   break;
      case AtomList:               s = "Data::List<Atom>";             break;
      case Bank:                   s = "Data::Bank";                   break;
      case Charge:                 s = "Data::Charge";                 break;
      case ChargeList:             s = "Data::List<Charge>";           break;
      case EfpFragment:            s = "Data::EfpFragment";            break;
      case EfpFragmentList:        s = "Data::List<EfpFragment>";      break;
      case EfpFragmentLibrary:     s = "Data::EfpFragmentLibrary";     break;
      case Grid:                   s = "Data::Grid";                   break;
      case GridList:               s = "Data::GridList";               break;
      case File:                   s = "Data::File";                   break;
      case FileList:               s = "Data::List<File>";             break;
      case Geometry:               s = "Data::Geometry";               break;
      case GeometryList:           s = "Data::List<Geometry>";         break;
      case Molecule:               s = "Data::Molecule";               break;
      case MoleculeList:           s = "Data::List<Molecule>";         break;
      case RemSection:             s = "Data::RemSection";             break;
      case Frequencies:            s = "Data::Frequencies";            break;
      case VibrationalMode:        s = "Data::VibrationalMode";        break;
      case VibrationalModeList:    s = "Data::VibrationalModeList";    break;

      case AtomicProperty:         s = "Data::AtomicProperty";         break;
      case AtomicSymbol:           s = "Data::AtomicSymbol";           break;
      case AtomColor:              s = "Data::AtomColor";              break;
      case NmrShiftIsotropic:      s = "Data::NmrShiftIsotropic";      break;
      case NmrShiftRelative:       s = "Data::NmrShiftRelative";       break;
      case Mass:                   s = "Data::Mass";                   break;
      case MullikenCharge:         s = "Data::MullikenCharge";         break;
      case StewartCharge:          s = "Data::StewartCharge";          break;
      case SpinDensity:            s = "Data::SpinDensity";            break;
      case VdwRadius:              s = "Data::VdwRadius";              break;

      case GeometricProperty:      s = "Data::GeometricProperty";      break;
      case DipoleMoment:           s = "Data::DipoleMoment";           break;
      case TotalEnergy:            s = "Data::TotalEnergy";            break;
      case ScfEnergy:              s = "Data::ScfEnergy";              break;
      case ForceFieldEnergy:       s = "Data::ForceFieldEnergy";       break;

      case MultipoleExpansion:     s = "Data::MultipoleExpansion";     break;
      case MultipoleExpansionList: s = "Data::MultipoleExpansionList"; break;

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
