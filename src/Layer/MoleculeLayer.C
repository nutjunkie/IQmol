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

// Data
#include "AtomicProperty.h"
#include "Bank.h"
#include "DipoleMoment.h"
#include "Energy.h"
#include "File.h"
#include "Frequencies.h"
#include "Geometry.h"
#include "GeometryList.h"
#include "MultipoleExpansion.h"
#include "PointGroup.h"
#include "SurfaceInfo.h"


// Layers
#include "LayerFactory.h"
#include "AtomLayer.h"
#include "BondLayer.h"
#include "ChargeLayer.h"
#include "ConstraintLayer.h"
#include "CubeDataLayer.h"
#include "DipoleLayer.h"
#include "FrequenciesLayer.h"
#include "EfpFragmentLayer.h"
#include "GroupLayer.h"
#include "MoleculeLayer.h"
#include "MolecularOrbitalsLayer.h"
#include "OrbitalsLayer.h"
#include "SurfaceLayer.h"


#include "UndoCommands.h"
#include "SpatialProperty.h"
#include "Constants.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include "QChemJobInfo.h" 
#include "ProgressDialog.h"
#include "Preferences.h"
//#include "GridEvaluator.h"
#include "IQmolParser.h"

#include "openbabel/mol.h"
#include "openbabel/format.h"
#include "openbabel/obconversion.h"
#include "openbabel/generic.h"
#include "openbabel/forcefield.h"
#include "openbabel/plugin.h"

#include <QFileDialog>
#include <QDropEvent>
#include <QProcess>
#include <QTime>
#include <QMenu>
#include <QUrl>
#include <vector>
#include <QtDebug>


extern "C" void symmol_(int*, double*, double*, int*, char*);

using namespace OpenBabel;
using namespace qglviewer;

namespace IQmol {
namespace Layer {

bool Molecule::s_autoDetectSymmetry = false;

Molecule::Molecule(QObject* parent) : Base(DefaultMoleculeName, parent), 
   m_drawMode(Primitive::BallsAndSticks), 
   m_atomScale(1.0), m_bondScale(1.0), m_chargeScale(1.0), 
   m_smallerHydrogens(true), 
   m_modified(false),
   m_reperceiveBondsForAnimation(false),
   m_configurator(*this), 
   m_surfaceAnimator(this), 
   m_info(this), 
   m_atomList(this, "Atoms"), 
   m_bondList(this, "Bonds"), 
   m_chargesList(this, "Charges"), 
   m_fileList(this, "Files"), 
   m_surfaceList(this, "Surfaces"), 
   m_constraintList(this, "Constraints"), 
   m_scanList(this, "Scan Coordinates"), 
   m_groupList(this, "Groups"), 
   m_efpFragmentList(this),
   m_molecularSurfaces(*this),
   m_currentGeometry(0), 
   m_chargeType(Data::Type::GasteigerCharge)
{
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | 
      Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

   m_chargesList.setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
   m_chargesList.setCheckState(Qt::Checked);

   setCheckState(Qt::Checked);
   setConfigurator(&m_configurator);

   // Okay, this is weird.  The OpenBabel plugins don't seem to get
   // initialized unless an OBConversion is used first.  This is a
   // problem if I try to use a force field before converting anything.
   // The following gets around this.
   OBConversion conv;
   conv.SetInFormat("xyz");

   // Add actions for the context menu
   connect(newAction("Configure"), SIGNAL(triggered()), 
      this, SLOT(configure()));
   connect(newAction("Select All"), SIGNAL(triggered()),
      this, SLOT(selectAll()));
   connect(newAction("Reperceive Bonds"), SIGNAL(triggered()), 
      this, SLOT(reperceiveBonds()));

   m_atomicChargesMenu = newAction("Atomic Charges");

   connect(newAction("Remove"), SIGNAL(triggered()), 
      this, SLOT(removeMolecule()));
//#warning "################################";
//#warning "# !!! TURN OFF FOR RELEASE !!! #";
//#warning "################################";
//   connect(newAction("Dump Data"), SIGNAL(triggered()), 
//      this, SLOT(dumpData()));

   initProperties();

   connect(&m_efpFragmentList, SIGNAL(updated()), this, SIGNAL(softUpdate()));
   connect(&m_groupList, SIGNAL(updated()), this, SIGNAL(softUpdate()));
   connect(&m_surfaceList, SIGNAL(updated()), this, SIGNAL(softUpdate()));
   connect(&m_molecularSurfaces, SIGNAL(updated()), this, SIGNAL(softUpdate()));
}


Molecule::~Molecule()
{
   deleteProperties();
}


void Molecule::setFile(QString const& fileName)
{
   m_inputFile.setFile(fileName);
   setText(m_inputFile.completeBaseName());
}


void Molecule::appendData(IQmol::Data::Bank& bank)
{
   Factory& factory(Factory::instance());
   Layer::List layerList(factory.toLayers(bank));
   appendData(layerList);
   m_bank.merge(bank);

   QList<Data::GeometryList*> list(m_bank.findData<Data::GeometryList>());
   if (!list.isEmpty()) {
      unsigned index(list.first()->defaultIndex());
      setGeometry(*(list.first()->at(index)));
   }
}


void Molecule::appendData(Layer::List& list)
{
   // !!! This needs fixing !!!
   // This is a bit cheesy, we rely on the QStandardItem text 
   // to determine the type of Layer.  
   QList<Base*> currentLayers(findLayers<Base>(Children));
   QStringList labels;

   labels << "Info";
   QList<Base*>::iterator base;
   for (base = currentLayers.begin(); base != currentLayers.end(); ++base) {
       labels << (*base)->text();
   }

   Layer::List::iterator iter;
   Files*        files(0);
   Atoms*        atoms(0);
   Bonds*        bonds(0);
   Charges*      charges(0);
   CubeData*     cubeData(0);
   Orbitals*     orbitals(0);
   EfpFragments* efpFragments(0);
   MolecularOrbitals* molecularOrbitals(0);

   QString text;
   PrimitiveList primitiveList;
   Layer::List toSet;

   for (iter = list.begin(); iter != list.end(); ++iter) {
       text = (*iter)->text();
qDebug() << "Layer text" << text;
       // Hack to allow multiple cube data files and frequencies
       if (text == "Cube Data") text = "cubedata";
       if (text == "Frequencies") text = "frequencies";

       if ((files = qobject_cast<Files*>(*iter))) {
          FileList fileList(files->findLayers<File>(Children));
          FileList::iterator file;
          for (file = fileList.begin(); file != fileList.end(); ++file) {
              files->removeLayer(*file);
              m_fileList.appendLayer(*file);
          }

       }else if ((molecularOrbitals = qobject_cast<MolecularOrbitals*>(*iter))) {
          m_molecularSurfaces.appendLayer(molecularOrbitals);

       }else if ((orbitals = qobject_cast<Orbitals*>(*iter))) {
          m_molecularSurfaces.appendLayer(orbitals);
          //toSet.append(*iter);
          orbitals->setMolecule(this);

// This is currently preventing the addition of primivees to an exisiting molecule
       }else if (!labels.contains(text)) {
          labels << (*iter)->text();

          if ((atoms = qobject_cast<Atoms*>(*iter))) {
             AtomList atomList(atoms->getAtoms());
qDebug() << "Atoms size" << atomList.size();
             AtomList::iterator atom;
             for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
                 atoms->removeLayer(*atom);
                 primitiveList.append(*atom);
             }
          }else if ((bonds = qobject_cast<Bonds*>(*iter))) {
             BondList bondList(bonds->findLayers<Bond>(Children));
qDebug() << "Bonds size" << bondList.size();
             BondList::iterator bond;
             for (bond = bondList.begin(); bond != bondList.end(); ++bond) {
                 bonds->removeLayer(*bond);
                 primitiveList.append(*bond);
             }
          }else if ((charges = qobject_cast<Charges*>(*iter))) {
             ChargeList chargeList(charges->findLayers<Charge>(Children));
             ChargeList::iterator charge;
             for (charge = chargeList.begin(); charge != chargeList.end(); ++charge) {
                 charges->removeLayer(*charge);
                 primitiveList.append(*charge);
             }
          }else if ((efpFragments = qobject_cast<EfpFragments*>(*iter))) {
             QList<EfpFragment*> efps(efpFragments->findLayers<EfpFragment>(Children));
             QList<EfpFragment*>::iterator efp;
             for (efp = efps.begin(); efp != efps.end(); ++efp) {
                 efpFragments->removeLayer(*efp);
                 primitiveList.append(*efp);
             }

           }else {
			 // The ordering of this is all wrong as the atoms have not been
			 // appended yet and some Layers need this in their setMolecule
			 // function.
             toSet.append(*iter);
            }
       } 

       if ( (cubeData = qobject_cast<CubeData*>(*iter)) ) {
          m_properties << cubeData->createProperty(); 
       }
   }

qDebug() << "Calling appendPrimitives()" << primitiveList.size();
   appendPrimitives(primitiveList);
   BondList bondList(findLayers<Bond>(Children));
   if (bondList.isEmpty()) reperceiveBonds();

   Surface* surface(0);

   for (iter = toSet.begin(); iter != toSet.end(); ++iter) {
       (*iter)->setMolecule(this);
       if ((surface = qobject_cast<Surface*>(*iter))) {
          m_surfaceList.appendLayer(surface);
       }else {
          appendLayer(*iter);
       }
   }

