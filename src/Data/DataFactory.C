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

#include "DataFactory.h"
#include "Atom.h"
#include "AtomicProperty.h"
#include "Bank.h"
#include "Charge.h"
#include "EfpFragment.h"
#include "File.h"
#include "Geometry.h"
#include "GridData.h"
#include "Molecule.h"
#include "GeometricProperty.h"
#include "RemSectionData.h"
#include "MultipoleExpansion.h"
#include "Frequencies.h"

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
      case Type::Charge:                  data = new Charge();                  break;
      case Type::ChargeList:              data = new ChargeList();              break;
      case Type::EfpFragment:             data = new EfpFragment();             break;
      case Type::EfpFragmentList:         data = new EfpFragmentList();         break;
      case Type::Grid:                    data = new Grid();                    break;
      case Type::File:                    data = new File();                    break;
      case Type::FileList:                data = new FileList();                break;
      case Type::Geometry:                data = new Geometry();                break;
      case Type::GeometryList:            data = new GeometryList();            break;
      case Type::Molecule:                data = new Molecule();                break;
      case Type::MoleculeList:            data = new MoleculeList();            break;
      case Type::RemSection:              data = new RemSection();              break;

      case Type::VibrationalMode:         data = new VibrationalMode();         break;
      case Type::Frequencies:             data = new Frequencies();             break;
      case Type::VibrationalModeList:     data = new VibrationalModeList();     break;

      // ---------- Atomic Properties ---------
      case Type::AtomicSymbol:            data = new AtomicSymbol();            break;
      case Type::AtomColor:               data = new AtomColor();               break;
      case Type::NmrShiftIsotropic:       data = new NmrShiftIsotropic();       break;
      case Type::NmrShiftRelative:        data = new NmrShiftRelative();        break;
      case Type::Mass:                    data = new Mass();                    break;
      case Type::MullikenCharge:          data = new MullikenCharge();          break;
      case Type::StewartCharge:           data = new StewartCharge();           break;
      case Type::SpinDensity:             data = new SpinDensity();             break;
      case Type::VdwRadius:               data = new VdwRadius();               break;

      // ---------- Geometry Properties ---------
      case Type::DipoleMoment:            data = new DipoleMoment();            break;
      case Type::ScfEnergy:               data = new ScfEnergy();               break;
      case Type::TotalEnergy:             data = new TotalEnergy();             break;

      case Type::MultipoleExpansion:      data = new MultipoleExpansion();      break;
      case Type::MultipoleExpansionList:  data = new MultipoleExpansionList();  break;


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
