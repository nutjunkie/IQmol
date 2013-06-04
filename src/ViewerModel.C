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

#include "ViewerModel.h"
#include "AtomLayer.h"
#include "ChargeLayer.h"
#include "FrequenciesLayer.h"
#include "ConstraintLayer.h"
#include "ConformerListLayer.h"
#include "Preferences.h"
#include "UndoCommands.h"
#include "DataLayer.h"
#include "OpenBabelParser.h"
#include "QMsgBox.h"
#include "JobInfo.h"
#include "QVariantPointer.h"
#include "SymmetryToleranceDialog.h"
#include <QStringList>
#include <QStandardItem>
#include "QsLog.h"
#include <QUrl>
#include <QDir>
#include <QClipboard>
#include <cmath>


#include <QtDebug>


using namespace qglviewer;

namespace IQmol {

ViewerModel::ViewerModel(QWidget* parent) : QStandardItemModel(0, 1, parent),
   m_parent(parent), m_global("Global", m_parent),
   m_symmetryTolerance(Preferences::SymmetryTolerance()), 
   m_forceField(Preferences::DefaultForceField()), m_updateEnabled(true)
{
   QStringList labels;
   labels << "Model View";
   setHorizontalHeaderLabels(labels);
   invisibleRootItem()->setCheckState(Qt::Checked);

   m_global.setFlags(Qt::ItemIsEnabled);
   appendRow(&m_global);

   m_global.appendRow(&m_background);
   m_global.appendRow(&m_axes);
   m_global.appendRow(&m_mesh);

   connect(&m_background, SIGNAL(updated()), this, SIGNAL(updated()));
   connect(&m_background, SIGNAL(foregroundColorChanged(QColor const&)), 
      this, SIGNAL(foregroundColorChanged(QColor const&))); 
   connect(&m_axes, SIGNAL(updated()), this, SIGNAL(updated()));
   connect(&m_mesh, SIGNAL(updated()), this, SIGNAL(updated()));

   connect(this, SIGNAL(sceneRadiusChanged(double const)), 
      &m_axes, SLOT(setSceneRadius(double const)));

   connect(this, SIGNAL(sceneRadiusChanged(double const)), 
      &m_mesh, SLOT(setSceneRadius(double const)));

   connect(this, SIGNAL(itemChanged(QStandardItem*)), 
      this, SLOT(checkItemChanged(QStandardItem*)));

   Layer::Molecule* mol(newMolecule());
   appendRow(mol);
   changeActiveViewerMode(Viewer::BuildAtom);
   sceneRadiusChanged(DefaultSceneRadius);
}


QStringList ViewerModel::mimeTypes () const
{
   QStringList types;
   types << "text/uri-list";
   return types;
}


Qt::DropActions ViewerModel::supportedDropActions () const
{
    return Qt::CopyAction | Qt::MoveAction;
}


bool ViewerModel::dropMimeData(QMimeData const* data, Qt::DropAction, int, int,
   const QModelIndex& index) 
{
   qDebug() << "Viewer Model drop activated";
   QStandardItem* item = itemFromIndex(index);

   if (item && data) {
      qDebug() << "Item dumped on:" << item->text();

      // parse the file
      // check if the molecules are the same (just the formula?)
      // append data layers excluding Atoms, Bonds, Info
      // create new conformer?
/*
   Layer::Base* layer;
   if ((layer = dynamic_cast<Layer::Base*>(item))) {
      layer->configure();
   }
*/
      if (data->hasUrls()) {
         QList<QUrl> urls(data->urls());
         QList<QUrl>::iterator iter;

         for (iter = urls.begin(); iter != urls.end(); ++iter) {
            qDebug() << "from dropMimeData:" << (*iter);
         }
      }
   }
   
   return true;
}


bool ViewerModel::open(QString const& fileName)
{
   QLOG_INFO() << "Opening file:" << fileName;

   QString name;
   QFileInfo fileInfo(fileName);
   DataList dataList;

   // If we are opening a directory, then we look for all files with the same
   // base name as the directory, which is how things are set up when IQmol is
   // used to run the calculation.  E.g. if the directory is ~/Ethane, then we 
   // look for all files of the form ~/Ethane/Ethane.*  
   if (fileInfo.isDir()) {
      QDir dir(fileName);

      QStringList list;
      list << dir.dirName() + ".*";
      dir.setNameFilters(list);

      QDir::Filters filters(QDir::Files | QDir::Readable);
      list = dir.entryList(filters);

      QStringList::iterator iter;
      for (iter = list.begin(); iter != list.end(); ++iter) {
          QFileInfo info(dir, *iter);
          *iter = info.filePath();
          QLOG_INFO() << "  " << *iter;
      }
      
      name = dir.dirName();
      dataList = Parser::ParseFiles(list);

   }else {
      name = fileInfo.completeBaseName();
      dataList = Parser::ParseFile(fileName);
   }

   if (dataList.isEmpty()) return false;

   // Obtain a Molecule handle, if the original 'Untitled'
   // molecule has nothing attached we can use that
   QStandardItem* child;
   QStandardItem* root(invisibleRootItem());
   Layer::Molecule* molecule(0);

   for (int i = 0; i < root->rowCount(); ++i) {
       child = root->child(i);
       if (child->text() == DefaultMoleculeName && !child->hasChildren()) {
          Layer::Base* base = QVariantPointer<Layer::Base>::toPointer(child->data());
          molecule = qobject_cast<Layer::Molecule*>(base);
          takeRow(child->row()); 
          break;
       }
   }

   if (!molecule)  {
       forAllMolecules(boost::bind(&Layer::Molecule::setCheckState, _1, Qt::Unchecked));
       molecule = newMolecule();
   }
   molecule->setText(name);
   molecule->appendData(dataList);
      
   Command::AddMolecule* cmd = new Command::AddMolecule(molecule, root);

   postCommand(cmd);
   changeActiveViewerMode(Viewer::Manipulate);
   sceneRadiusChanged(sceneRadius());

   return true;
}


void ViewerModel::openCalculationResults(JobInfo* jobInfo)
{

qDebug() << "opening calculation results";
   QStringList files(jobInfo->outputFiles());
   QLOG_INFO() << "Opening files:";
   for (int i = 0; i < files.size(); ++i) {
       QLOG_INFO() << "  " << files[i];
   }

   DataList dataList(Parser::ParseFiles(files));
   if (dataList.isEmpty()) return;

   Layer::Molecule* molecule(newMolecule());

   molecule->appendData(dataList);
   molecule->setIcon(QIcon(":/resources/icons/Favourites.png"));
   molecule->setText(jobInfo->get(JobInfo::BaseName));

   bool visibleOnly(false);
   Layer::Molecule* defaultMolecule(0);
   MoleculeList molecules(moleculeList(visibleOnly));
   MoleculeList::iterator iter;

   for (iter = molecules.begin(); iter != molecules.end(); ++iter) {
       if ( (*iter)->jobInfoMatch(jobInfo)) {
          molecule->setCheckState((*iter)->checkState());
          int row((*iter)->row());
          // !!! What happens to the molecule we are replacing?
          takeRow(row);
          insertRow(row, molecule);
qDebug() << "found existing molecule";
          return;
          break;
       }else if ((*iter)->text() == DefaultMoleculeName && !(*iter)->hasChildren()) {
          defaultMolecule = *iter;
       }
   }

   // if we get here the user must have removed the Layer::Molecule used to
   // submit the calculation so we just append the results or add it to the
   // Untitled molecule.
   if (defaultMolecule) {
qDebug() << "using default molecule";
      if (defaultMolecule->checkState() == Qt::Checked) {
         molecule->setCheckState(Qt::Checked);
         changeActiveViewerMode(Viewer::Manipulate);
         sceneRadiusChanged(sceneRadius());
         
      }else {
         molecule->setCheckState(Qt::Unchecked);
      }

      int row(defaultMolecule->row());
      takeRow(row);
      insertRow(row, molecule);
      updateVisibleObjects();
   }else {
qDebug() << "Appending new molecule";
      molecule->setCheckState(Qt::Unchecked);
      appendRow(molecule);
   }

}


void ViewerModel::setPartialChargeType(QString const& type)
{
   if (type == "Gasteiger") {
      forAllMolecules(boost::bind(&Layer::Molecule::setGasteigerCharges, _1));
   }else if (type == "Sanderson") {
      forAllMolecules(boost::bind(&Layer::Molecule::setSandersonCharges, _1));
   }else if (type == "Mulliken") {
      forAllMolecules(boost::bind(&Layer::Molecule::setMullikenCharges, _1));
   }
}


void ViewerModel::saveAs()
{
   Layer::Molecule* molecule(activeMolecule());
   if (molecule) molecule->save(true);
}


bool ViewerModel::saveAllAndClose()
{
   bool visibleOnly(false);
   MoleculeList molecules(moleculeList(visibleOnly));
   MoleculeList::iterator iter;
   for (iter = molecules.begin(); iter != molecules.end(); ++iter) {
       qDebug() << "saveAllandClose" << (*iter)->text();
       if ((*iter)->save() == false) return false;
   }
   return true;
}


bool ViewerModel::saveRequired()
{
   bool save(false);
   bool visibleOnly(false);
   MoleculeList molecules(moleculeList(visibleOnly));
   MoleculeList::iterator iter;
   for (iter = molecules.begin(); iter != molecules.end(); ++iter) {
       save = save || (*iter)->isModified();
   }
   return save;
}


void ViewerModel::newMoleculeMenu()
{
   forAllMolecules(boost::bind(&Layer::Molecule::setCheckState, _1, Qt::Unchecked));
   Command::AddMolecule* cmd(new Command::AddMolecule(newMolecule(), invisibleRootItem()));
   changeActiveViewerMode(Viewer::BuildAtom);
   postCommand(cmd);
}


Layer::Molecule* ViewerModel::newMolecule()
{
   Layer::Molecule* molecule = new Layer::Molecule(m_parent);

   connect(molecule, SIGNAL(updated()), 
      this, SLOT(updateVisibleObjects()));
   connect(molecule, SIGNAL(softUpdate()), 
     this, SIGNAL(updated()));
   connect(molecule, SIGNAL(postMessage(QString const&)), 
      this, SIGNAL(displayMessage(QString const&)));
   connect(molecule, SIGNAL(postCommand(QUndoCommand*)), 
      this, SIGNAL(postCommand(QUndoCommand*)));
   connect(molecule, SIGNAL(pushAnimators(AnimatorList const&)), 
      this, SIGNAL(pushAnimators(AnimatorList const&)));
   connect(molecule, SIGNAL(popAnimators(AnimatorList const&)), 
      this, SIGNAL(popAnimators(AnimatorList const&)));
   connect(molecule, SIGNAL(removeMolecule(Layer::Molecule*)), 
      this, SLOT(removeMolecule(Layer::Molecule*)));
   connect(molecule, SIGNAL(select(QModelIndex const&, QItemSelectionModel::SelectionFlags)), 
      this, SIGNAL(select(QModelIndex const&, QItemSelectionModel::SelectionFlags)));

   return molecule;
}


void ViewerModel::removeMolecule(Layer::Molecule* molecule)
{
   QLOG_INFO() << "Removing molecule" << molecule->text();
   postCommand(new Command::RemoveMolecule(molecule, invisibleRootItem()));
}


void ViewerModel::toggleAxes()
{
   if (m_axes.checkState() == Qt::Checked) {
      m_axes.setCheckState(Qt::Unchecked);
      axesOn(false);
   }else {
      m_axes.setCheckState(Qt::Checked);
      axesOn(true);
   }
}


void ViewerModel::updateVisibleObjects()
{
   if (!m_updateEnabled) return;
   QLOG_DEBUG() << "Updating visible objects";
   // We don't want nested objects as Fragments should appear as one object in
   // the Viewer.  This means the Fragment is respnsible for drawing its children
   m_visibleObjects = findLayers<Layer::GLObject>(Layer::Children | Layer::Visible | Layer::Nested);

   // Make sure the selection only contains visible objects;
/*
   GLObjectList::iterator object;
   for (object = m_selectedObjects.begin(); object != m_selectedObjects.end(); ++object) {
       if (! m_visibleObjects.contains(*object)) {
          m_selectedObjects.removeAll(*object);
          if (*object) (*object)->deselect();
       }
   }
*/


   GLObjectList::iterator object(m_selectedObjects.begin());
   while (object != m_selectedObjects.end()) {
       if ( m_visibleObjects.contains(*object)) {
          ++object;
       } else {
          (*object)->deselect();
          object = m_selectedObjects.erase(object);
       }
   }
      

   // Sort our objects based on opacity, high to low
   qSort(m_visibleObjects.begin(), m_visibleObjects.end(), Layer::GLObject::AlphaSort);
   updated();
}


// --------------- Selection Routines ---------------

// The is the main selection routine which should only receive selection
// signals from m_viewerSelectionModel.
void ViewerModel::selectionChanged(QItemSelection const& selected, 
   QItemSelection const& deselected)
{
   QModelIndexList list;
   QModelIndexList::iterator iter; 
   Layer::Base* base;
   Layer::Mode* mode;
   Layer::Conformer* conformer;
   Layer::ConformerList* conformerList1(0);
   Layer::ConformerList* conformerList2(0);
   Layer::GLObject* glObject;
   bool setDefaultConformer(false);

   list = deselected.indexes();
   for (iter = list.begin(); iter != list.end(); ++iter) {
       base = QVariantPointer<Layer::Base>::toPointer((*iter).data(Qt::UserRole+1));
       if ( (glObject = qobject_cast<Layer::GLObject*>(base)) ) {
          glObject->deselect();
          m_selectedObjects.removeAll(glObject);

       }else if ( (mode = qobject_cast<Layer::Mode*>(base)) ) {
          QStandardItem* parent(mode->QStandardItem::parent());
          Layer::Frequencies* frequencies;
          base = QVariantPointer<Layer::Base>::toPointer(parent->data());
          if ( (frequencies = qobject_cast<Layer::Frequencies*>(base)) ) {
             frequencies->clearActiveMode();
          }
       }else if ( (conformer = qobject_cast<Layer::Conformer*>(base)) ) {
          QStandardItem* parent(conformer->QStandardItem::parent());
          base = QVariantPointer<Layer::Base>::toPointer(parent->data());
          // grab a handle on the conformer list here, but we only need to
          // reset the geometry if we don't select another conformer in the
          // next section
          if ( (conformerList1 = qobject_cast<Layer::ConformerList*>(base)) ) {
             setDefaultConformer = true;
          }
       }

   }

   list = selected.indexes();
   for (iter = list.begin(); iter != list.end(); ++iter) {
       base = QVariantPointer<Layer::Base>::toPointer((*iter).data(Qt::UserRole+1));
       if ( (glObject = qobject_cast<Layer::GLObject*>(base)) ) {
          glObject->select();
          m_selectedObjects.append(glObject);
       }else if ( (mode = qobject_cast<Layer::Mode*>(base)) ) {
          QStandardItem* parent(mode->QStandardItem::parent());
          Layer::Frequencies* frequencies;
          base = QVariantPointer<Layer::Base>::toPointer(parent->data());
          if ( (frequencies = qobject_cast<Layer::Frequencies*>(base)) ) {
             frequencies->setActiveMode(*mode);
             Layer::Atom::setDisplayVibrationVector(true);
          }
       }else if ( (conformer = qobject_cast<Layer::Conformer*>(base)) ) {
          QStandardItem* parent(conformer->QStandardItem::parent());
          base = QVariantPointer<Layer::Base>::toPointer(parent->data());
          if ( (conformerList2 = qobject_cast<Layer::ConformerList*>(base)) ) {
             conformerList2->setActiveConformer(*conformer);
          }
          if (conformerList2 == conformerList1) setDefaultConformer = false;
       }
   }

   if (setDefaultConformer) conformerList1->setDefaultConformer();
   if (m_updateEnabled) updated();
}


// The following selection functions interact directly with the selection model.
void ViewerModel::invertSelection()
{
   QItemSelection select;
   QItemSelection deselect;
   
   GLObjectList::iterator iter;
   for (iter = m_visibleObjects.begin(); iter != m_visibleObjects.end(); ++iter) {
       if ((*iter)->isSelected()) {
          deselect.select((*iter)->index(), (*iter)->index());
       }else {
          select.select((*iter)->index(), (*iter)->index());
       }
   }

   selectionChanged(deselect, QItemSelectionModel::Deselect);
   selectionChanged(select, QItemSelectionModel::Select);
}


void ViewerModel::selectAll()
{
   QItemSelection select;
   GLObjectList::iterator iter;
   for (iter = m_visibleObjects.begin(); iter != m_visibleObjects.end(); ++iter) {
       if ( ! (*iter)->isSelected()) {
          select.append(QItemSelectionRange((*iter)->index()));
       }
   }

   selectionChanged(select, QItemSelectionModel::Select);
}


void ViewerModel::selectNone()
{
   enableUpdate(false);
   updateVisibleObjects();
   enableUpdate(true);

   QItemSelection all;
   GLObjectList::iterator iter;
   for (iter = m_visibleObjects.begin(); iter != m_visibleObjects.end(); ++iter) {
       all.append(QItemSelectionRange((*iter)->index()));
   }
   selectionChanged(all, QItemSelectionModel::Deselect);
}


void ViewerModel::setConstraint()
{
   int n(m_selectedObjects.size());
   if (n < 1 || n > 4) {
      displayMessage("Invalid number of atoms for constraint");
      return;
   }

   // Check all the selected atoms belong to the same Molecule.
   unsigned int findFlags(Layer::Parents | Layer::Visible);
   MoleculeList parents(m_selectedObjects[0]->findLayers<Layer::Molecule>(findFlags));

   for (int i = 1; i < n; ++i) {
       if (m_selectedObjects[i]->findLayers<Layer::Molecule>(findFlags) != parents) {
          QString msg("Cannot enforce constraints between atoms in different molecules");
          QMsgBox::warning(m_parent, "IQmol", msg);
          return;
       }
   }

   AtomList atoms;
   Layer::Bond* bond;

   if ( (n == 1) && (bond = qobject_cast<Layer::Bond*>(m_selectedObjects[0])) ) {
      Layer::Atom* A(bond->beginAtom()); 
      Layer::Atom* B(bond->endAtom()); 
      if (A && B) atoms << A << B;
   }else {
      Layer::Atom* A;
      for (int i = 0; i < n; ++i) {
          if ((A = qobject_cast<Layer::Atom*>(m_selectedObjects[i]))) atoms << A;
      } 
      if (atoms.size() != n) atoms.clear();
   }

   if (atoms.isEmpty()) {
      displayMessage("Unable to set constraint");
      return;
   }

   Layer::Constraint* constraint = new Layer::Constraint(atoms);
   constraint->configure();
   if (constraint->accepted()) {
      selectNone();
      parents.first()->appendConstraint(constraint);
   }else {
      delete constraint;
   }
}


template <class T> 
QList<T*> ViewerModel::findLayers(unsigned int findFlags)
{
   QStandardItem* root(invisibleRootItem());
   QStandardItem* child;
   Layer::Base* base;
   QList<T*> list;

   findFlags = findFlags | Layer::IncludeSelf;

   for (int i = 0; i < root->rowCount(); ++i) {
       child = root->child(i);
       base = QVariantPointer<Layer::Base>::toPointer(child->data());
       if (base) list += base->findLayers<T>(findFlags);
   }
   return list;
}


MoleculeList ViewerModel::moleculeList(bool visibleOnly)
{
   unsigned int findFlags(Layer::Children);
   if (visibleOnly) findFlags = (findFlags | Layer::Visible);
   return findLayers<Layer::Molecule>(findFlags);
}


// This should probably be made smarter, for the time being we simply return
// the last visible molecule.
Layer::Molecule* ViewerModel::activeMolecule()
{
   Layer::Molecule* mol(0);
   if (!moleculeList().isEmpty()) mol = moleculeList().last();
   return  mol;
}


void ViewerModel::addHydrogens()
{
   forAllMolecules(boost::bind(&Layer::Molecule::addHydrogens, _1));
}


void ViewerModel::minimizeEnergy()
{
   forAllMolecules(boost::bind(&Layer::Molecule::minimizeEnergy, _1, m_forceField));
}


void ViewerModel::translateToCenter()
{
   forAllMolecules(boost::bind(&Layer::Molecule::translateToCenter, _1, m_selectedObjects));
}


void ViewerModel::symmetrize(double const tolerance)
{
   forAllMolecules(boost::bind(&Layer::Molecule::symmetrize, _1, tolerance, true));
}


void ViewerModel::toggleAutoDetectSymmetry()
{
   Layer::Molecule::toggleAutoDetectSymmetry();
}


void ViewerModel::determineSymmetry()
{
   forAllMolecules(boost::bind(&Layer::Molecule::autoDetectSymmetry, _1));
}


void ViewerModel::deleteSelection()
{
   enableUpdate(false);
   forAllMolecules(boost::bind(&Layer::Molecule::deleteSelection, _1));
   enableUpdate(true);
   updateVisibleObjects();
}


void ViewerModel::cutSelection()
{
   copySelectionToClipboard();
   deleteSelection();
}


void ViewerModel::copySelectionToClipboard()
{
   MoleculeList molecules(moleculeList());
   MoleculeList::iterator mol;
    
   bool selectedOnly(true);
   QString coordinates;
   for (mol = molecules.begin(); mol != molecules.end(); ++mol) {
       coordinates += (*mol)->coordinatesAsString(selectedOnly);
   }

   qDebug() << "Coordinates copied to ClipBoard:";
   qDebug() << coordinates;
   QApplication::clipboard()->setText(coordinates);
}


// Note that we can only paste a string containing a list of coordinates,
// nothing else.  We also offset the coordinates so that if a copy and paste is
// done, they pasted primitives do not obscure the existing ones.

// There appears to be an OS X specific bug that prevents this from working properly.
// The clipboard text gets mangled if the clipboard contents originates from another
// application.
//http://www.qtforum.org/article/28645/qclipboard-problem-mac-osx-tiger.html#post97862
void ViewerModel::pasteSelectionFromClipboard()
{
   QString xyz(QApplication::clipboard()->text(QClipboard::Clipboard));
qDebug() << "=====================================";
qDebug() << "Pasting" << xyz;
qDebug() << "=====================================";
   QStringList lines(xyz.split(QRegExp("\\n"), QString::SkipEmptyParts));
   xyz = QString::number(lines.size());
   xyz += "\n\n";

   double x, y, z;
   double offset(0.5); 
   bool xOK, yOK, zOK;

   QStringList tokens;
   QStringList::iterator line;
   for (line = lines.begin(); line != lines.end(); ++line) {
       tokens = (*line).split(QRegExp("\\s+"), QString::SkipEmptyParts);
       if (tokens.size() < 4) return;  // invalid format
       x = tokens[1].toDouble(&xOK);
       y = tokens[2].toDouble(&yOK);
       z = tokens[3].toDouble(&zOK);

       if (xOK && yOK && zOK) {
          xyz += QString("%1").arg(tokens[0], 3);
          xyz += QString("%1").arg(x-offset, 12, 'f', 6);
          xyz += QString("%1").arg(y-offset, 12, 'f', 6);
          xyz += QString("%1").arg(z-offset, 12, 'f', 6) + "\n";
       }else { 
          return;
       }
   }

   xyz.chop(1);

   Parser::OpenBabel parser;
   QTextStream stream(&xyz);
   DataList data(parser.parse(stream));
   Layer::Atoms* atoms(0);
   Layer::Bonds* bonds(0);
   PrimitiveList primitives;

   DataList::iterator iter;
   for (iter = data.begin(); iter != data.end(); ++iter) {
       if ((atoms = qobject_cast<Layer::Atoms*>(*iter))) {
          AtomList atomList(atoms->findLayers<Layer::Atom>(Layer::Children));
          AtomList::iterator atom;
          for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
              atoms->removeLayer(*atom);
              primitives.append(*atom);
          }

       }else if ((bonds = qobject_cast<Layer::Bonds*>(*iter))) {
          BondList bondList(bonds->findLayers<Layer::Bond>(Layer::Children));
          BondList::iterator bond;
          for (bond = bondList.begin(); bond != bondList.end(); ++bond) {
              bonds->removeLayer(*bond);
              primitives.append(*bond);
          }

       }
   }