   reindexAtomsAndBonds();
   autoDetectSymmetry();
}


bool Molecule::save(bool prompt)
{
   if (!m_modified && !prompt) return true;

   QFileInfo tmp(m_inputFile);
   if (tmp.fileName().isEmpty()) {
      QFileInfo lastFile(Preferences::LastFileAccessed());
      tmp.setFile(lastFile.dir(), text());
      prompt = true;
   } 
   
   if (prompt) {
      QString filter(tr("XYZ") + " (*.xyz)");
      QStringList extensions;
      extensions << filter
                 << tr("CML") + " (*.cml)"
                 << tr("Z-matrix Input") + " (*.gzmat)"
                 << tr("MDL SDfile") + "(*.mol)"
                 << tr("PDB") + " (*.pdb)"
                 << tr("Sybyl Mol2") + " (*.mol2)"
                 << tr("IQmol Archive") + " (*.iqmol)";

      QString fileName(QFileDialog::getSaveFileName(0, tr("Save File"), 
         tmp.filePath(), extensions.join(";;"), &filter));

      if (fileName.isEmpty()) {
         // This will occur if the user cancels the action.
         return false;
      }else {
         QRegExp rx("\\*(\\..+)\\)");
         if (rx.indexIn(filter) > 0) {
            filter = rx.cap(1);
            if (!fileName.endsWith(filter, Qt::CaseInsensitive)) {
               fileName += filter;
            }
         }
         setFile(fileName);
      }
   }

   bool success(true);

   try {

      qDebug() << "Attempting to save" << m_inputFile.filePath();
      if (m_inputFile.suffix().endsWith("iqmol", Qt::CaseInsensitive)) {
         Parser::IQmol iqmol;

         if (!m_currentGeometry) {
            m_currentGeometry = new Data::Geometry();
            m_bank.prepend(m_currentGeometry);
            AtomList atoms(findLayers<Atom>(Children));
            for (int i = 0; i < atoms.size(); ++i) {
                unsigned Z(atoms[i]->getAtomicNumber());
                m_currentGeometry->append(Z, atoms[i]->getPosition());
            }
         }

         iqmol.save(m_inputFile.filePath(), m_bank);
      }else {
         writeToFile(m_inputFile.filePath());
      }

   } catch (QString const& ioerr) {
      success = false;
      QMsgBox::warning(0, "IQmol", ioerr);
   }

   return success;
}


void Molecule::writeToFile(QString const& filePath)
{     
   QFileInfo info(filePath);
   OBConversion conv;
   OBFormat *outFormat = conv.FormatFromExt(QFile::encodeName(info.fileName()).data());
   if ( (!outFormat) || (!conv.SetOutFormat(outFormat))) {
      throw QString("Parser::ExtensionError()");;
   }

   Preferences::LastFileAccessed(filePath);

   QString tmpName(filePath + ".iqmoltmp");
   std::ofstream ofs;
   ofs.open(QFile::encodeName(tmpName).data());
   if (!ofs) { 
      throw QString("Parser::WriteError()"); 
   }

   AtomMap atomMap;
   BondMap bondMap;
   if (!conv.Write(toOBMol(&atomMap, &bondMap), &ofs)) {
      throw QString("Parser::FormatError()");
   }
   ofs.close();

   QFile target(filePath);
   if (target.exists()) target.remove();

   QFile tmpFile(tmpName);
   tmpFile.rename(filePath);

   Preferences::AddRecentFile(filePath);
   m_modified = false;
}


PrimitiveList Molecule::fromOBMol(OBMol* obMol, AtomMap* atomMap, BondMap* bondMap, 
   GroupMap* groupMap)
{
   bool deleteAtomMap(false);
   bool deleteBondMap(false);
   PrimitiveList addedPrimitives;

   if (!atomMap) {
      atomMap = new AtomMap();
      deleteAtomMap = true; 
   }

   if (!bondMap) {
      bondMap = new BondMap();
      deleteBondMap = true; 
   }

   QList<OBAtom*> groupAtoms;
   QList<Group*> groups;
   
   if (groupMap) {
      groupAtoms = groupMap->keys(); 
      groups = groupMap->values(); 
   }

   Atom* atom;
   Group* currentGroup(0);
   QList<Vec> coordinates;
   
   FOR_ATOMS_OF_MOL(obAtom, obMol) {
      Vec pos(obAtom->x(), obAtom->y(), obAtom->z());
      atom = atomMap->value(&*obAtom); 
//qDebug() << "Valency ended up  " << obAtom->GetImplicitValence();
      if (!atom) {
         atom = createAtom(obAtom->GetAtomicNum(), pos);
         addedPrimitives.append(atom);
         atomMap->insert(&*obAtom, atom);
      }

      if (groupAtoms.contains(&*obAtom)) {
         if (currentGroup == groupMap->value(&*obAtom)) {
            coordinates.append(pos);
         }else if (currentGroup == 0) {
            currentGroup = groupMap->value(&*obAtom);
            coordinates.append(pos);
         }else {
            currentGroup->align(coordinates);
            currentGroup = groupMap->value(&*obAtom);
            coordinates.clear();
            coordinates.append(pos);
         }
      }else {
         atom->setPosition(pos);
       }
   }

   // Tidy up last Group
   if (currentGroup) currentGroup->align(coordinates);

   Bond* bond;
   Atom *begin, *end;
   FOR_BONDS_OF_MOL(obBond, obMol) {
      bond  = bondMap->value(&*obBond);
      begin = atomMap->value(obBond->GetBeginAtom());
      end   = atomMap->value(obBond->GetEndAtom());

      if (!begin || !end) {
         QString msg("Error encountered converting from OBMol object");
         QMsgBox::critical(0, "IQmol", msg);
         return PrimitiveList();
      }

      if (!bond) {
         bond  = createBond(begin, end, obBond->GetBondOrder());
         addedPrimitives.append(bond);
      }
      
   }

   // We don't copy the charge and multiplicity values as Open Babel doesn't
   // seem to respect the values passed in.

   if (deleteAtomMap) delete atomMap;
   if (deleteBondMap) delete bondMap;
   return addedPrimitives;
}


OBMol* Molecule::toOBMol(AtomMap* atomMap, BondMap* bondMap, GroupMap* groupMap)
{
   OBAtom* obAtom;
   OBBond* obBond;
   Vec     position;

   OBMol* obMol(new OBMol());
   atomMap->clear();
   bondMap->clear();
   AtomList atoms(findLayers<Atom>(Children));
   AtomList::iterator atomIter;

   obMol->BeginModify();
   obMol->SetImplicitValencePerceived();
   obMol->SetHybridizationPerceived();

   for (atomIter = atoms.begin(); atomIter != atoms.end(); ++atomIter) {
       obAtom = obMol->NewAtom();
       atomMap->insert(obAtom, *atomIter);
       position = (*atomIter)->getPosition();
       obAtom->SetAtomicNum((*atomIter)->getAtomicNumber());
       obAtom->SetVector(position.x, position.y, position.z);
   }

   BondList bonds(findLayers<Bond>(Children));
   BondList::iterator bondIter;

   for (bondIter = bonds.begin(); bondIter != bonds.end(); ++bondIter) {
       obBond = obMol->NewBond();
       bondMap->insert(obBond, *bondIter);

       obBond->SetBondOrder((*bondIter)->getOrder());
       obAtom = atomMap->key((*bondIter)->beginAtom());
       obBond->SetBegin(obAtom);
       obAtom->AddBond(obBond);
       obAtom = atomMap->key((*bondIter)->endAtom());
       obBond->SetEnd(obAtom);
       obAtom->AddBond(obBond);
   }

   if (groupMap) {
      groupMap->clear();
      QList<Group*> efps(findLayers<Group>(Children));

      QList<Group*>::iterator efp;
      for (efp = efps.begin(); efp != efps.end(); ++efp) {

          AtomList::iterator atom;
          AtomList atoms((*efp)->getAtoms());
          for (atom = atoms.begin(); atom != atoms.end(); ++atom) {
              obAtom = obMol->NewAtom();
              atomMap->insert(obAtom, *atom);
              obAtom->SetAtomicNum((*atom)->getAtomicNumber());
              position = (*atom)->getPosition();
              obAtom->SetVector(position.x, position.y, position.z);
              groupMap->insert(obAtom,*efp);
          }

          BondList::iterator bond;
          BondList bonds((*efp)->getBonds());
          for (bond = bonds.begin(); bond != bonds.end(); ++bond) {

              obBond = obMol->NewBond();
              bondMap->insert(obBond, *bond);

              obBond->SetBondOrder((*bond)->getOrder());

              obAtom = atomMap->key((*bond)->beginAtom());
              obBond->SetBegin(obAtom);
              obAtom->AddBond(obBond);

              obAtom = atomMap->key((*bond)->endAtom());
              obBond->SetEnd(obAtom);
              obAtom->AddBond(obBond);
          }
      }
      
   }

   obMol->SetTotalCharge(totalCharge());
   obMol->SetTotalSpinMultiplicity(multiplicity());
   obMol->EndModify();

   return obMol;
}


Vec Molecule::getBuildAxis(Atom* atom)
{
   BondList bonds(findLayers<Bond>(Children | Visible));
   BondList::iterator bondIter;
   Atom* beginAtom;
   Atom* endAtom;

   Vec axis;
   int bondCount(0);
   for (bondIter = bonds.begin(); bondIter != bonds.end(); ++bondIter) {
       beginAtom = (*bondIter)->beginAtom();
       endAtom   = (*bondIter)->endAtom();
       if (beginAtom == atom) {
          ++bondCount;
          axis += endAtom->getPosition() - atom->getPosition();
       }else if (endAtom == atom) {
          ++bondCount;
          axis += beginAtom->getPosition() - atom->getPosition();
       }
   }

   if (bondCount == 0) {
      axis.setValue(0.0, 0.0, 1.0);
   }else if (axis.normalize() < 0.001) {
      QLOG_DEBUG() << "Could not determine build axis";
   }

   return axis;
}


// The following three functions should probably be migrated to the
// Data::Geometry class

QStringList Molecule::coordinatesForCubeFile() 
{
   AtomList atomList(findLayers<Atom>(Children | Visible));
   Vec position;
   QString line;
   QStringList coords;

   AtomList::iterator iter;
   // This additional loop may not be required.
   for (int i = 1; i <=  atomList.size(); ++i) {
       for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
           if ((*iter)->getIndex() == i) {
              position = (*iter)->getPosition()*Constants::AngstromToBohr;
              int Z = (*iter)->getAtomicNumber();
              line  = QString("%1").arg(Z, 5);
              line += QString("%1").arg((double)Z,  14, 'f', 6);
              line += QString("%1").arg(position.x, 14, 'f', 6);
              line += QString("%1").arg(position.y, 14, 'f', 6);
              line += QString("%1").arg(position.z, 14, 'f', 6);
              coords.append(line);
           }
       }
   }
   return coords;
}


