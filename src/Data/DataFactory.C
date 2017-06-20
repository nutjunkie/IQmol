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

#include "DataFactory.h"

#include "Atom.h"
#include "AtomicProperty.h"
#include "Bank.h"
#include "ChargeMultiplicity.h"
#include "Constraint.h"
#include "DipoleMoment.h"
#include "EfpFragment.h"
#include "Energy.h"
#include "File.h"
#include "Frequencies.h"
#include "GeminalOrbitals.h"
#include "Geometry.h"
#include "GeometryList.h"
#include "GridData.h"
#include "Hessian.h"
#include "Mesh.h"
#include "NmrReference.h"
#include "NmrReferenceLibrary.h"
#include "MultipoleExpansion.h"
#include "OrbitalsList.h"
#include "PointCharge.h"
#include "PovRay.h"
#include "YamlNode.h"
#include "PointGroup.h"
#include "RemSectionData.h"
#include "Surface.h"
#include "SurfaceInfo.h"
#include "SurfaceType.h"

#include <cstdlib>
#include <exception>
#include <QDebug>

namespace IQmol {
namespace Data {

Factory* Factory::s_instance = 0;


Factory& Factory::instance()
{
   if (s_instance == 0) {
      s_instance = new Factory();
      atexit(Factory::destroy);
   }
   return *s_instance;
}


void Factory::destroy()
{
   delete s_instance;
}


Base* Factory::create(Type::ID const id)
{
   Base* data(0);

   switch (id) {
      case Type::Atom:                    data = new Atom();                    break;
      case Type::AtomList:                data = new AtomList();                break;
      case Type::Bank:                    data = new Bank();                    break;
      case Type::PointCharge:             data = new PointCharge();             break;
      case Type::PointChargeList:         data = new PointChargeList();         break;
      case Type::EfpFragment:             data = new EfpFragment();             break;
      case Type::EfpFragmentList:         data = new EfpFragmentList();         break;
      case Type::GridData:                data = new GridData();                break;
      case Type::File:                    data = new File();                    break;
      case Type::FileList:                data = new FileList();                break;
      case Type::Geometry:                data = new Geometry();                break;
      case Type::GeometryList:            data = new GeometryList();            break;
      case Type::RemSection:              data = new RemSection();              break;

      case Type::VibrationalMode:         data = new VibrationalMode();         break;
      case Type::Frequencies:             data = new Frequencies();             break;
      case Type::VibrationalModeList:     data = new VibrationalModeList();     break;
      case Type::Energy:                  data = new Energy();                  break;
      case Type::Orbitals:                data = new Orbitals();                break;
      case Type::OrbitalsList:            data = new OrbitalsList();                break;

      // ---------- Atomic Properties ---------
      case Type::AtomicSymbol:            data = new AtomicSymbol();            break;
      case Type::AtomicNumber:            data = new AtomicNumber();            break;
      case Type::AtomColor:               data = new AtomColor();               break;
      case Type::NmrShielding:            data = new NmrShielding();            break;
      case Type::NmrShift:                data = new NmrShift();                break;
      case Type::Mass:                    data = new Mass();                    break;
      case Type::AtomicCharge:            data = new AtomicCharge();            break;
      case Type::MullikenCharge:          data = new MullikenCharge();          break;
      case Type::GasteigerCharge:         data = new GasteigerCharge();         break;
      case Type::MultipoleDerivedCharge:  data = new MultipoleDerivedCharge();  break;
      case Type::ChelpgCharge:            data = new ChelpgCharge();            break;
      case Type::SpinDensity:             data = new SpinDensity();             break;
      case Type::VdwRadius:               data = new VdwRadius();               break;

      // ---------- Geometry Properties ---------
      case Type::DipoleMoment:            data = new DipoleMoment();            break;
      case Type::ScfEnergy:               data = new ScfEnergy();               break;
      case Type::TotalEnergy:             data = new TotalEnergy();             break;
      case Type::ForceFieldEnergy:        data = new ForceFieldEnergy();        break;
      case Type::PointGroup:              data = new PointGroup();              break;
      case Type::Hessian:                 data = new Hessian();                 break;

      case Type::MultipoleExpansion:      data = new MultipoleExpansion();      break;
      case Type::MultipoleExpansionList:  data = new MultipoleExpansionList();  break;

      case Type::PositionConstraint:      data = new PositionConstraint();      break;
      case Type::DistanceConstraint:      data = new DistanceConstraint();      break;
      case Type::AngleConstraint:         data = new AngleConstraint();         break;
      case Type::TorsionConstraint:       data = new TorsionConstraint();       break;

      // ---------- Molecular Properties ---------
      case Type::ChargeMultiplicity:      data = new ChargeMultiplicity();      break;

      case Type::Mesh:                    data = new Mesh();                    break;
      case Type::MeshList:                data = new MeshList();                break;
      case Type::Surface:                 data = new Surface();                 break;
      case Type::SurfaceList:             data = new SurfaceList();             break;

      case Type::SurfaceInfo:             data = new SurfaceInfo();             break;
      case Type::SurfaceInfoList:         data = new SurfaceInfoList();         break;
      case Type::SurfaceType:             data = new SurfaceType();             break;
      case Type::NmrReference:            data = new NmrReference();            break;

      case Type::YamlNode:                data = new YamlNode();                break;
      case Type::PovRay:                  data = new PovRay();                  break;

      case Type::GeminalOrbitals:         data = new GeminalOrbitals();         break;
   default: 
      qDebug() << "TypeID:" << id;
      throw std::runtime_error("Unrecognized type in Data::Factory");
      break;
   }

   if (data->typeID() != id) {
      delete data;
      data = 0;
      throw std::runtime_error("Type mismatch in Data::Factory");
   }

   return data;
}

} } // end namespace IQmol::Data

BOOST_CLASS_EXPORT(IQmol::Data::Base)