   enableUpdate(false);
   selectAll(); 
   activeMolecule()->appendPrimitives(primitives);
   enableUpdate(true);
   invertSelection(); 
   updateVisibleObjects();
}



void ViewerModel::groupSelection()
{
   forAllMolecules(boost::bind(&Layer::Molecule::groupSelection, _1));
}


void ViewerModel::ungroupSelection()
{
   forAllMolecules(boost::bind(&Layer::Molecule::ungroupSelection, _1));
}


void ViewerModel::saveAll()
{
   forAllMolecules(boost::bind(&Layer::Molecule::save, _1, false));
}


void ViewerModel::forAllMolecules(boost::function<void(Layer::Molecule&)> function)
{
   MoleculeList molecules(moleculeList());
   MoleculeList::iterator iter;
   for (iter = molecules.begin(); iter != molecules.end(); ++iter) {
       function(*(*iter));
   }
}


void ViewerModel::adjustSymmetryTolerance() 
{
   SymmetryToleranceDialog dialog(m_parent, m_symmetryTolerance);
   connect(&dialog, SIGNAL(symmetrizeRequest(double const)),
      this, SLOT(symmetrize(double const)));
   dialog.exec();
   if (dialog.result() == QDialog::Accepted) {
      m_symmetryTolerance = dialog.value();
   }
}