QString Molecule::coordinatesAsString(bool const selectedOnly)
{
   AtomList atomList(findLayers<Atom>(Children | Visible));
   Vec position;
   QString coords;

   if (atomList.isEmpty()) {
      QList<EfpFragment*>::iterator iter;
      QList<EfpFragment*> efps(findLayers<EfpFragment>(Children | Visible));
      for (iter = efps.begin(); iter != efps.end(); ++iter) {
          coords += "--\n0 1\n";
          coords += (*iter)->format(EfpFragment::Frame) + "\n";
      }

   }else {

      AtomList::iterator iter;
      for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
          if (!selectedOnly || (*iter)->isSelected()) {
             position = (*iter)->getPosition();
             coords += QString("%1").arg((*iter)->getAtomicSymbol(), 3);
             coords += QString("%1").arg(position.x, 13, 'f', 7);
             coords += QString("%1").arg(position.y, 13, 'f', 7);
             coords += QString("%1").arg(position.z, 13, 'f', 7) + "\n";
          }
      }
   }

   coords.chop(1);
   return coords;
}


QList<Vec> Molecule::coordinates()
{
   QList<Vec> coordinates;
   AtomList atoms(findLayers<Atom>(Children));
   for (int i = 0; i < atoms.size(); ++i) {
       coordinates << atoms[i]->getPosition();
   }
   return coordinates;
}


QString Molecule::efpFragmentsAsString()
{
   QString efpFragmentsSection;

   EfpFragment::Mode mode;
   AtomList atoms(findLayers<Atom>(Children | Visible));
   if (atoms.isEmpty()) {
      // EFP-only job
      mode = EfpFragment::Name;
   }else {
      // EFP-QM only job
      mode = EfpFragment::NameAndFrame;
   }

   QList<EfpFragment*>::iterator iter;
   QList<EfpFragment*> efps(findLayers<EfpFragment>(Children | Visible));
   for (iter = efps.begin(); iter != efps.end(); ++iter) {
       efpFragmentsSection += (*iter)->format(mode) + "\n";
   }

   return efpFragmentsSection;
}


QString Molecule::efpParametersAsString()
{
   QSet<QString> names;
   QList<EfpFragment*>::iterator iter;
   QList<EfpFragment*> efps(findLayers<EfpFragment>(Children | Visible));
   for (iter = efps.begin(); iter != efps.end(); ++iter) {
       names.insert((*iter)->text());
   }

   return EfpFragment::efpParamsSection(names);
}


QString Molecule::externalChargesAsString()
{
   ChargeList charges(m_chargesList.findLayers<Charge>(Visible|Children));
   if (charges.isEmpty()) return QString();

   QString s;

   ChargeList::iterator iter;
   for (iter = charges.begin(); iter != charges.end(); ++iter) {
       s += (*iter)->toString() + "\n"; 
   }

   return s;
}


QString Molecule::constraintsAsString() 
{
   ConstraintList constraints(findLayers<Constraint>(Visible|Children));
   if (constraints.isEmpty()) return QString();

   ConstraintList fixed;
   ConstraintList internal;

   ConstraintList::iterator iter;
   for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
       if ((*iter)->optimizeConstraint()) {
          if ((*iter)->type() == Constraint::Position) {
             fixed.append(*iter);
          }else {
             internal.append(*iter);
          }
       }
   }

   if (internal.isEmpty() && fixed.isEmpty()) return QString();

   QString s("$opt\n");

   if (!fixed.isEmpty()) {
      s += "FIXED\n";
      for (iter = fixed.begin(); iter != fixed.end(); ++iter) {
          s += (*iter)->formatQChem();
      }
      s += "ENDFIXED\n";
   }

   if (!internal.isEmpty()) {
      s += "CONSTRAINT\n";
      for (iter = internal.begin(); iter != internal.end(); ++iter) {
          s += (*iter)->formatQChem();
      }
      s += "ENDCONSTRAINT\n";
   }

   s += "$end";
   return s;
}


QString Molecule::scanCoordinatesAsString() 
{
   ConstraintList constraints(findLayers<Constraint>(Visible | Children));

   QString s;
   ConstraintList::iterator iter;
   for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
       if ((*iter)->scanConstraint()) {
          s += (*iter)->formatQChem();
       }
   }

   return s;
}


Constraint* Molecule::findMatchingConstraint(AtomList const& atoms)
{
   ConstraintList::iterator iter;
   ConstraintList constraints(findLayers<Constraint>(Children | Visible));

   for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
       if ((*iter)->sameAtoms(atoms)) return (*iter);
   }
   return 0;
}


bool Molecule::canAcceptConstraint(Constraint* constraint)
{
   int scanCount(m_scanList.findLayers<Constraint>().size());
   return constraint->scanConstraint() ? (scanCount < 2) : true;
}


void Molecule::addConstraintLayer(Constraint* constraint)
{
   if (!constraint) return;

   if (constraint->scanConstraint()) {
      m_scanList.appendLayer(constraint);
   }else {
      m_constraintList.appendLayer(constraint);
   }

   connect(constraint, SIGNAL(updated()), this, SLOT(constraintUpdated()));
   connect(constraint, SIGNAL(invalid()), this, SLOT(removeConstraint()));
   updated();
}


void Molecule::removeConstraintLayer(Constraint* constraint)
{
   if (!constraint) return;

   if (constraint->optimizeConstraint()) {
      m_constraintList.removeLayer(constraint);
   }else if (constraint->scanConstraint()) {
      m_scanList.removeLayer(constraint);
   }

   disconnect(constraint, SIGNAL(updated()), this, SLOT(constraintUpdated()));
   disconnect(constraint, SIGNAL(invalid()), this, SLOT(removeConstraint()));
   updated();
}


void Molecule::removeConstraint()
{
   removeConstraintLayer(qobject_cast<Constraint*>(sender()));
}


void Molecule::addConstraint(Constraint* constraint)
{
   if (constraint->scanConstraint() || constraint->optimizeConstraint()) {
      Command::AddConstraint* cmd(new Command::AddConstraint(this, constraint));
      postCommand(cmd);
   }
   applyConstraint(constraint);
}


void Molecule::constraintUpdated()
{
   Constraint* constraint(qobject_cast<Constraint*>(sender()));
   applyConstraint(constraint);
}


void Molecule::applyConstraint(Constraint* constraint)
{
   Command::MoveObjects* cmd(new Command::MoveObjects(this, "", true));

   QString msg = constraint->optimizeConstraint() ? "Apply" : "Set";

   switch (constraint->constraintType()) {
      case Constraint::Invalid:
         delete cmd;
         return;
         break;
      case Constraint::Position:
         QLOG_TRACE() << "Applying position constraint";
         msg += " position";
         applyPositionConstraint(constraint);
         break;
      case Constraint::Distance:
         QLOG_TRACE() << "Applying distance constraint";
         msg += " bond";
         applyDistanceConstraint(constraint);
         break;
      case Constraint::Angle:
         QLOG_TRACE() << "Applying angle constraint";
         msg += " angle";
         applyAngleConstraint(constraint);
         break;
      case Constraint::Torsion:
         QLOG_TRACE() << "Applying torsion constraint";
         msg += " torsion";
         applyTorsionConstraint(constraint);
         break;
   }

   if (constraint->optimizeConstraint()) msg += " constraint";
   cmd->setText(msg);

   postCommand(cmd);
   postMessage(constraint->message());
}


void Molecule::applyPositionConstraint(Constraint* constraint)
{
   AtomList allAtoms(findLayers<Atom>(Children));
   Vec displacement(constraint->targetDisplacement());
   Vec position;

   AtomList::iterator iter;
   for (iter = allAtoms.begin(); iter != allAtoms.end(); ++iter) {
	   position = (*iter)->getPosition();
       (*iter)->setPosition(position+displacement);
   }
}


void Molecule::applyDistanceConstraint(Constraint* constraint)
{
   AtomList allAtoms(findLayers<Atom>(Children | Visible));
   AtomList atoms(constraint->atomList());
   Atom *A(atoms[0]), *B(atoms[1]);

   AtomList fragA;
   fragA.append(A);
   fragA += getContiguousFragment(B, A);

   AtomList fragB;
   fragB.append(B);
   fragB += getContiguousFragment(A, B);

   // First check for a ring bond constraint.  
   AtomList::iterator iter;
   for (iter = fragA.begin(); iter != fragA.end(); ++iter) {
       if (fragB.contains(*iter)) return applyRingConstraint();
   }

   Vec a(A->getPosition());
   Vec b(B->getPosition());
   Vec ab(a-b);

   double value(constraint->targetValue());
   Vec shift(0.5*(ab.norm() - value)*ab.unit());
   //Vec delta(value*(a-b).unit());
   Vec c;

   for (iter = allAtoms.begin(); iter != allAtoms.end(); ++iter) {
       c = (*iter)->getPosition(); 
       c += fragB.contains(*iter) ? shift : -shift;
       (*iter)->setPosition(c);
   }
}


