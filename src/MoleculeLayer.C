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

#include "IQmol.h"
#include "Grid.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include "JobInfo.h"
#include "AtomLayer.h"
#include "BondLayer.h"
#include "DataLayer.h"
//#include "Sanderson.h"
#include "BaseParser.h"
#include "Preferences.h"
#include "ChargeLayer.h"
#include "FragmentLayer.h"
#include "SurfaceLayer.h"
#include "CubeDataLayer.h"
#include "UndoCommands.h"
#include "SpatialProperty.h"
#include "AtomicDensity.h"
#include "MarchingCubes.h"
#include "MoleculeLayer.h"
#include "ProgressDialog.h"
#include "ConstraintLayer.h"
#include "ConformerListLayer.h"

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
#include <QUrl>
#include <vector>

#include <QtDebug>


extern "C" void symmol_(int*, double*, double*, int*, char*);

using namespace OpenBabel;
using namespace qglviewer;

namespace IQmol {
namespace Layer {

Molecule::Molecule(QObject* parent) : Base(DefaultMoleculeName, parent), 
   m_drawMode(Primitive::BallsAndSticks), 
   m_atomScale(1.0), m_bondScale(1.0), m_chargeScale(1.0), 
   m_smallerHydrogens(true), 
   m_configurator(this), 
   m_jobInfo(0),
   m_surfaceList("Surfaces"), 
   m_info(this), 
   m_constraintList("Constraints"), 
   m_modified(false)
{
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | 
      Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
   setCheckState(Qt::Checked);
   setConfigurator(&m_configurator);

   connect(&m_configurator, SIGNAL(surfaceRequested(Layer::Surface*)),
      this, SLOT(appendSurface(Layer::Surface*)));
   
   // Okay, this is weird.  The OpenBabel plugins don't seem to get
   // initialized unless an OBConversion is used first.  This is a
   // problem if I try to use a force field before converting anything.
   // The following gets around this.

   //OBMol* mol(parseQChemMolecule("0 1\nH  0.00 0.00 0.00"));
   //if (mol) delete mol;
   OBConversion conv;
   conv.SetInFormat("xyz");
   m_openBabelESP =  new PointChargePotential("ESP (MM)");
   m_properties << new RadialDistance() << m_openBabelESP;

   // Add actions for the context menu
   connect(newAction("Configure"), SIGNAL(triggered()), 
      this, SLOT(configure()));
   connect(newAction("Select All"), SIGNAL(triggered()),
      this, SLOT(selectAll()));
   connect(newAction("Reperceive Bonds"), SIGNAL(triggered()), 
      this, SLOT(reperceiveBonds()));
   connect(newAction("Remove"), SIGNAL(triggered()), 
      this, SLOT(removeMolecule()));

   m_constraintList.setPersistentParent(this);
   m_constraintList.setPropertyFlags(RemoveWhenChildless);
   m_fileList.setPersistentParent(this);
   m_fileList.setPropertyFlags(RemoveWhenChildless);

   m_atomList.setPersistentParent(this);
   m_atomList.setPropertyFlags(RemoveWhenChildless);
   m_bondList.setPersistentParent(this);
   m_bondList.setPropertyFlags(RemoveWhenChildless);
}

Molecule::~Molecule()
{
   QList<SpatialProperty*>::iterator iter;
   for (iter = m_properties.begin(); iter != m_properties.end(); ++iter) {
       delete (*iter);
   }
}



void Molecule::setFile(QString const& fileName)
{
   m_inputFile.setFile(fileName);
   setText(m_inputFile.completeBaseName());
}


void Molecule::dragEnterEvent(QDragEnterEvent* event)
{  
   if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}


// This is not working
void Molecule::dropEvent(QDropEvent* event)
{
   qDebug() << "Drop Event on Molecule";
   
   QList<QUrl> urls(event->mimeData()->urls());
   QList<QUrl>::iterator iter;
   
   for (iter = urls.begin(); iter != urls.end(); ++iter) {
       qDebug() << (*iter).path();
       //appendData((*iter).path(), *this);
   }
   event->acceptProposedAction();
}


void Molecule::appendData(DataList& dataList)
{
   // !!! This needs fixing !!!
   // This is a bit cheesy, we rely on the QStandardItem text 
   // to determine the type of Layer.  
   QList<Base*> currentLayers(findLayers<Base>(Children));
   QStringList labels;

   labels << "Info";
   for (QList<Base*>::iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter) {
       labels << (*iter)->text();
   }

   DataList::iterator iter;
   CubeData* cubeData(0);
   Files* files(0);
   Atoms* atoms(0);
   Bonds* bonds(0);
   QString text;
   PrimitiveList primitiveList;
   DataList toSet;

   for (iter = dataList.begin(); iter != dataList.end(); ++iter) {
       text = (*iter)->text();
       
       if ((files = qobject_cast<Files*>(*iter))) {
          FileList fileList(files->getFiles());
          FileList::iterator file;
          for (file = fileList.begin(); file != fileList.end(); ++file) {
              files->removeLayer(*file);
              m_fileList.appendLayer(*file);
          }

       }else if (!labels.contains(text)) {
          labels << (*iter)->text();

          if ((atoms = qobject_cast<Atoms*>(*iter))) {
             AtomList atomList(atoms->getAtoms());
             AtomList::iterator atom;
             for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
                 atoms->removeLayer(*atom);
                 primitiveList.append(*atom);
             }
          }else if ((bonds = qobject_cast<Bonds*>(*iter))) {
             BondList bondList(bonds->getBonds());
             BondList::iterator bond;
             for (bond = bondList.begin(); bond != bondList.end(); ++bond) {
                 bonds->removeLayer(*bond);
                 primitiveList.append(*bond);
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

   appendPrimitives(primitiveList);

   for (iter = toSet.begin(); iter != toSet.end(); ++iter) {
       (*iter)->setMolecule(this);
       appendLayer(*iter);
   }

//   QList<ConformerList*> conformers(findLayers<ConformerList>(Children));
//   if (!conformers.isEmpty()) {
//      conformers.first()->setDefulatConformer();
//   }

   reindexAtomsAndBonds();
   determineSymmetry();
}


void Molecule::removeData(DataList&)
{
   // update display
}


bool Molecule::save(bool prompt)
{
   if (!m_modified && !prompt) return true;

   QFileInfo tmp(m_inputFile);
   if (tmp.fileName().isEmpty()) {
      tmp.setFile(text());
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
                 << tr("Sybyl Mol2") + " (*.mol2)";

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
      writeToFile(m_inputFile.filePath());
   } catch (Parser::IOError& ioerr) {
      success = false;
      QMsgBox::warning(0, "IQmol", ioerr.what());
   }

   return success;
}


void Molecule::writeToFile(QString const& filePath)
{     
   QFileInfo info(filePath);
   OBConversion conv;
   OBFormat *outFormat = conv.FormatFromExt(info.fileName().toAscii().data());
   if ( (!outFormat) || (!conv.SetOutFormat(outFormat))) throw Parser::ExtensionError();

   Preferences::LastFileAccessed(filePath);

   QString tmpName(filePath + ".iqmoltmp");
   std::ofstream ofs;
   ofs.open(tmpName.toAscii().data());
   if (!ofs) throw Parser::WriteError();

   AtomMap atomMap;
   BondMap bondMap;
   if (!conv.Write(toOBMol(atomMap, bondMap), &ofs)) throw Parser::FormatError();
   ofs.close();

   QFile target(filePath);
   if (target.exists()) target.remove();

   QFile tmpFile(tmpName);
   tmpFile.rename(filePath);

   Preferences::AddRecentFile(filePath);
   m_modified = false;
}


PrimitiveList Molecule::fromOBMol(OBMol* obMol, AtomMap* atomMap, BondMap* bondMap)
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

   Atom* atom;
   FOR_ATOMS_OF_MOL(obAtom, obMol) {
      Vec pos(obAtom->x(), obAtom->y(), obAtom->z());
      atom = atomMap->value(&*obAtom); 
      if (!atom) {
         atom = createAtom(obAtom->GetAtomicNum(), pos);
         addedPrimitives.append(atom);
         atomMap->insert(&*obAtom, atom);
      }
      atom->setCharge(obAtom->GetPartialCharge());
      atom->setPosition(pos);
   }

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

   QLOG_DEBUG() << "Retrieving charge and multiplicity from OBMol:" 
            << obMol->GetTotalCharge() << obMol->GetTotalSpinMultiplicity();
   m_info.setCharge(obMol->GetTotalCharge());
   m_info.setMultiplicity(obMol->GetTotalSpinMultiplicity());

   if (deleteAtomMap) delete atomMap;
   if (deleteBondMap) delete bondMap;
   return addedPrimitives;
}


OBMol* Molecule::toOBMol(AtomMap& atomMap, BondMap& bondMap)
{
   OBAtom* obAtom;
   OBBond* obBond;
   Vec     position;

   OBMol* obMol(new OBMol());
   atomMap.clear();
   bondMap.clear();

   AtomList atoms(findLayers<Atom>(Children));
   AtomList::iterator atomIter;

   obMol->BeginModify();

   QLOG_DEBUG() << "Setting OBMol charge and multiplicity:" << totalCharge() << multiplicity();
   QLOG_TRACE() << "Automatic charge determination =" << obMol->AutomaticPartialCharge();
   QLOG_TRACE() << "Has Partial charges percieved = " << obMol->HasPartialChargesPerceived();
   obMol->SetTotalCharge(totalCharge());
   obMol->SetTotalSpinMultiplicity(multiplicity());

   for (atomIter = atoms.begin(); atomIter != atoms.end(); ++atomIter) {
       obAtom = obMol->NewAtom();
       atomMap.insert(obAtom, *atomIter);

       position = (*atomIter)->getPosition();
       obAtom->SetAtomicNum((*atomIter)->m_atomicNumber);
       obAtom->SetVector(position.x, position.y, position.z);
   }

   BondList bonds(findLayers<Bond>(Children));
   BondList::iterator bondIter;

   for (bondIter = bonds.begin(); bondIter != bonds.end(); ++bondIter) {
       obBond = obMol->NewBond();
       bondMap.insert(obBond, *bondIter);
       obBond->SetBondOrder((*bondIter)->m_order);

       obAtom = atomMap.key((*bondIter)->m_begin);
       obBond->SetBegin(obAtom);
       obAtom->AddBond(obBond);
       obAtom = atomMap.key((*bondIter)->m_end);
       obBond->SetEnd(obAtom);
       obAtom->AddBond(obBond);
   }

   obMol->EndModify();

   return obMol;
}



OBMol* Molecule::toOBMol() 
{
   AtomList atoms(findLayers<Atom>(Children));
   QString s(QString::number(atoms.size()));
   s += "\n\n" + coordinatesAsString();

   OBConversion conv;
   conv.SetInFormat("xyz");
   OBMol* mol = new OBMol();
   std::istringstream iss(std::string(s.toLatin1()));
   conv.Read(mol, &iss);
   qDebug() << "Setting OBMol charge and multiplicity (2) to" << totalCharge() << multiplicity();
   mol->SetTotalCharge(totalCharge());
   mol->SetTotalSpinMultiplicity(multiplicity());
   return mol;
}


QString Molecule::coordinatesAsString(bool const selectedOnly)
{
   AtomList atomList(findLayers<Atom>(Children));
   Vec position;
   QString coords;

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

   coords.chop(1);
   return coords;
}


void Molecule::selectAll()
{
   PrimitiveList primitiveList(findLayers<Primitive>(Visible | Children));
   //PrimitiveList primitiveList(findVisibleChildren<Primitive>());

   PrimitiveList::iterator iter;
   for (iter = primitiveList.begin(); iter != primitiveList.end(); ++iter) {
       select((*iter)->QStandardItem::index(), QItemSelectionModel::Select);
   }
}


QString Molecule::constraintsAsString() 
{
   ConstraintList constraints(findLayers<Constraint>(Visible|Children));
   if (constraints.isEmpty()) return QString();

   ConstraintList fixed;
   ConstraintList internal;

   ConstraintList::iterator iter;
   for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
       if ((*iter)->type() == Constraint::Position) {
          fixed.append(*iter);
       }else {
          internal.append(*iter);
       }
   }

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


void Molecule::appendConstraint(Constraint* constraint)
{
   // Check to see if we already have a constraint involving these atoms.
   ConstraintList constraints(findLayers<Constraint>(Children));
   ConstraintList::iterator iter;
   for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
       if (*(*iter) == *constraint) {
          *(*iter) = *constraint;
          delete constraint;
          applyConstraint(*iter);
          return;
       }
   }

   //if (!m_constraintList.hasChildren()) appendRow(&m_constraintList);
   //m_constraintList.appendRow(constraint);
   m_constraintList.appendLayer(constraint);

   applyConstraint(constraint);
   connect(constraint, SIGNAL(updated()), this, SLOT(constraintUpdated()));
   connect(constraint, SIGNAL(invalid()), this, SLOT(deleteConstraint()));
   updated();
}


void Molecule::deleteConstraint()
{
   Constraint* constraint(qobject_cast<Constraint*>(sender()));
   //m_constraintList.takeRow(constraint->row());
   m_constraintList.removeLayer(constraint);
}


void Molecule::constraintUpdated()
{
   Constraint* constraint(qobject_cast<Constraint*>(sender()));
   applyConstraint(constraint);
}


void Molecule::applyConstraint(Constraint* constraint)
{
   Command::ApplyConstraint* cmd = new Command::ApplyConstraint(this);

   switch (constraint->constraintType()) {
      case Constraint::Invalid:
         delete cmd;
         return;
         break;
      case Constraint::Position:
         QLOG_TRACE() << "Applying position constraint";
         applyPositionConstraint(constraint);
         break;
      case Constraint::Distance:
         QLOG_TRACE() << "Applying distance constraint";
         applyDistanceConstraint(constraint);
         break;
      case Constraint::Angle:
         QLOG_TRACE() << "Applying angle constraint";
         applyAngleConstraint(constraint);
         break;
      case Constraint::Torsion:
         QLOG_TRACE() << "Applying torsion constraint";
         applyTorsionConstraint(constraint);
         break;
   }

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
   AtomList allAtoms(findLayers<Atom>(Children));
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
   Vec delta(value*(a-b).unit());
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
   if (messageBox.clickedButton() == nowButton) minimizeEnergy(Preferences::DefaultForceField());
}


void Molecule::appendDataFile(Data* data)
{
   m_fileList.appendLayer(data);
   //if (!m_fileList.hasChildren()) appendRow(&m_fileList);
   //m_fileList.appendRow(data);
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
   Fragment* fragment;

   AtomList atoms(findLayers<Atom>(Children));
   int initialNumberOfAtoms(atoms.size());
   QLOG_TRACE() << "Appending" << primitives.size() 
               << "primitives to Molecule with" << initialNumberOfAtoms << "atoms"; 

   PrimitiveList::const_iterator primitive;
   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {

       if ( (atom = qobject_cast<Atom*>(*primitive)) ) {
          m_atomList.appendLayer(*primitive);

       }else if ( (bond = qobject_cast<Bond*>(*primitive)) ) {
          m_bondList.appendLayer(*primitive);

       }else if ( (charge = qobject_cast<Charge*>(*primitive)) ) {
          if (!m_chargeList.hasChildren()) appendRow(&m_chargeList);
          m_chargeList.appendRow(*primitive);

       }else if ( (fragment = qobject_cast<Fragment*>(*primitive)) ) {
          appendRow(*primitive);

       } else {
          QMsgBox::warning(0, "IQmol", "Attempt to add unknown primitive type to molecule");
       }
   }

   reindexAtomsAndBonds();
   atoms = findLayers<Atom>(Children);

   if (atoms.size() > initialNumberOfAtoms) {
      if (initialNumberOfAtoms == 0) insertRow(0,&m_info);
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
   Fragment* fragment;

   AtomList atoms(findLayers<Atom>(Children));
   int initialNumberOfAtoms(atoms.size());
   QLOG_TRACE() << "Taking" << primitives.size() 
                << "primitives from molecule with" << initialNumberOfAtoms << "atoms";

   PrimitiveList::const_iterator primitive;
   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       QLOG_TRACE() << "   removing row" << (*primitive)->row();

       // Make sure out primitive is no longer selected
       (*primitive)->deselect();

       if ( (atom = qobject_cast<Atom*>(*primitive)) ) {
          //22m_atomList.takeRow((*primitive)->row());
          //22if (!m_atomList.hasChildren()) takeRow(m_atomList.row());
          m_atomList.removeLayer(*primitive);

       }else if ( (bond = qobject_cast<Bond*>(*primitive)) ) {
          //22m_bondList.takeRow((*primitive)->row());
          //22if (!m_bondList.hasChildren()) takeRow(m_bondList.row());
          m_bondList.removeLayer(*primitive);

       }else if ( (charge = qobject_cast<Charge*>(*primitive)) ) {
          m_chargeList.takeRow((*primitive)->row());
          if (!m_chargeList.hasChildren()) takeRow(m_chargeList.row());

       }else if ( (fragment = qobject_cast<Fragment*>(*primitive)) ) {
          takeRow((*primitive)->row());

       } else {
          QMsgBox::warning(0, "IQmol", "Atempt to remove unknown primitive type to molecule");
       }
   }

   reindexAtomsAndBonds();
   atoms = findLayers<Atom>(Children);
   QLOG_TRACE() << "Number of atoms now" <<atoms.size();

   if (atoms.isEmpty()) {
      takeRow(m_info.row());
   }else if (atoms.size() < initialNumberOfAtoms) {
      updateInfo();
   }
   m_modified = true;
   updated();
}


void Molecule::updateInfo()
{
   AtomList atoms(findLayers<Atom>(Children));
   m_info.clear();
   m_info.addAtoms(atoms);
   determineSymmetry();
}


void Molecule::deleteSelection()
{
   PrimitiveList deleteTargets;

   BondList allBonds(findLayers<Bond>(Visible|Children));
   QSet<Bond*> deleteBonds;
   BondList::iterator bond;

   for (bond = allBonds.begin(); bond != allBonds.end(); ++bond) {
       if ((*bond)->isSelected()) deleteBonds.insert(*bond);
   } 

   AtomList allAtoms(findLayers<Atom>(Visible|Children));
   AtomList::iterator atom;

   for (atom = allAtoms.begin(); atom != allAtoms.end(); ++atom) {
       if ((*atom)->isSelected()) {
          deleteTargets.append(*atom);

         // Must delete any bond attached to atoms we are deleting.
         for (bond = allBonds.begin(); bond != allBonds.end(); ++bond) {
             if ( (*bond)->m_begin == *atom || (*bond)->m_end == *atom ) {
                deleteBonds.insert(*bond);
             }
         } 
      }
   } 

   QSet<Bond*>::iterator iter;
   for (iter = deleteBonds.begin(); iter != deleteBonds.end(); ++iter) {
       deleteTargets.append(*iter);
   }

   ChargeList allCharges(findLayers<Charge>(Visible|Children));
   ChargeList::iterator charge;

   for (charge = allCharges.begin(); charge != allCharges.end(); ++charge) {
       if ((*charge)->isSelected()) deleteTargets.append(*charge);
   } 

   if (!deleteTargets.isEmpty()) {
      QLOG_DEBUG() << "Deleting" << deleteTargets.size() << "selected objects";
      PrimitiveList::iterator iter;
      for (iter = deleteTargets.begin(); iter != deleteTargets.end(); ++iter) {
          QLOG_TRACE() << "   Target: "<< (*iter)->text();
      }
      Command::RemovePrimitives* cmd(new Command::RemovePrimitives(this, deleteTargets));
      postCommand(cmd);
   }
   m_modified = true;
}


PrimitiveList Molecule::getSelected(bool danglingBonds)
{
   PrimitiveList selected;

   AtomList allAtoms(findLayers<Atom>(Visible|Children));
   AtomList::iterator atom;
   for (atom = allAtoms.begin(); atom != allAtoms.end(); ++atom) {
       if ((*atom)->isSelected()) selected.append(*atom);
   } 

   BondList allBonds(findLayers<Bond>(Visible|Children));
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

   ChargeList allCharges(findLayers<Charge>(Visible|Children));
   ChargeList::iterator charge;
   for (charge = allCharges.begin(); charge != allCharges.end(); ++charge) {
       if ((*charge)->isSelected()) selected.append(*charge);
   } 

   return selected;
}


void Molecule::groupSelection()
{
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
}


void Molecule::ungroupSelection() 
{
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
}


void Molecule::updateAtomOrder()
{
   AtomList atoms(findLayers<Atom>(Children));
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


void Molecule::reindexAtomsAndBonds()
{
   QList<double> charges;
   QList<Vec> coordinates;

   AtomList atoms(findLayers<Atom>(Children));
   QLOG_DEBUG() << "Reindexing" << atoms.size() << "atoms";
   for (int i = 0; i < atoms.size(); ++i) {
       atoms[i]->setIndex(i+1);
       charges << atoms[i]->getCharge();
       coordinates << atoms[i]->getPosition();
   }
   m_openBabelESP->update(charges, coordinates);

   BondList bonds(findLayers<Bond>(Children));
   for (int i = 0; i < bonds.size(); ++i) {
       bonds[i]->setIndex(i+1);
   }
}


double Molecule::radius()
{
   double radius(0);
   PrimitiveList primitives(findLayers<Primitive>(Children));
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


JobInfo* Molecule::jobInfo()
{
   if (m_jobInfo) disconnect(m_jobInfo, SIGNAL(updated()), this, SLOT(jobInfoChanged()));
   // This is sloppy.  We create a JobInfo object here, but don't delete it in
   // the dtor as we may have passed the JobInfo on to a Process or Server.
   m_jobInfo = new JobInfo();

   m_jobInfo->set(JobInfo::Charge, totalCharge());
   m_jobInfo->set(JobInfo::Multiplicity, multiplicity());
   m_jobInfo->set(JobInfo::Coordinates, coordinatesAsString());
   m_jobInfo->set(JobInfo::Constraints, constraintsAsString());

   QString name;
qDebug() << "Creating JobInfo in MoleculeLayer.C";
qDebug() << "text()   = " << text();


   if (m_inputFile.filePath().isEmpty()) {
      QFileInfo fileInfo(Preferences::LastFileAccessed());
qDebug() << "LastFile = " << Preferences::LastFileAccessed();
      m_jobInfo->set(JobInfo::LocalWorkingDirectory, fileInfo.path());
      m_jobInfo->set(JobInfo::BaseName, text());
   }else {
      m_jobInfo->set(JobInfo::LocalWorkingDirectory, m_inputFile.path());
      m_jobInfo->set(JobInfo::BaseName, m_inputFile.completeBaseName());
      name =  + "/" + m_inputFile.completeBaseName() + ".inp";
   }

   connect(m_jobInfo, SIGNAL(updated()), this, SLOT(jobInfoChanged()));
   return m_jobInfo;
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
   OBMol* obMol(toOBMol(atomMap, bondMap));
   obMol->Translate(displacement);
   obMol->BeginModify();
   obMol->EndModify();
   obMol->AddHydrogens(false,false);
   obMol->Translate(-displacement);
   obMol->BeginModify();
   obMol->EndModify();

   Command::AddHydrogens* cmd  = 
      new Command::AddHydrogens(this, fromOBMol(obMol, &atomMap, &bondMap));
   postCommand(cmd);

   delete obMol;
   m_modified = true;
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
      msg += forceFieldName + "\nUnable to optimize structure";
      QMsgBox::warning(0, "IQmol", msg);
      QLOG_ERROR() << "Failed to load force field";
      return;
   }

   forceField->SetLogFile(&std::cout);
   forceField->SetLogLevel(OBFF_LOGLVL_LOW);

   AtomMap* atomMap(new AtomMap());
   BondMap* bondMap(new BondMap());
   OBMol* obMol(toOBMol(*atomMap, *bondMap));

   // constraints
   OBFFConstraints obffconstraints;
   ConstraintList  constraints(findLayers<Constraint>(Visible|Children));

   if (!constraints.isEmpty()) {
      QLOG_DEBUG() << "Enforcing" << constraints.size() << "active constraints";
      ConstraintList::iterator iter;
      for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
          (*iter)->addTo(obffconstraints);
      }
   }

   if (!forceField->Setup(*obMol, obffconstraints)) {
      QString msg("Failed to setup force field for molecule.  ");
      msg += text() + "\nUnable to optimize structure.";
      QMsgBox::warning(0, "IQmol", msg);
      QLOG_ERROR() << "Failed to setup force field";
      return;
   }

   double energy, convergence(1e-5f);
   int count(0), nSteps(1001), maxSteps(1000);
   QString mesg(forceFieldName + " energy: ");

   forceField->SetConformers(*obMol);
   forceField->ConjugateGradientsInitialize(maxSteps, convergence);
   Command::MinimizeStructure* cmd = new Command::MinimizeStructure(this);
  
   while (forceField->ConjugateGradientsTakeNSteps(nSteps)) {
      if (forceField->GetCoordinates(*obMol)) {
         fromOBMol(obMol, atomMap);
         softUpdate();
      }else {
         QLOG_ERROR() << "GetCoordinates failed in minimizeEnergy";
      }
      energy = forceField->Energy();
      postMessage(mesg + QString::number(energy, 'f', 4));
      count += nSteps;
   }
  
   forceField->GetCoordinates(*obMol);
   fromOBMol(obMol, atomMap);

   energy = forceField->Energy();
   if (count < maxSteps) {
      cmd->setMessage(mesg + QString::number(energy, 'f', 4));
      energyAvailable(energy, Info::KJmol);
   }else {
      energyAvailable(0.0, Info::KJmol);
      cmd->setMessage(mesg + QString::number(energy, 'f', 4) + QString(" unconverged"));
      QString msg("Force field minimization failed to converge within "); 
      msg + QString::number(maxSteps) + " cycles.\n";
      msg += "Try minimizing again, or select a different force field";
      QMsgBox::warning(0, "IQmol", msg);
      QLOG_WARN() << "Minimization did not converge";
   }

   postCommand(cmd);
   m_modified = true;

   reindexAtomsAndBonds();
   delete obMol;

}


// This is a nasty function used to obtain naked pointers for the symmol routine
void Molecule::symmetrize(double tolerance, bool updateCoordinates)
{
   QLOG_TRACE() << "Determining symmetry";
   QTime time;
   time.start();

   AtomList atomList(findLayers<Atom>(Children));
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
         for (iter = atomList.begin(); iter != atomList.end(); ++iter) {
             position.x = coordinates[3*cnt  ];
             position.y = coordinates[3*cnt+1];
             position.z = coordinates[3*cnt+2];
             (*iter)->setPosition(position);
             ++cnt;
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
   }else {
      delete cmd;
   }

   double t = time.elapsed() / 1000.0;
   QLOG_TRACE() << "Point group symmetry set to" << pointGroup << " time taken:" << t << "s";
   computeMoments();
}


void Molecule::computeMoments()
{
   AtomList atomList(findLayers<Atom>(Children));
   if (atomList.isEmpty()) return;
   Vec coch(centerOfNuclearCharge());
   Vec dipole;
   double q;

   AtomList::iterator atom;
   for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
       q = (*atom)->getCharge();
       dipole += q*( (*atom)->getPosition() - coch);
   }

   double const ea0toDebye(0.393430307);
   dipole *= AngstromToBohr/ea0toDebye;

   bool estimated(true);
   dipoleAvailable(dipole, estimated); 
}


// The following is essentially a wrapper around OBMol::FindChildren
AtomList Molecule::getContiguousFragment(Atom* first, Atom* second)
{
   AtomMap atomMap;
   BondMap bondMap;
   OBMol*  obMol(toOBMol(atomMap, bondMap));
 
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


void Molecule::reperceiveBonds()
{
   AtomList atomList(findLayers<Atom>(Children));
   BondList bondList(findLayers<Bond>(Children));
   PrimitiveList primitiveList;

   AtomList::iterator atom;
   for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
       primitiveList.append(*atom); 
   }

   BondList::iterator bond;
   for (bond = bondList.begin(); bond != bondList.end(); ++bond) {
       primitiveList.append(*bond); 
   }

   OBMol* obMol(toOBMol());

   
   Command::EditPrimitives* cmd;
   cmd = new Command::EditPrimitives("Reperceive bonds", this, primitiveList, fromOBMol(obMol));
   postCommand(cmd);

   //takePrimitives(primitiveList);
   //appendPrimitives(fromOBMol(obMol));

   //PrimitiveList::iterator primitive;
   //for (primitive = primitiveList.begin(); primitive != primitiveList.end(); ++primitive) {
   //    delete (*primitive);
   //}
}


void Molecule::setMullikenCharges()
{
   AtomList::iterator iter;
   AtomList atoms(findLayers<Atom>(Children));
   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       (*iter)->setCharge(0.0);
   }
   qDebug() << "Warning: setting Mulliken charges to 0";
}


void Molecule::setGasteigerCharges()
{
   AtomMap atomMap;
   BondMap bondMap;

   OBMol* obMol(toOBMol(atomMap, bondMap));

   Atom* atom;
   FOR_ATOMS_OF_MOL(obAtom, obMol) {
      atom = atomMap.value(&*obAtom); 
      if (atom) atom->setCharge(obAtom->GetPartialCharge());
   }

   delete obMol;
}


void Molecule::setSandersonCharges()
{
   QList<Vec> coordinates;
   QList<int> atomicNumbers;

   AtomList atoms(findLayers<Atom>(Children));
   AtomList::iterator iter;
   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       coordinates.append( (*iter)->getPosition() );
       atomicNumbers.append( (*iter)->getAtomicNumber());
   }
   // TMP
   for (int i = 0; i < atoms.size(); ++i) {
       atoms[i]->setCharge(0.0);
   }
   
/* No Sanderson at the moment
   qDebug() << "Total charge set to" << totalCharge();
   Sanderson eem(atomicNumbers, coordinates, totalCharge());
   QList<double> charges = eem.solve();

   for (int i = 0; i < atoms.size(); ++i) {
       atoms[i]->setCharge(charges[i]);
   }
*/
}


