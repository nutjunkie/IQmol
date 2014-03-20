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

#include "LayerFactory.h"
#include "Data.h"
#include "Bank.h"
#include "Mesh.h"

#include "Surface.h"
#include "GridData.h"
#include "Frequencies.h"
#include "MolecularOrbitals.h"

#include "AtomLayer.h"
#include "BondLayer.h"
#include "FileLayer.h"
#include "CubeDataLayer.h"
#include "DipoleLayer.h"
#include "GeometryLayer.h"
#include "GeometrylistLayer.h"
#include "EfpFragmentListLayer.h"
#include "EfpFragmentLayer.h"
#include "FrequenciesLayer.h"
#include "MolecularOrbitalsLayer.h"
#include "QsLog.h"
#include "openbabel/mol.h";

#include <typeinfo>   // for std::bad_cast

#include <QDebug>


namespace IQmol {
namespace Layer {

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


Layer::List Factory::toLayers(Data::Base& data)
{
   Layer::List layers;

   qDebug() << "Layer::Factory converting" << Data::Type::toString(data.typeID());
   data.dump();

   try {

      switch (data.typeID()) {
   
         case Data::Type::Bank: {
            Data::Bank& bank(dynamic_cast<Data::Bank&>(data));
            layers.append(convert(bank));
         } break;

         case Data::Type::GeometryList: {
            Data::GeometryList& 
               geometryList(dynamic_cast<Data::GeometryList&>(data));
            layers.append(convert(geometryList));
         } break;

         case Data::Type::Geometry: {
            Data::Geometry& 
               geometry(dynamic_cast<Data::Geometry&>(data));
            layers.append(convert(geometry));
         } break;

         case Data::Type::MolecularOrbitals: {
            Data::MolecularOrbitals& 
               molecularOrbitals(dynamic_cast<Data::MolecularOrbitals&>(data));
            layers.append(new MolecularOrbitals(molecularOrbitals));
         } break;

         case Data::Type::Frequencies: {
            Data::Frequencies& 
               frequencies(dynamic_cast<Data::Frequencies&>(data));
            layers.append(new Frequencies(frequencies));
         } break;

         case Data::Type::FileList: {
            Data::FileList& fileList(dynamic_cast<Data::FileList&>(data));
            layers.append(convert(fileList));
         } break;

         case Data::Type::GridData: {
            Data::GridData& grid(dynamic_cast<Data::GridData&>(data));
            layers.append(new CubeData(grid));
         } break;

         case Data::Type::EfpFragment: {
            Data::EfpFragment& efp(dynamic_cast<Data::EfpFragment&>(data));
            layers.append(new EfpFragment(efp));
         } break;

         case Data::Type::EfpFragmentList: {
            Data::EfpFragmentList& 
               efpList(dynamic_cast<Data::EfpFragmentList&>(data));
            layers.append(convert(efpList));
         } break;

         case Data::Type::Mesh: {
            Data::Mesh& mesh(dynamic_cast<Data::Mesh&>(data));
            Data::Surface surface(mesh);
            Layer::Surface* surfaceLayer(new Surface(surface));
            surfaceLayer->setCheckState(Qt::Checked);
            layers.append(surfaceLayer);
         } break;

         default:
            QLOG_WARN() << "Conversion from unimplemented data type in Layer::Factory";
            break;
      }

   } catch (const std::bad_cast& e) {
       QLOG_ERROR() << "Data cast in Layer::Factory failed"
                    << Data::Type::toString(data.typeID());
   }

   return layers;
}


List Factory::convert(Data::Bank& bank) 
{
   List list;

   if (!bank.isEmpty()){
      Data::Bank::iterator iter;
      for (iter = bank.begin(); iter != bank.end(); ++iter) {
          list.append(toLayers(**iter));
      }
   }

   return list;
}


List Factory::convert(Data::Geometry& geometry)
{
   List list;
   Atoms* atoms(new Atoms());
   Bonds* bonds(new Bonds());
   list.append(atoms);
   list.append(bonds);

   unsigned nAtoms(geometry.nAtoms());
   OpenBabel::OBMol obMol;
   obMol.BeginModify();
   AtomMap atomMap;
   
   for (unsigned i = 0; i < nAtoms; ++i) {
       unsigned Z(geometry.atomicNumber(i));
       qglviewer::Vec position(geometry.position(i));

       Atom* atom(new Atom(geometry.atomicNumber(i)));
       atom->setPosition(geometry.position(i));
       atoms->appendLayer(atom);

       OpenBabel::OBAtom* obAtom(obMol.NewAtom());
       obAtom->SetAtomicNum(Z);
       obAtom->SetVector(position.x, position.y, position.z);
       atomMap.insert(obAtom, atom);
   }

   obMol.SetTotalCharge(geometry.charge());
   obMol.SetTotalSpinMultiplicity(geometry.multiplicity());
   obMol.EndModify();
   obMol.ConnectTheDots();
   obMol.PerceiveBondOrders();

   for (OpenBabel::OBMolBondIter obBond(&obMol); obBond; ++obBond) {
       Atom* begin(atomMap.value(obBond->GetBeginAtom()));
       Atom* end(atomMap.value(obBond->GetEndAtom()));
       Bond* bond(new Bond(begin, end));
       bond->setOrder(obBond->GetBondOrder());
       bonds->appendLayer(bond);
   }

   return list;
}


List Factory::convert(Data::GeometryList& geometryList)
{
   List list;
   unsigned nGeometries(geometryList.size());

   if (nGeometries > 0) {
      list.append( convert( *(geometryList.first()) ) );  // Atom and Bond lists
   }

   if (nGeometries > 1) {
      list.append(new GeometryList(geometryList));
   }

   return list;
}


List Factory::convert(Data::EfpFragmentList& efpList) 
{
   List list;

   if (!efpList.isEmpty()){
      EfpFragments* efpFragments( new EfpFragments );
      Data::EfpFragmentList::iterator iter;
      for (iter = efpList.begin(); iter != efpList.end(); ++iter) {
          efpFragments->appendLayer(new EfpFragment(**iter));
      }
      list.append(efpFragments);
   }

   return list;
}


List Factory::convert(Data::FileList& fileList) 
{
   List list;

   if (!fileList.isEmpty()){
      Data::FileList::iterator iter;
      Files* files( new Files );
      for (iter = fileList.begin(); iter != fileList.end(); ++iter) {
          files->appendLayer(new File(**iter));
      }
      list.append(files);
   }

   return list;
}

} } // end namespace IQmol::Layer