void Molecule::applyAngleConstraint(Constraint* constraint)
{
   AtomList atoms(constraint->atomList());
   Atom *A(atoms[0]), *B(atoms[1]), *C(atoms[2]);

   AtomList fragA;
   fragA.append(A);
   fragA += getContiguousFragment(B, A);

   AtomList fragC;
   fragC.append(C);
   fragC += getContiguousFragment(B, C);

   // First check for a ring angle constraint.  
   AtomList::iterator iter;
   for (iter = fragA.begin(); iter != fragA.end(); ++iter) {
       if (fragC.contains(*iter)) return applyRingConstraint();
   }

   double delta( 0.5*(Atom::angle(A,B,C) - constraint->targetValue()) );
   delta = delta * M_PI / 180.0;
   Vec b(B->getPosition());
   Vec ab(b - A->getPosition());
   Vec cb(b - C->getPosition());
   Quaternion quaternion(cross(ab, cb), delta);

   Vec x;
   for (iter = fragA.begin(); iter != fragA.end(); ++iter) {
       x = (*iter)->getPosition(); 
       x = quaternion.rotate(x-b) + b;
       (*iter)->setPosition(x);
   }

   quaternion.setAxisAngle(cross(ab, cb), -delta);
   for (iter = fragC.begin(); iter != fragC.end(); ++iter) {
       x = (*iter)->getPosition(); 
       x = quaternion.rotate(x-b) + b;
       (*iter)->setPosition(x);
   }
}


void Molecule::applyTorsionConstraint(Constraint* constraint)
{
   AtomList atoms(constraint->atomList());
   Atom *A(atoms[0]), *B(atoms[1]), *C(atoms[2]), *D(atoms[3]);

   AtomList fragA;
   fragA += getContiguousFragment(B, C);

   AtomList fragD;
   fragD += getContiguousFragment(C, B);

   // First check for a ring angle constraint.  
   AtomList::iterator iter;
   for (iter = fragA.begin(); iter != fragA.end(); ++iter) {
       if (fragD.contains(*iter)) return applyRingConstraint();
   }

   double delta( 0.5*(Atom::torsion(A,B,C,D) - constraint->targetValue()) );
   delta = delta * M_PI / 180.0;

   Vec b(B->getPosition());
   Vec c(C->getPosition());
   Vec m(0.5*(b+c));
   Quaternion quaternion(b-c, delta);

   Vec x;
   for (iter = fragA.begin(); iter != fragA.end(); ++iter) {
       x = (*iter)->getPosition(); 
       x = quaternion.rotate(x-m) + m;
       (*iter)->setPosition(x);
   }

   quaternion.setAxisAngle(b-c, -delta);
   for (iter = fragD.begin(); iter != fragD.end(); ++iter) {
       x = (*iter)->getPosition(); 
       x = quaternion.rotate(x-m) + m;
       (*iter)->setPosition(x);
   }
}


void Molecule::applyRingConstraint()
{
   QLOG_DEBUG() << "Ring constraint found";
   QString msg("Imposing constraints within rings requires optimization of the structure.\n\n");
   msg += "Mimimize structure?";

   QPixmap pixmap;
   pixmap.load(":/imageQuestion");
   QMessageBox messageBox(QMessageBox::NoIcon, "IQmol", msg);
   QPushButton* nowButton(messageBox.addButton("Now", QMessageBox::AcceptRole));
   messageBox.addButton("Later", QMessageBox::RejectRole);
   messageBox.setIconPixmap(pixmap);
   messageBox.exec();

   // Note that this simply uses the default force field, which may not be the
   // current one.  This avoids polling the ViewerModel for the latest forcefield.
   if (messageBox.clickedButton() == nowButton) {
      minimizeEnergy(Preferences::DefaultForceField());
   }
}





// This is not active at the moment.  Data layers should be deleted
void Molecule::clearData()
{
   return;
   takeRow(m_fileList.row());
}


void Molecule::takePrimitive(Primitive* primitive)
{
   PrimitiveList primitiveList;
   primitiveList.append(primitive);
   takePrimitives(primitiveList);
}


void Molecule::appendPrimitive(Primitive* primitive)
{
   PrimitiveList primitiveList;
   primitiveList.append(primitive);
   appendPrimitives(primitiveList);
}


void Molecule::appendPrimitives(PrimitiveList const& primitives)
{
   Atom* atom;
   Bond* bond;
   Charge* charge;
   EfpFragment* efp;
   Group* group;

   AtomList atoms(findLayers<Atom>(Children));
   int initialNumberOfAtoms(atoms.size());

   PrimitiveList::const_iterator primitive;
   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {

       if ( (atom = qobject_cast<Atom*>(*primitive)) ) {
          m_atomList.appendLayer(atom);

       }else if ( (bond = qobject_cast<Bond*>(*primitive)) ) {
          m_bondList.appendLayer(bond);

       }else if ( (charge = qobject_cast<Charge*>(*primitive)) ) {
          m_chargesList.appendLayer(charge);

       }else if ( (efp = qobject_cast<EfpFragment*>(*primitive)) ) {
          m_efpFragmentList.appendLayer(efp);

       }else if ( (group = qobject_cast<Group*>(*primitive)) ) {
          m_groupList.appendLayer(group);
          //appendPrimitives(group->ungroup());

       } else {
          QMsgBox::warning(0, "IQmol", "Attempt to add unknown primitive type to molecule");
       }
   }

   reindexAtomsAndBonds();
   atoms = findLayers<Atom>(Children);

   if (atoms.size() > initialNumberOfAtoms) {
      if (initialNumberOfAtoms == 0) { 
         insertRow(0,&m_info);
         appendRow(&m_molecularSurfaces);
      }
      updateInfo();
      radius();
   }
   m_modified = true;
   updated();
}


void Molecule::takePrimitives(PrimitiveList const& primitives)
{
   Atom* atom;
   Bond* bond;
   Charge* charge;
   EfpFragment* efp;
   Group* group;

   AtomList atoms(findLayers<Atom>(Children | Visible));
   int initialNumberOfAtoms(atoms.size());
//   QLOG_TRACE() << "Taking" << primitives.size() 
//                << "primitives from molecule with" << initialNumberOfAtoms << "atoms";

   PrimitiveList::const_iterator primitive;
   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       // Make sure out primitive is no longer selected
       (*primitive)->deselect();

       if ( (atom = qobject_cast<Atom*>(*primitive)) ) {
          m_atomList.removeLayer(*primitive);

       }else if ( (bond = qobject_cast<Bond*>(*primitive)) ) {
          m_bondList.removeLayer(*primitive);

       }else if ( (charge = qobject_cast<Charge*>(*primitive)) ) {
          m_chargesList.takeRow((*primitive)->row());

       }else if ( (efp = qobject_cast<EfpFragment*>(*primitive)) ) {
          m_efpFragmentList.takeRow((*primitive)->row());

       }else if ( (group = qobject_cast<Group*>(*primitive)) ) {
          m_groupList.takeRow((*primitive)->row());

       } else {
          QMsgBox::warning(0, "IQmol", "Atempt to remove unknown primitive type to molecule");
       }
   }

   reindexAtomsAndBonds();
   atoms = findLayers<Atom>(Children);

   if (atoms.isEmpty()) {
      takeRow(m_info.row());
      takeRow(m_molecularSurfaces.row());
   }else if (atoms.size() < initialNumberOfAtoms) {
      updateInfo();
   }
   m_modified = true;
   updated();
}


void Molecule::updateInfo()
{
   m_info.clear();
   AtomList atoms(findLayers<Atom>(Children));
   if (atoms.isEmpty()) return;

   setAtomicCharges(Data::Type::GasteigerCharge);
   bool estimated(true);
   dipoleAvailable(dipoleFromPointCharges(), estimated); 

   m_info.addAtoms(atoms);
   autoDetectSymmetry();
}


void Molecule::selectAll()
{
   PrimitiveList::iterator iter;
   PrimitiveList primitiveList(findLayers<Primitive>(Visible | Children));
   for (iter = primitiveList.begin(); iter != primitiveList.end(); ++iter) {
       select((*iter)->QStandardItem::index(), QItemSelectionModel::Select);
   }
}


// Note the indices are coming in 0-indexed 
void Molecule::selectAtoms(QList<int> const& indices)
{
   AtomList::iterator iter;
   AtomList atomList(findLayers<Atom>(Visible | Children));
   for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
       if (indices.contains((*iter)->getIndex()-1)) {
          select((*iter)->QStandardItem::index(), QItemSelectionModel::Select);
       }else {
          select((*iter)->QStandardItem::index(), QItemSelectionModel::Deselect);
       }
   }
}