Bond* Molecule::getBond(Atom* A, Atom* B)
{
   BondList allBonds(findLayers<Bond>(Visible|Children));
   BondList::iterator bond;

   for (bond = allBonds.begin(); bond != allBonds.end(); ++bond) {
       if ( (*bond)->m_begin == A) {
          if ((*bond)->m_end == B) return *bond;
       }else if ((*bond)->m_begin == B) {
          if ((*bond)->m_end == A) return *bond;
       }
   } 

   return 0;
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
   m_modified = true;
   computeMoments();
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
   centerOfChargeAvailable(center);
   return center;
}


void Molecule::translate(Vec const& displacement)
{
   PrimitiveList primitives(findLayers<Primitive>(Children));
   PrimitiveList::iterator iter;
   for (iter = primitives.begin(); iter != primitives.end(); ++iter) {
       (*iter)->setPosition((*iter)->getPosition()+displacement);
   }
}


void Molecule::rotate(Quaternion const& rotation)
{
   PrimitiveList primitives(findLayers<Primitive>(Children));
   PrimitiveList::iterator iter;
   for (iter = primitives.begin(); iter != primitives.end(); ++iter) {
       (*iter)->setPosition(rotation.rotate((*iter)->getPosition()));
   }
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


void Molecule::jobInfoChanged()
{ 
   if (m_jobInfo) {
      setText(m_jobInfo->get(JobInfo::BaseName)); 
      m_info.setCharge(m_jobInfo->getCharge());
      m_info.setMultiplicity(m_jobInfo->getMultiplicity());
   }
}


int Molecule::totalCharge() const
{
   return m_info.getCharge();
}


int Molecule::multiplicity() const
{
   return m_info.getMultiplicity();
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
       if ( (*iter)->text() == name) return (*iter)->evaluator();
   }

   return Function3D();
}


