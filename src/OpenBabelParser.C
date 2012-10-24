/*******************************************************************************
       
  Copyright (C) 2011 Andrew Gilbert
           
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

#include "MoleculeLayer.h"
#include "ConformerListLayer.h"
#include "FrequenciesLayer.h"
#include "CubeDataLayer.h"
#include "OpenBabelParser.h"
#include "Grid.h"
#include "QsLog.h"
#include "openbabel/mol.h"
#include "openbabel/format.h"
#include "openbabel/obconversion.h"
#include "openbabel/generic.h"
#include "openbabel/griddata.h"


using namespace OpenBabel;
using namespace qglviewer;

namespace IQmol {
namespace Parser {


DataList OpenBabel::parse(QTextStream& stream)
{
   // Not very pretty, but how else do you convert a QTextStream to
   // std::istringstream?
   QString s(stream.readAll());

   OBConversion conv;
   conv.SetInFormat("xyz");
   OBMol mol;
   std::istringstream iss(std::string(s.toLatin1()));
   conv.Read(&mol, &iss);

   m_dataList = extractData(mol);
   return m_dataList;
}


DataList OpenBabel::parseFile(QString const& fileName)
{
   OBConversion conv;
   OBFormat *inFormat = conv.FormatFromExt(fileName.toAscii().data());
   if (!inFormat || !conv.SetInFormat(inFormat)) throw ExtensionError();

   std::ifstream ifs;
   ifs.open(fileName.toAscii().data());
   if (!ifs) throw ReadError();

   ::OpenBabel::OBMol mol;
   if (!conv.Read(&mol, &ifs)) throw FormatError();

   ifs.close();
   m_dataList = extractData(mol);
   return m_dataList;
}


DataList OpenBabel::extractData(::OpenBabel::OBMol& obMol)
{
   // Atoms, partial charges
   Layer::Atoms* atoms = new Layer::Atoms();
   Layer::Atom* atom;
   Layer::AtomMap atomMap;

   FOR_ATOMS_OF_MOL(obAtom, &obMol) {
      Vec pos(obAtom->x(), obAtom->y(), obAtom->z());
      atom = new Layer::Atom(obAtom->GetAtomicNum());  // Need to set parent !!!
      atom->setCharge(obAtom->GetPartialCharge());     // display options (Molecule::createAtom())
      atom->setPosition(pos);
      atoms->appendRow(atom);
      atomMap.insert(&*obAtom, atom);
   }

   if (!atoms->hasChildren()) {
      // bail early if no atoms were found, as is the case for QChem input files.
      delete atoms;
      return DataList();
   }

   // Bonds
   Layer::Bonds* bonds = new Layer::Bonds();
   Layer::Bond* bond;
   Layer::Atom *begin, *end;

   FOR_BONDS_OF_MOL(obBond, &obMol) {
      begin = atomMap.value(obBond->GetBeginAtom());
      end   = atomMap.value(obBond->GetEndAtom());

      if (begin && end) {
         bond = new Layer::Bond(begin, end);
         bond->setOrder(obBond->GetBondOrder());
         bonds->appendRow(bond);
      }
   }

   Layer::Info* info = new Layer::Info();
   info->addAtoms(atomMap.values());
   info->setCharge(obMol.GetTotalCharge());
   info->setMultiplicity(obMol.GetTotalSpinMultiplicity());

   DataList dataList;
   dataList.append(info);
   dataList.append(atoms);
   dataList.append(bonds);

   ::OpenBabel::OBGenericData* data;

   // Frequencies
   data = obMol.GetData(::OpenBabel::OBGenericDataType::VibrationData);
   if (data) {
      QLOG_INFO() << "Vibrational data found";
      ::OpenBabel::OBVibrationData* vibrationData = 
         static_cast< ::OpenBabel::OBVibrationData* >(data);
      if (vibrationData) {
         Layer::Frequencies* frequencies = new Layer::Frequencies(*vibrationData);
         dataList.append(frequencies);
      }
   }

   // Cube data
   data = obMol.GetData(::OpenBabel::OBGenericDataType::GridData);
   if (data) {
      ::OpenBabel::OBGridData* gridData = static_cast< ::OpenBabel::OBGridData* >(data);
      if (gridData) {
         Layer::CubeData* cubeData = new Layer::CubeData(new Grid(*gridData));
         dataList.append(cubeData);
      }
   }

   return dataList;
}


} } // end namespace IQmol::Parser