void Molecule::deleteSelection()
{
   // Must also delete any bond attached to any atom we are deleting.
   BondList::iterator bond;
   BondList allBonds(findLayers<Bond>(Visible | Children));

   for (bond = allBonds.begin(); bond != allBonds.end(); ++bond) {
       if ( (*bond)->beginAtom()->isSelected() || (*bond)->endAtom()->isSelected() ) {
          (*bond)->select();
       }
   }

   PrimitiveList deleteTargets(findLayers<Primitive>(Visible | Children | SelectedOnly));

   if (!deleteTargets.isEmpty()) {
      QLOG_DEBUG() << "Deleting" << deleteTargets.size() << "selected objects";
      PrimitiveList::iterator iter;
      Command::EditPrimitives* cmd(new Command::EditPrimitives("Remove atoms/bonds", this));
      cmd->remove(deleteTargets);
      postCommand(cmd);
   }
   m_modified = true;
}


PrimitiveList Molecule::getSelected(bool danglingBonds)
{
   PrimitiveList selected;

   AtomList allAtoms(findLayers<Atom>(Visible | Children));
   AtomList::iterator atom;
   for (atom = allAtoms.begin(); atom != allAtoms.end(); ++atom) {
       if ((*atom)->isSelected()) selected.append(*atom);
   } 

   BondList allBonds(findLayers<Bond>(Visible | Children));
   BondList::iterator bond;
   for (bond = allBonds.begin(); bond != allBonds.end(); ++bond) {
       if ((*bond)->isSelected()) {
          if (danglingBonds) {
             selected.append(*bond);
          }else if (selected.contains((*bond)->beginAtom()) && 
                    selected.contains((*bond)->endAtom())) {
             selected.append(*bond);
          }
       }
   } 

   ChargeList allCharges(findLayers<Charge>(Visible | Children));
   ChargeList::iterator charge;
   for (charge = allCharges.begin(); charge != allCharges.end(); ++charge) {
       if ((*charge)->isSelected()) selected.append(*charge);
   } 

   return selected;
}


void Molecule::groupSelection()
{
/*
   qDebug() << "Molecule::groupSelection called()";
   PrimitiveList primitives(findLayers<Primitive>(Visible|Children));
   PrimitiveList selected;
   PrimitiveList::iterator iter;

   qDebug() << "   groupSelection found" << primitives.size() << "primitives";
   for (iter = primitives.begin(); iter != primitives.end(); ++iter) {
       if ((*iter)->isSelected()) selected.append(*iter);
   } 

   qDebug() << "   selection count" << selected.size();
   if (selected.isEmpty()) return;

   // Check to see we haven't already grouped any of the objects
   for (iter = selected.begin(); iter != selected.end(); ++iter) {
       if ((*iter)->isInGroup()) {
          QMsgBox::warning(0, "IQmol", "Unable to nest fragments");
          return;
       }
   }

   takePrimitives(selected);
   appendPrimitive(new Layer::Fragment(selected));

   //postCommand(new Command::GroupPrimitives(this, selected));
   m_modified = true;
*/
}


void Molecule::ungroupSelection() 
{
/*
   qDebug() << "Molecule::ungroupSelection called()";
   FragmentList fragments(findLayers<Fragment>(Visible|Children));
   FragmentList selected;
   FragmentList parents;

   FragmentList::iterator frag, parent;

   qDebug() << "   Number of fragments found:" << fragments.size();
   for (frag = fragments.begin(); frag != fragments.end(); ++frag) {
       if ((*frag)->isSelected()) selected.append(*frag);
   } 

   
   qDebug() << "   Number of selected fragments :" << selected.size();

   fragments.clear();

   // We only want the top-most selected fragments, so if any of the parent
   // fragments are also selected we skip.
   for (frag = selected.begin(); frag != selected.end(); ++frag) {
       parents = (*frag)->findLayers<Fragment>(Parents);
       bool append(true);
       for (parent = parents.begin(); parent != parents.end(); ++parent) {
           if (selected.contains(*parent)) append = false;
       }

       if (append) fragments.append(*frag);
   } 

   //takePrimitives(fragments);
   qDebug() << "   Number of fragments to remove:" << fragments.size();
 
   PrimitiveList primitiveList;
   for (frag = fragments.begin(); frag != fragments.end(); ++frag) {
      
       Base* base( QVariantPointer<Base>::toPointer((*frag)->QStandardItem::parent()->data()) );
       Fragment* fragment;
       if ((dynamic_cast<Molecule*>(base) == this)) {
          qDebug() << "   removing " << (*frag)->text();
          takePrimitive(*frag);
          appendPrimitives((*frag)->ungroup());
       } else if ( (fragment = dynamic_cast<Fragment*>(base)) ) {
          qDebug() << "   removing " << (*frag)->text();
          takePrimitive(*frag);
          fragment->group((*frag)->ungroup());
       }else {
          qDebug() << "Molecule/Fragment parent not found in ungroupSelection()";
       }

       //delete *frag;
   } 

   //postCommand(new Command::UngroupFragments(this, fragments));
   m_modified = true;
*/
}


void Molecule::updateAtomOrder()
{
   AtomList atoms(findLayers<Atom>(Children | Visible));
   AtomList::iterator iter;
   QStandardItem* parent;

   for (int i = atoms.size(); i > 0; --i) {
       for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
           if ((*iter)->getReorderIndex() == i) {
              parent = (*iter)->QStandardItem::parent();
              parent->takeRow((*iter)->row());
              parent->insertRow(0, *iter);
              break;
         }
      }
   }
   reindexAtomsAndBonds();
}


void Molecule::setGeometry(IQmol::Data::Geometry& geometry)
{
   m_currentGeometry = &geometry;
   AtomList atoms(findLayers<Atom>(Children));
   unsigned nAtoms(atoms.size());
   if (nAtoms != geometry.nAtoms()) {
      QLOG_DEBUG() << "Invalid Geometry passed to Molecule::setGeometry";
      return;
   }

   for (unsigned i = 0; i < nAtoms; ++i) {
       atoms[i]->setTranslation(geometry.position(i));
       atoms[i]->setSpinDensity(geometry.getAtomicProperty<Data::SpinDensity>(i).value());
       if (geometry.hasProperty<Data::NmrShift>()) {
          atoms[i]->setNmrShift(geometry.getAtomicProperty<Data::NmrShift>(i).value());
       }else if (geometry.hasProperty<Data::NmrShielding>()) {
          atoms[i]->setNmrShielding(geometry.getAtomicProperty<Data::NmrShielding>(i).value());
       }else {
          atoms[i]->setNmrShielding(0.0);
       }
   }

   setAtomicCharges(m_chargeType);
   centerOfNuclearChargeAvailable(centerOfNuclearCharge());

   initProperties();

   if (geometry.hasProperty<Data::TotalEnergy>()) {
      Data::TotalEnergy const& energy(geometry.getProperty<Data::TotalEnergy>());
      // This needs cleaning up
      Data::Energy::Units units(energy.units());
      switch (units) {
         case Data::Energy::Hartree: 
            energyAvailable(energy.value(), Info::Hartree); 
            break;
         case Data::Energy::KJMol: 
            energyAvailable(energy.value(), Info::KJMol); 
            break;
         case Data::Energy::KCalMol: 
            energyAvailable(energy.value(), Info::KCalMol); 
            break;
         default:
            break;
      }
   }

   if (geometry.hasProperty<Data::PointGroup>()) {
      pointGroupAvailable(geometry.getProperty<Data::PointGroup>().value());
   }

   if (geometry.hasProperty<Data::DipoleMoment>()) {
      Data::DipoleMoment const& dipole(geometry.getProperty<Data::DipoleMoment>());
      bool estimated(false);
      dipoleAvailable(dipole.value(), estimated); 
   }else {
      bool estimated(true);
      dipoleAvailable(dipoleFromPointCharges(), estimated);
   }

   multiplicityAvailable(geometry.multiplicity());
   chargeAvailable(geometry.charge());
}


void Molecule::saveToCurrentGeometry()
{
   if (!m_currentGeometry) return;

   AtomList atomList(findLayers<Atom>(Children));
   unsigned nAtoms(atomList.size());
   if (nAtoms != m_currentGeometry->nAtoms()) {
      QLOG_DEBUG() << "Invalid Geometry passed to Molecule::saveToCurrentGeometry";
      return;
   }
   qDebug() << "Saving current geometry";

   QList<qglviewer::Vec> coordinates;
   AtomList::iterator iter;
   for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
       coordinates.append( (*iter)->getPosition() );
   }

   m_currentGeometry->setCoordinates(coordinates);
}



void Molecule::reindexAtomsAndBonds()
{
   m_maxAtomicNumber = 0;
   AtomList atoms(findLayers<Atom>(Children));
   for (int i = 0; i < atoms.size(); ++i) {
       atoms[i]->setIndex(i+1);
       m_maxAtomicNumber = std::max((int)m_maxAtomicNumber, atoms[i]->getAtomicNumber());
   }

   BondList bonds(findLayers<Bond>(Children));
   for (int i = 0; i < bonds.size(); ++i) {
       bonds[i]->setIndex(i+1);
   }
}


double Molecule::radius()
{
   double radius(0);
   PrimitiveList primitives(findLayers<Primitive>(Children | Visible));
   PrimitiveList::iterator iter;
   for (iter = primitives.begin(); iter != primitives.end(); ++iter) {
       radius = std::max(radius, (double)(*iter)->getPosition().norm());
   }
   radiusAvailable(radius);
   return radius;
}


Atom* Molecule::createAtom(unsigned int const Z, qglviewer::Vec const& position)
{
   Atom* atom(new Atom(Z));
   atom->setDrawMode(m_drawMode);
   atom->setScale(m_atomScale);
   atom->setSmallerHydrogens(m_smallerHydrogens);
   atom->setPosition(position);
   return atom;
}


Bond* Molecule::createBond(Atom* begin, Atom* end, int const order)
{
   Bond* bond(new Bond(begin, end));
   bond->setOrder(order);
   bond->setDrawMode(m_drawMode);
   bond->setScale(m_bondScale);
   return bond;
}