void Molecule::appendSurface(Surface* surface)
{
   
   switch (surface->gridDataType().type()) {
      case Grid::DataType::VanDerWaals: {
         double scale(surface->isovalue());
         double solventRadius(0.0);
         calculateVanDerWaals(surface, scale, solventRadius);
          } break;

      case Grid::DataType::Promolecule:
         calculateSuperposition<AtomicDensity::AtomShellApproximation>(surface);
         break;

      case Grid::DataType::SolventExcluded: {
         // The solvent radius is masqurading as the isovalue here.
         double scale(1.0);
         double solventRadius(surface->isovalue());
         calculateVanDerWaals(surface, scale, solventRadius);
         } break;

      case Grid::DataType::SID:
         calculateSID(surface);
         break;

      default:
         return;
         break;
   }

   if (!m_surfaceList.hasChildren()) appendRow(&m_surfaceList);

   connect(surface, SIGNAL(updated()), this, SIGNAL(softUpdate()));
   surface->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
   surface->setCheckState(Qt::Checked);
   m_surfaceList.appendRow(surface);
   updated();
}


template <class T>
void Molecule::calculateSuperposition(Surface* surface)
{
   QMap<int, T*> uniqueAtoms;
   QList<AtomicDensity::Base*> atomList;
   QList<Vec> coordinates;
   int atomicNumber;
   T* atom(0);

   AtomList atoms(findLayers<Atom>(Children));
   AtomList::iterator iter;
   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       atomicNumber = (*iter)->getAtomicNumber();

       if (uniqueAtoms.contains(atomicNumber)) {
          atom = uniqueAtoms[atomicNumber];
       }else {
          atom = new T(atomicNumber);
          uniqueAtoms.insert(atomicNumber, atom);
       }
       coordinates.append( (*iter)->getPosition() );
       atomList.append(atom); 
   }

   PromoleculeDensity rho("Superposition", atomList, coordinates);
   
   // Set up the grid
   Vec min, max;
   rho.boundingBox(min, max);
   Grid grid(surface->gridDataType(), Grid::Size(min, max, surface->quality()));

   ProgressDialog progressDialog("Computing Surface",0);
   progressDialog.setInfo("Calculating density data");
   connect(&grid, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
   progressDialog.show();

   grid.generateData(rho.evaluator());

   MarchingCubes mc(&grid);
   surface->setSurfaceData(mc.generateSurface(surface->isovalue())); 
}