double ViewerModel::sceneRadius(bool all)
{
   double radius(DefaultSceneRadius);
   MoleculeList molecules(moleculeList(!all));
   MoleculeList::iterator iter;
   for (iter = molecules.begin(); iter != molecules.end(); ++iter) {
       radius = std::max(radius, (*iter)->radius());
   }
   return radius;
}


void ViewerModel::checkItemChanged(QStandardItem* item)
{
   if (item->isCheckable()) {
      // disconnect to avoid recursion 
      disconnect(this, SIGNAL(itemChanged(QStandardItem*)), 
         this, SLOT(checkItemChanged(QStandardItem*)));

      if (item->checkState() == Qt::Checked) {
         item->setIcon(QIcon()); 
      }else {
         // close all the child configurators
         Layer::Base* base;
         if ((base = dynamic_cast<Layer::Base*>(item))) {
            QList<Layer::Base*> children(base->findLayers<Layer::Base>(Layer::Children));
            QList<Layer::Base*>::iterator child;
            for (child = children.begin(); child != children.end(); ++child) {
                (*child)->closeConfigurator();
            }
         }     
      }

      connect(this, SIGNAL(itemChanged(QStandardItem*)), 
         this, SLOT(checkItemChanged(QStandardItem*)));
      updateVisibleObjects();
      displayMessage("");
   }
}


void ViewerModel::itemDoubleClicked(QModelIndex const& index)
{
   QStandardItem* item = itemFromIndex(index);
   Layer::Base* layer;
   if ((layer = dynamic_cast<Layer::Base*>(item))) {
      layer->configure();
   }
}


void ViewerModel::itemExpanded(QModelIndex const& index)
{
   // This is a bit of an overkill, we reaaly only want to remove the star from
   // molecules with new results.
   QStandardItem* item = itemFromIndex(index);
   item->setIcon(QIcon()); 
}


void ViewerModel::displayGlobals()
{
   m_background.draw();
   m_axes.draw();
   m_mesh.draw();
}


} // end namespace IQmol