Charge* Molecule::createCharge(double const Q, Vec const& position)
{
   Charge* charge(new Charge(Q, position));
   charge->setDrawMode(m_drawMode);
   charge->setScale(m_bondScale);
   return charge;
}


Process::QChemJobInfo Molecule::qchemJobInfo()
{
   Process::QChemJobInfo jobInfo;

   jobInfo.set(Process::QChemJobInfo::Charge,          totalCharge());
   jobInfo.set(Process::QChemJobInfo::Multiplicity,    multiplicity());
   jobInfo.set(Process::QChemJobInfo::Coordinates,     coordinatesAsString());
   jobInfo.set(Process::QChemJobInfo::Constraints,     constraintsAsString());
   jobInfo.set(Process::QChemJobInfo::ScanCoordinates, scanCoordinatesAsString());
   jobInfo.set(Process::QChemJobInfo::EfpFragments,    efpFragmentsAsString());
   jobInfo.set(Process::QChemJobInfo::EfpParameters,   efpParametersAsString());
   jobInfo.set(Process::QChemJobInfo::ExternalCharges, externalChargesAsString());

   AtomList atomList(findLayers<Atom>(Children | Visible));
   if (atomList.isEmpty()) jobInfo.setEfpOnlyJob(true);

   QString name;

   if (m_inputFile.filePath().isEmpty()) {
      QFileInfo fileInfo(Preferences::LastFileAccessed());
      jobInfo.set(Process::QChemJobInfo::LocalWorkingDirectory, fileInfo.path());
      jobInfo.setBaseName(text());
   }else {
      jobInfo.set(Process::QChemJobInfo::LocalWorkingDirectory, m_inputFile.path());
      jobInfo.setBaseName(m_inputFile.completeBaseName());
      name =  + "/" + m_inputFile.completeBaseName() + ".inp";
   }

   jobInfo.setMoleculePointer(this);
   return jobInfo;
}


void Molecule::qchemJobInfoChanged(Process::QChemJobInfo const& qchemJobInfo)
{ 
   if (text() == DefaultMoleculeName) {
      setText(qchemJobInfo.baseName());
      m_info.setCharge(qchemJobInfo.getCharge());
      m_info.setMultiplicity(qchemJobInfo.getMultiplicity());
   }
}


void Molecule::addHydrogens()
{
   QLOG_DEBUG() << "Adding hydrogens";
   AtomMap atomMap;
   BondMap bondMap;

   // Not my proudest moment.  If addHydrogens is called with a single atom at
   // the origin, the positions of the added hydrogens are also set to the origin
   // so all the atoms are on top of each other.  We get around this by translating
   // the molecule by a random unit vector and then back again afterwards.
   vector3 displacement;
   displacement.randomUnitVector();
   OBMol* obMol(toOBMol(&atomMap, &bondMap));
   obMol->Translate(displacement);
   obMol->BeginModify();
   obMol->EndModify();

   // We now type the atoms and overwrite the valency/hybridization info
   // if it has been changed by the user.
   OBAtomTyper atomTyper;
   atomTyper.AssignImplicitValence(*obMol);
   atomTyper.AssignHyb(*obMol);

   AtomMap::iterator iter;
   for (iter = atomMap.begin(); iter != atomMap.end(); ++iter) {
       int hybrid(iter.value()->getHybridization());
       if (hybrid > 0) {
          iter.key()->SetImplicitValence(iter.value()->getValency());
          iter.key()->SetHyb(hybrid);
       }
   }

   obMol->AddHydrogens(false,false);
   obMol->Translate(-displacement);
   obMol->BeginModify();
   obMol->EndModify();

   Command::AddHydrogens* cmd  = 
      new Command::AddHydrogens(this, fromOBMol(obMol, &atomMap, &bondMap));
   postCommand(cmd);

   delete obMol;
   m_modified = true;
   postMessage("");
}


void Molecule::minimizeEnergy(QString const& forceFieldName)
{
   QLOG_DEBUG() << "Minimizing energy with forcefield" << forceFieldName;
   OBPlugin::List("forcefields");
   QByteArray ff(forceFieldName.toLatin1());
   const char* obff(ff.data());

   OBForceField* forceField = OBForceField::FindForceField(obff);
   if (!forceField)  {
      QString msg("Failed to load force field: ");
      msg += forceFieldName + "\nUnable to optimize structure\n";
      msg += "BABEL_DATADIR environment variable may not be set correctly.";
      QMsgBox::warning(0, "IQmol", msg);
      return;
   }

   forceField->SetLogFile(&std::cout);
   forceField->SetLogLevel(OBFF_LOGLVL_LOW);

   AtomMap atomMap;
   BondMap bondMap;
   GroupMap groupMap;
   OBMol* obMol(toOBMol(&atomMap, &bondMap, &groupMap));
   if (atomMap.size() == 0) return;

   // constraints
   OBFFConstraints obffconstraints;
   ConstraintList  constraints(findLayers<Constraint>(Visible | Children));

   if (!constraints.isEmpty()) {
      QLOG_DEBUG() << "Enforcing" << constraints.size() << "active constraints";
      ConstraintList::iterator iter;
      for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
          if ((*iter)->optimizeConstraint()) (*iter)->addTo(obffconstraints);
      }
   }

   if (!forceField->Setup(*obMol, obffconstraints)) {
      QString msg("Failed to setup force field for molecule.  ");
      msg += text() + "\n";
      msg += "Try using a different force field\n";
      msg += "\nUnable to optimize structure.";
      QMsgBox::warning(0, "IQmol", msg);
      return;
   }

   forceField->SetConformers(*obMol);

   Command::MinimizeStructure* cmd(new Command::MinimizeStructure(this));

   double convergence(1e-6f);
   int maxSteps(1000);

   // We pre-optimize with conjugate gradient 
   forceField->ConjugateGradientsInitialize(maxSteps, convergence);
   forceField->ConjugateGradientsTakeNSteps(maxSteps);

   // And finish off with steepest descent
   forceField->SteepestDescentInitialize(maxSteps, convergence);
   forceField->SteepestDescentTakeNSteps(maxSteps);
  
   if (!forceField->GetCoordinates(*obMol)) {
      QString msg("Failed to get updated coordinates");
      QMsgBox::warning(0, "IQmol", msg);
      delete obMol;
      delete cmd;
      return;
   }

   fromOBMol(obMol, &atomMap, &bondMap, &groupMap);
   delete obMol;

   QString unit(QString::fromStdString(forceField->GetUnit()));
   double energy(forceField->Energy());
   QString mesg(forceFieldName + " energy: ");
   cmd->setMessage(mesg + QString::number(energy, 'f', 4));
   if (unit.contains("kJ/mol")) { 
      energyAvailable(energy, Info::KJMol);
   }else {
      energyAvailable(energy, Info::KCalMol);
   }

   m_modified = true;
   reindexAtomsAndBonds();
   postCommand(cmd);

   // The following need to be posted after the command otherwise the 
   // molecule gets updated to the final geometry before the animators
   // have done their work
   centerOfNuclearChargeAvailable(centerOfNuclearCharge());
   setAtomicCharges(Data::Type::GasteigerCharge);
   bool estimated(true);
   dipoleAvailable(dipoleFromPointCharges(), estimated);
}


void Molecule::openSurfaceAnimator()
{
   m_surfaceAnimator.update();
   m_surfaceAnimator.show();
}


void Molecule::autoDetectSymmetry()
{ 
   if (s_autoDetectSymmetry) detectSymmetry();
}


void Molecule::invalidateSymmetry()
{
   pointGroupAvailable("?");
   postMessage("");
}


void Molecule::detectSymmetry()
{
   symmetrize(0.00001, false); 
}


// This is a nasty function used to obtain naked pointers for the symmol routine
void Molecule::symmetrize(double tolerance, bool updateCoordinates)
{
   QLOG_TRACE() << "Determining symmetry";
   QTime time;
   time.start();

   AtomList atomList(findLayers<Atom>(Children | Visible));
   Command::SymmetrizeStructure* cmd = new Command::SymmetrizeStructure(this);
   QString pointGroup;
   int nAtoms(atomList.size());

   if (nAtoms == 0) {
      pointGroup = "";

   }else if (nAtoms == 1) {
      pointGroup = "Kh";
      if (updateCoordinates) {
         atomList.first()->setPosition(Vec(0.0, 0.0, 0.0));
      }

   }else if (nAtoms == 2) {
      Atom *A(atomList[0]), *B(atomList[1]);
      pointGroup = (A->getAtomicNumber() == B->getAtomicNumber()) ? "Dih" : "Civ";
      if (updateCoordinates) {
         double bondLength(Atom::distance(A, B));
         A->setPosition( Vec(0.0, 0.0,  bondLength/2.0) );
         B->setPosition( Vec(0.0, 0.0, -bondLength/2.0) );
      }

   }else if (nAtoms > 1000) {
      pointGroup = "C1";

   }else {
      double* coordinates   = new double[3*nAtoms];
      int*    atomicNumbers = new int[nAtoms];
      char*   pg = new char[4];

      pg[3] = '\0';
      Vec position;
      int cnt(0);

      AtomList::iterator iter;
      for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
          position = (*iter)->getPosition();
          atomicNumbers[cnt] = (*iter)->getAtomicNumber();
          coordinates[3*cnt  ] = position.x;
          coordinates[3*cnt+1] = position.y;
          coordinates[3*cnt+2] = position.z;
          ++cnt;
      }

      symmol_(&nAtoms, &tolerance, coordinates, atomicNumbers, pg);
      pointGroup = pg;

      if (updateCoordinates) {
         cnt = 0;
         for (iter = atomList.begin(); iter != atomList.end(); ++iter, ++cnt) {
             position.x = coordinates[3*cnt  ];
             position.y = coordinates[3*cnt+1];
             position.z = coordinates[3*cnt+2];
             (*iter)->setPosition(position);
         }
      }

      delete[] pg;
      delete[] coordinates;
      delete[] atomicNumbers;
   }

   pointGroupAvailable(pointGroup);

   if (updateCoordinates) {
      QChar ch(0x221E); // infinity
      if (pointGroup == "Civ") {
         pointGroup = "C" + QString(ch) + "v";
      }else if (pointGroup == "Dih") {
         pointGroup = "D" + QString(ch) + "h";
      }
      cmd->setMessage("Point group: " + pointGroup);
      postCommand(cmd);
      m_modified = true;
      softUpdate();
      saveToCurrentGeometry();

      centerOfNuclearChargeAvailable(centerOfNuclearCharge());
      setAtomicCharges(Data::Type::GasteigerCharge);
      bool estimated(true);
      dipoleAvailable(dipoleFromPointCharges(), estimated); 
      // invalidate the energy
      energyAvailable(0.0, Info::KJMol);
   }else {
      delete cmd;
   }

   double t = time.elapsed() / 1000.0;
   QLOG_TRACE() << "Point group symmetry set to" << pointGroup << " time taken:" << t << "s";
}