void Molecule::calculateVanDerWaals(Surface* surface, double const scale, 
   double const solventRadius)
{
   Layer::Surface::Data surfaceData;

   QList<AtomicDensity::VanDerWaals*> vdwAtoms;
   AtomicDensity::VanDerWaals* atom;

   AtomList atoms(findLayers<Atom>(Children));
   AtomList::iterator iter;

   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       atom = new AtomicDensity::VanDerWaals( (*iter)->getAtomicNumber(), 
                      (*iter)->getPosition(), scale, solventRadius);
       vdwAtoms.append(atom);
   }

   QList<AtomicDensity::VanDerWaals*>::iterator iter2;
   for (iter2 = vdwAtoms.begin(); iter2 != vdwAtoms.end(); ++iter2) {
       surfaceData += (*iter2)->surfaceData(surface->quality(), vdwAtoms);
   }

   surface->setSurfaceData(surfaceData);

   for (iter2 = vdwAtoms.begin(); iter2 != vdwAtoms.end(); ++iter2) {
       delete (*iter2);
   }
}





void Molecule::calculateSID(Surface* surface)
{
   QList<Vec> coordinates;
   QList<int> atomicNumbers;

   AtomList atoms(findLayers<Atom>(Children));
   AtomList::iterator iter;
   for (iter = atoms.begin(); iter != atoms.end(); ++iter) {
       coordinates.append( (*iter)->getPosition() );
       atomicNumbers.append( (*iter)->getAtomicNumber());
   }

   QList<double> charges;
/* No Sanderson at the moment
   qDebug() << "Total charge set to" << totalCharge();
   Sanderson eem(atomicNumbers, coordinates, totalCharge());
   charges = eem.solve();
*/

   for (int i = 0; i < atoms.size(); ++i) {
       charges.append(atoms[i]->getCharge());
   }

   QList<AtomicDensity::Base*> atomList;
   AtomicDensity::SuperpositionIonicDensities* atom;
   for (int i = 0; i < atoms.size(); ++i) {
       atom = new AtomicDensity::SuperpositionIonicDensities(atomicNumbers[i], charges[i]); 
       atomList.append(atom); 
   }


   PromoleculeDensity rho("Superposition", atomList, coordinates);
   
   // Set up the grid
   Vec min, max;
   rho.boundingBox(min, max);
   Grid grid(surface->gridDataType(), Grid::Size(min, max, surface->quality()));

   ProgressDialog progressDialog("Computing Surface",0);
   progressDialog.setInfo("Calculating density data");
   connect(&grid, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
   progressDialog.show();

   grid.generateData(rho.evaluator());

   MarchingCubes mc(&grid);
   surface->setSurfaceData(mc.generateSurface(surface->isovalue())); 
}


} } // end namespace IQmol::Layer