void Molecule::translateToCenter(GLObjectList const& selection)
{
   Command::MoveObjects* cmd(new Command::MoveObjects(this, "Translate to center", true));

   // the ordering here is important!!
   Atom* atom;
   AtomList atomList;
   GLObjectList::const_iterator iter;
   for (iter = selection.begin(); iter != selection.end(); ++iter) {
       if ( (atom = qobject_cast<Atom*>(*iter)) ) atomList.append(atom); 
   }

   switch (atomList.size()) {
      case 1:
         translate(-atomList[0]->getPosition());
         break;
      case 2:
         translate(-atomList[0]->getPosition());
         alignToAxis(atomList[1]->getPosition());
         break;
      case 3:
         translate(-atomList[0]->getPosition());
         alignToAxis(atomList[1]->getPosition());
         rotateIntoPlane(atomList[2]->getPosition());
         break;
      default:
         translate(-centerOfNuclearCharge());
         break;
   }

   postCommand(cmd);

   softUpdate();
   centerOfNuclearChargeAvailable(centerOfNuclearCharge());
   reindexAtomsAndBonds();
   saveToCurrentGeometry();
   m_modified = true;
}


void Molecule::translate(Vec const& displacement)
{
   GLObjectList objects(findLayers<GLObject>(Children));
   GLObjectList::iterator iter;
   for (iter = objects.begin(); iter != objects.end(); ++iter) {
       (*iter)->setPosition((*iter)->getPosition()+displacement);
   }
   m_frame.setPosition(m_frame.position()+displacement);
}


void Molecule::rotate(Quaternion const& rotation)
{
   GLObjectList objects(findLayers<GLObject>(Children));
   GLObjectList::iterator iter;
   for (iter = objects.begin(); iter != objects.end(); ++iter) {
       (*iter)->setPosition(rotation.rotate((*iter)->getPosition()));
       (*iter)->setOrientation(rotation * (*iter)->getOrientation());
   }
   m_frame.setPosition(rotation.rotate(m_frame.position()));
   m_frame.setOrientation(rotation * m_frame.orientation());
}


// Aligns point along axis (default z-axis)
void Molecule::alignToAxis(Vec const& point, Vec const axis)
{
   rotate(Quaternion(point, axis));
}


// Rotates point into the plane defined by the normal vector
void Molecule::rotateIntoPlane(Vec const& pt, Vec const& axis, Vec const& normal)
{
   Vec pp(pt);
   pp.projectOnPlane(axis);
   rotate(Quaternion(pp, cross(normal, axis)));
}


// The following is essentially a wrapper around OBMol::FindChildren
AtomList Molecule::getContiguousFragment(Atom* first, Atom* second)
{
   AtomMap atomMap;
   BondMap bondMap;
   OBMol*  obMol(toOBMol(&atomMap, &bondMap));
 
   std::vector<OBAtom*> frag;
   obMol->FindChildren(frag, atomMap.key(first), atomMap.key(second));

   AtomList fragment;
   std::vector<OBAtom*>::iterator iter;
   for (iter = frag.begin(); iter != frag.end(); ++iter) {
       fragment.append(atomMap.value(*iter)); 
   }
   
   delete obMol;
   return fragment;
}


void Molecule::reperceiveBonds(bool postCmd)
{
   AtomMap atomMap;
   BondMap bondMap;
   OBAtom* obAtom;
   Vec     position;

   AtomList hydrogens;

   OBMol* obMol(new OBMol());
   AtomList::iterator atomIter;
   AtomList atoms(findLayers<Atom>(Children));
   obMol->BeginModify();

   for (atomIter = atoms.begin(); atomIter != atoms.end(); ++atomIter) {
       obAtom = obMol->NewAtom();
       atomMap.insert(obAtom, *atomIter);
       position = (*atomIter)->getPosition();
       int atomicNumber((*atomIter)->getAtomicNumber());
       obAtom->SetAtomicNum(atomicNumber);
       obAtom->SetVector(position.x, position.y, position.z);
       if (atomicNumber == 1) hydrogens.append(*atomIter);
   }

   obMol->SetTotalCharge(totalCharge());
   obMol->SetTotalSpinMultiplicity(multiplicity());
   obMol->EndModify();

   obMol->ConnectTheDots();
   obMol->PerceiveBondOrders();

   BondList removed(findLayers<Bond>(Children));
   PrimitiveList added(fromOBMol(obMol, &atomMap, &bondMap));

   // Special case code for Hydrogen atoms as OB doesn't seem to bond them
   QMap<QPair<Atom*, Atom*>, Bond*> h2Bonds;
   BondMap::iterator bondIter;
   for (bondIter = bondMap.begin(); bondIter != bondMap.end(); ++bondIter) {
       Bond* bond(*bondIter);
       if (bond->beginAtom()->getAtomicNumber() == 1 &&
           bond->endAtom()->getAtomicNumber() == 1) {
           h2Bonds.insert(qMakePair(bond->beginAtom(), bond->endAtom()), bond);
       }
   }

   int nHydrogens(hydrogens.size());
   double hydrogenBondLength(0.76);
   for (int i = 0; i < nHydrogens-1; ++i) {
       Atom* h1(hydrogens[i]);
       for (int j = i+1; j < nHydrogens; ++j) {
           Atom* h2(hydrogens[j]);
           if (Atom::distance(h1, h2) < hydrogenBondLength) {
              //qDebug() << "Found H2 bond" << Atom::distance(h1, h2);
              // Check if the two hydrogens are already bonded
              if (h2Bonds.contains(qMakePair(h1, h2)) || h2Bonds.contains(qMakePair(h2, h1))) {
              }else {
                 added.append(createBond(h1, h2, 1)); 
              }
           }
       }
   }

   if (postCmd) {
      Command::EditPrimitives* cmd(new Command::EditPrimitives("Reperceive bonds", this));
      cmd->remove(removed).add(added);
      postCommand(cmd);
   }else {
      appendPrimitives(added);
      added.clear();
      BondList::iterator iter;
      for (iter = removed.begin(); iter != removed.end(); ++iter) {
          added.append(*iter);
      }
      takePrimitives(added);
   }

   delete obMol;
}


// - - - - - Point Charges - - - - -

template <class T>
QList<double> Molecule::atomicCharges()
{
   QList<double> charges;
   if (m_currentGeometry) {
      unsigned nAtoms(m_currentGeometry->nAtoms());
      for (unsigned i = 0; i < nAtoms; ++i) {
          T& t(m_currentGeometry->getAtomicProperty<T>(i));
          charges << t.value();
      }
   }else {
      charges = zeroCharges();
   }
   return charges;
}


QList<double> Molecule::atomicCharges(Data::Type::ID type)
{
   QList<double> charges; 

   switch (type) {
      case Data::Type::GasteigerCharge:
         charges = gasteigerCharges();
         break;
      case Data::Type::MullikenCharge:
         charges = atomicCharges<Data::MullikenCharge>();
         break;
      case Data::Type::MultipoleDerivedCharge:
         charges = atomicCharges<Data::MultipoleDerivedCharge>();
         break;
      case Data::Type::ChelpgCharge:
         charges = atomicCharges<Data::ChelpgCharge>();
         break;
      case Data::Type::AtomicNumber:
         charges = atomicCharges<Data::AtomicNumber>();
         break;
      default:
         QLOG_WARN() << "Invalid charge type passed to Molecule::atomicCharges" << type;
         charges = zeroCharges();
         break;
   }

   return charges;
}


void Molecule::setAtomicCharges(Data::Type::ID type)
{
   QList<double> charges(atomicCharges(type));
   AtomList atoms(findLayers<Atom>(Children));
   int nAtoms(atoms.size());

   if (nAtoms != charges.size()) {
      QLOG_WARN() << "Invalid charges in Molecule::setAtomicCharges";
      return;
   }

   double totalCharge(0.0);
   for (int i = 0; i < nAtoms; ++i) {
       double charge(charges[i]);
       totalCharge += charge;
       atoms[i]->setCharge(charge);
   }
   m_chargeType = type;
   m_info.setCharge(totalCharge);
}


void Molecule::updateAtomicCharges()
{
   QAction* action(qobject_cast<QAction*>(sender()));
   if (action) {
      int data(action->data().toInt());
      Data::Type::ID type(static_cast<Data::Type::ID>(data));
      setAtomicCharges(type);
   }
   softUpdate();
}


QList<double> Molecule::gasteigerCharges()
{
   QList<double> charges;
   AtomMap atomMap;
   BondMap bondMap;

   OBMol* obMol(toOBMol(&atomMap, &bondMap));

   Atom* atom;
   FOR_ATOMS_OF_MOL(obAtom, obMol) {
      atom = atomMap.value(&*obAtom); 
      if (atom) charges << obAtom->GetPartialCharge();
   }

   delete obMol;
   return charges;
}


QList<double> Molecule::zeroCharges()
{
   QList<double> charges;
   AtomList atoms(findLayers<Atom>(Children));
   unsigned nAtoms(atoms.size());
   for (unsigned i = 0; i < nAtoms; ++i) {
       charges << 0.0;
   }
   return charges;
}


Vec Molecule::dipoleFromPointCharges()
{
   AtomList atomList(findLayers<Atom>(Children));
   if (atomList.isEmpty()) return Vec(0.0, 0.0, 0.0);

   Vec conc(centerOfNuclearCharge());
   Vec dipole;

   AtomList::iterator atom;
   for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
       double q((*atom)->getCharge());
       dipole += q*( (*atom)->getPosition() - conc);
   }

   double const ea0toDebye(0.393430307);
   dipole *= Constants::AngstromToBohr/ea0toDebye;

   return dipole;
}


BondList Molecule::getBonds(Atom* A)
{
  BondList allBonds(findLayers<Bond>(Children));
  BondList bonds;
  BondList::iterator bond;

   for (bond = allBonds.begin(); bond != allBonds.end(); ++bond) {
       if ( (*bond)->beginAtom() == A ||
            (*bond)->endAtom() == A) {
          bonds.append(*bond);
       }
   } 

   return bonds;
}


Bond* Molecule::getBond(Atom* A, Atom* B)
{
   BondList allBonds(findLayers<Bond>(Children));
   BondList::iterator bond;

   for (bond = allBonds.begin(); bond != allBonds.end(); ++bond) {
       if ( (*bond)->beginAtom() == A) {
          if ((*bond)->endAtom() == B) return *bond;
       }else if ((*bond)->beginAtom() == B) {
          if ((*bond)->endAtom() == A) return *bond;
       }
   } 

   return 0;
}


Vec Molecule::centerOfNuclearCharge()
{
   Vec center;
   int Z;
   double totalCharge(0.0);
   AtomList atoms(findLayers<Atom>(Children));

   AtomList::iterator iter;
   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       Z = (*iter)->getAtomicNumber();
       totalCharge += Z;
       center += Z * (*iter)->getPosition();
   }

   if (atoms.size() > 0) center = center / totalCharge;
   return center;
}



// ---------- Update functions ----------

template <class T>
void Molecule::update(boost::function<void(T&)> updateFunction)
{
   QList<T*> list(findLayers<T>(Children));
   typename QList<T*>::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       updateFunction(*(*iter));
   }
   softUpdate();
}


void Molecule::updateDrawMode(Primitive::DrawMode drawMode)
{
   m_drawMode = drawMode;
   update<Primitive>(boost::bind(&Primitive::setDrawMode, _1, m_drawMode));
}


void Molecule::updateAtomScale(double const scale)
{
   m_atomScale = scale;
   update<Atom>(boost::bind(&Atom::setScale, _1, m_atomScale));
}

void Molecule::updateSmallerHydrogens(bool smallerHydrogens)
{
   m_smallerHydrogens = smallerHydrogens;
   update<Atom>(boost::bind(&Atom::setSmallerHydrogens, _1, m_smallerHydrogens));
}


void Molecule::updateBondScale(double const scale)
{
   m_bondScale = scale;
   update<Bond>(boost::bind(&Bond::setScale, _1, m_bondScale));
}


void Molecule::updateChargeScale(double const scale)
{
   m_chargeScale = scale;
   update<Charge>(boost::bind(&Charge::setScale, _1, m_chargeScale));
}


int Molecule::totalCharge() const
{
   return m_info.getCharge();
}


int Molecule::multiplicity() const
{
   return m_info.getMultiplicity();
}


void Molecule::deleteProperties()
{
   QList<SpatialProperty*>::iterator iter;
   for (iter = m_properties.begin(); iter != m_properties.end(); ++iter) {
       delete (*iter);
   }
   m_properties.clear();

   if (m_atomicChargesMenu) {
      QMenu* menu(m_atomicChargesMenu->menu());
      if (menu) delete menu;
   }
}


void Molecule::initProperties()
{
   deleteProperties();

   m_properties << new RadialDistance();

   m_properties << new MeshIndex("Nuclei"); 

   QList<CubeData*> cubeFiles(findLayers<CubeData>(Children));
   QList<CubeData*>::iterator iter;
   for (iter = cubeFiles.begin(); iter != cubeFiles.end(); ++iter) {
       m_properties << (*iter)->createProperty();
   }

   QMenu* menu(new QMenu);
   m_atomicChargesMenu->setMenu(menu);
   QActionGroup* chargeTypes(new QActionGroup(menu));

   // Gasteiger
   QAction* action(menu->addAction("Gasteiger"));
   action->setCheckable(true);
   action->setChecked(true);
   action->setData(Data::Type::GasteigerCharge);
   chargeTypes->addAction(action);
   connect(action, SIGNAL(triggered()), this, SLOT(updateAtomicCharges()));
   m_properties.append( new PointChargePotential(Data::Type::GasteigerCharge, 
       "ESP (Gasteiger)", this) );

   if (!m_currentGeometry) return;

   // Mulliken
   if (m_currentGeometry->hasProperty<Data::MullikenCharge>()) {
      action = menu->addAction("Mulliken");
      action->setCheckable(true);
      action->setData(Data::Type::MullikenCharge);
      chargeTypes->addAction(action);
      connect(action, SIGNAL(triggered()), this, SLOT(updateAtomicCharges()));
      m_properties 
        << new PointChargePotential(Data::Type::MullikenCharge, "ESP (Mulliken)", this);
   }

   // Multipole Derived
   if (m_currentGeometry->hasProperty<Data::MultipoleDerivedCharge>()) {
      action = menu->addAction("Multipole Derived");
      action->setCheckable(true);
      action->setData(Data::Type::MultipoleDerivedCharge);
      chargeTypes->addAction(action);
      connect(action, SIGNAL(triggered()), this, SLOT(updateAtomicCharges()));
      m_properties 
        << new PointChargePotential(Data::Type::MultipoleDerivedCharge, "ESP (MDC)", this);
   }

   // ChelpG
   if (m_currentGeometry->hasProperty<Data::ChelpgCharge>()) {
      action = menu->addAction("CHELPG");
      action->setCheckable(true);
      action->setData(Data::Type::ChelpgCharge);
      chargeTypes->addAction(action);
      connect(action, SIGNAL(triggered()), this, SLOT(updateAtomicCharges()));
      m_properties 
        << new PointChargePotential(Data::Type::ChelpgCharge, "ESP (CHELPG)", this);
   }

   if (m_currentGeometry->hasProperty<Data::MultipoleExpansionList>()) {
      Data::MultipoleExpansionList& dma(
         m_currentGeometry->getProperty<Data::MultipoleExpansionList>());
      if (!dma.isEmpty()) {
         int maxOrder(dma.first()->order());
         MultipolePotential* dmaEsp;
         if (maxOrder >= 0) {
            dmaEsp = new MultipolePotential("ESP (DMA, charges)", 0, dma);
            m_properties << dmaEsp;
         }
         if (maxOrder >= 1) {
            dmaEsp = new MultipolePotential("ESP (DMA, dipoles)", 1, dma);
            m_properties << dmaEsp;
         }
         if (maxOrder >= 2) {
            dmaEsp = new MultipolePotential("ESP (DMA, quadrupoles)", 2, dma);
            m_properties << dmaEsp;
         }
         if (maxOrder >= 3) {
            dmaEsp = new MultipolePotential("ESP (DMA, octopoles)", 3, dma);
            m_properties << dmaEsp;
         }
      }
   }
}


QStringList Molecule::getAvailableProperties()
{
  QStringList properties;
  QList<SpatialProperty*>::iterator iter;
  for (iter = m_properties.begin(); iter != m_properties.end(); ++iter) {
      properties << (*iter)->text();
  }
  return properties;
}


Function3D Molecule::getPropertyEvaluator(QString const& name)
{
   QList<SpatialProperty*>::iterator iter;
   for (iter = m_properties.begin(); iter != m_properties.end(); ++iter) {
       if ( (*iter)->text() == name) {
          return (*iter)->evaluator();
       }
   }

   QLOG_WARN() << "Evaluator for property" << name << "not found, returning null function";
   return Function3D();
}


void Molecule::appendSurface(Data::Surface* surfaceData)
{
   m_bank.append(surfaceData);
   Layer::Surface* surfaceLayer(new Layer::Surface(*surfaceData));

qDebug() << "Need to check if surface needs to be oriented to the molecular frame";
// surfaceLayer->setFrame(getReferenceFrame());
   connect(surfaceLayer, SIGNAL(updated()), this, SIGNAL(softUpdate()));
   surfaceLayer->setCheckState(Qt::Checked);
   surfaceLayer->setCheckStatus(Qt::Checked);
   m_surfaceList.appendLayer(surfaceLayer);
   updated();
}


} } // end namespace IQmol::Layer
