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

#include "ViewerModel.h"
#include "AtomLayer.h"
#include "ChargeLayer.h"
#include "FrequenciesLayer.h"
#include "ConstraintLayer.h"
#include "GeometryListLayer.h"
#include "GeometryLayer.h"
#include "Preferences.h"
#include "UndoCommands.h"
#include "QMsgBox.h"
#include "Exception.h"
#include "QVariantPointer.h"
#include "CartesianCoordinatesParser.h"
#include "TextStream.h"
#include "ParseJobFiles.h"
#include "OpenBabelParser.h"
#include "YamlNode.h"
#include "SymmetryToleranceDialog.h"
#include "ServerConfiguration.h"
#include "ServerConfigurationDialog.h"
#include <QStringList>
#include <QStandardItem>
#include "QsLog.h"
#include <QUrl>
#include <QDir>
#include <QClipboard>
#include <QMimeData>
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
   m_global.appendRow(&m_clippingPlane);

   connect(&m_background, SIGNAL(updated()), this, SIGNAL(updated()));
   connect(&m_background, SIGNAL(foregroundColorChanged(QColor const&)), 
      this, SIGNAL(foregroundColorChanged(QColor const&))); 
   connect(&m_background, SIGNAL(backgroundColorChanged(QColor const&)), 
      this, SIGNAL(backgroundColorChanged(QColor const&))); 

   connect(&m_axes, SIGNAL(updated()), this, SIGNAL(updated()));
   connect(&m_mesh, SIGNAL(updated()), this, SIGNAL(updated()));
   connect(&m_clippingPlane, SIGNAL(updated()), this, SIGNAL(updated()));

   connect(this, SIGNAL(sceneRadiusChanged(double const)), 
      &m_axes, SLOT(setSceneRadius(double const)));

   connect(this, SIGNAL(sceneRadiusChanged(double const)), 
      &m_mesh, SLOT(setSceneRadius(double const)));

   connect(this, SIGNAL(sceneRadiusChanged(double const)), 
      &m_clippingPlane, SLOT(setSceneRadius(double const)));

   connect(this, SIGNAL(itemChanged(QStandardItem*)), 
      this, SLOT(checkItemChanged(QStandardItem*)));

   Layer::Molecule* mol(newMolecule());
   appendRow(mol);
   changeActiveViewerMode(Viewer::BuildAtom);
   sceneRadiusChanged(DefaultSceneRadius);
}


GLObjectList ViewerModel::getVisibleObjects() 
{ 
   //qDebug() << "Number of visible objects" << m_visibleObjects.size(); 
   return m_visibleObjects; 
}


GLObjectList ViewerModel::getSelectedObjects() 
{ 
   //qDebug() << "Number of selected objects" << m_selectedObjects.size(); 
   return m_selectedObjects; 
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



void ViewerModel::open(QString const& filePath)
{
   QString path(filePath);
   while (path.endsWith("/")) {
       path.chop(1);
   }
#ifdef Q_OS_WIN32
   // Not sure where this is getting added, but it causes problems on Windows, obviously
   while (path.startsWith("/")) {
       path.remove(0,1);
   }
#endif
   ParseJobFiles* parser = new ParseJobFiles(path);
   connect(parser, SIGNAL(finished()), this, SLOT(fileOpenFinished()));
   parser->start();
}


void ViewerModel::open(QString const& filePath, QString const& filter, void* moleculePointer)
{
   QString path(filePath);
   while (path.endsWith("/")) {
       path.chop(1);
   }
#ifdef Q_OS_WIN32
   // Not sure where this is getting added, but it causes problems on Windows, obviously
   while (path.startsWith("/")) {
       path.remove(0,1);
   }
#endif
   ParseJobFiles* parser = new ParseJobFiles(path, filter, moleculePointer);
   connect(parser, SIGNAL(finished()), this, SLOT(fileOpenFinished()));
   parser->start();
}


void ViewerModel::fileOpenFinished()
{
   ParseJobFiles* parser = qobject_cast<ParseJobFiles*>(sender());
   if (!parser) return;

   if (parser->status() == Task::SigTrap) {
      throw SignalException();
   }

   Data::Bank& bank(parser->data());
   QFileInfo info(parser->filePath());
   QStringList errors(parser->errors());

   if (bank.isEmpty()) {
      if (errors.isEmpty()) errors.append("No valid data found in " + info.filePath());
      QMsgBox::warning(m_parent, "IQmol", errors.join("\n"));
      parser->deleteLater();
      return;
   }

   if (!errors.isEmpty()) {
      QMsgBox::warning(m_parent, "IQmol", errors.join("\n"));
   }

   processConfigData(parser);
   processParsedData(parser);
   parser->deleteLater();

/*
qDebug() << "***************************************************";
qDebug() << "Finished opening file/dir";
qDebug() << "   file path" << info.filePath();
qDebug() << "   path     " << info.path();
qDebug() << "   dir name " << info.dir().dirName();
qDebug() << "   base name" << info.completeBaseName();
qDebug() << "   is dir?  " << info.isDir();
qDebug() << "   is file? " << info.isFile();
qDebug() << "***************************************************";
*/
}


void ViewerModel::processConfigData(ParseJobFiles* parser)
{
   Data::Bank& bank(parser->data());
   QList<Data::YamlNode*> yaml(bank.takeData<Data::YamlNode>());
   for (int i = 0; i < yaml.size(); ++i) {
       yaml[i]->dump();
       Process::ServerConfiguration config(*(yaml[i]));
       Process::ServerConfigurationDialog dialog(config); 
       dialog.exec();
   }
}


void ViewerModel::processParsedData(ParseJobFiles* parser)
{
   Data::Bank& bank(parser->data());
   if (bank.isEmpty()) return;

   QString name(parser->name());

   // Determine if we need to replace an existing Molecule in the Model View.
   bool overwrite(parser->flags()  & ParseJobFiles::Overwrite);
   bool makeActive(parser->flags() & ParseJobFiles::MakeActive);
   bool addStar(parser->flags()    & ParseJobFiles::AddStar);
   bool found(false);

   Layer::Molecule* molecule(newMolecule());
   molecule->setCheckState(Qt::Unchecked);
   molecule->setText(name);
   molecule->appendData(bank);
   if (addStar) molecule->setIcon(QIcon(":/resources/icons/Favourites.png"));

   QStandardItem* child;
   QStandardItem* root(invisibleRootItem());

   void* moleculePointer(parser->moleculePointer());

qDebug() << "======= QJI ====== ViewerModel *moleculePointer" << moleculePointer;
qDebug() << "======= QJI ====== ViewerModel *molecule       " << molecule;
   if (overwrite && moleculePointer) {
      for (int row = 0; row < root->rowCount(); ++row) {
          child = root->child(row);
          if (child->text() == name) {
             Layer::Base* base = QVariantPointer<Layer::Base>::toPointer(child->data());
             Layer::Molecule* mol = qobject_cast<Layer::Molecule*>(base);
qDebug() << "======= QJI ====== ViewerModel *mol            " << mol;

             if (mol && mol == moleculePointer) {
                qDebug() << "found existing molecule";
qDebug() << "replacing " << mol << "with" << molecule;
                molecule->setCheckState(mol->checkState());
                // makeActive = makeActive || (mol->checkState() == Qt::Checked);
                // This may result in a memory leak, but we need the Molecule
                // to remain lying around in case of an undo action.
                disconnectMolecule(mol);
                takeRow(row);
                insertRow(row, molecule);
                found = true;
                break;
             }
          }
      }
   }

   // If we didn't find an existing Molecule to overwrite, we check the last
   // Molecule on the list and if that is still 'Untitled' and empty, use it.
   if (!found) {
      child = root->child(root->rowCount()-1);
      if (child->text() == DefaultMoleculeName && !child->hasChildren()) {
         Layer::Base* base = QVariantPointer<Layer::Base>::toPointer(child->data());
         Layer::Molecule* mol = qobject_cast<Layer::Molecule*>(base);
         makeActive = makeActive || (mol->checkState() == Qt::Checked);
         // This may result in a memory leak, but we need the Molecule
         // to remain lying around in case of an undo action.
         disconnectMolecule(mol);
         takeRow(child->row());
      }

      Command::AddMolecule* cmd = new Command::AddMolecule(molecule, root);
      postCommand(cmd);
   }

   if (makeActive) {
qDebug() << "QJI: making molecules inactive";
      forAllMolecules(boost::bind(&Layer::Molecule::setCheckState, _1, Qt::Unchecked));
qDebug() << "QJI: making new molecule active";
      molecule->setCheckState(Qt::Checked);
      sceneRadiusChanged(sceneRadius());
      changeActiveViewerMode(Viewer::Manipulate);
   }

   fileOpened(parser->filePath());
qDebug() << "QJI: finished process parse data";
}


// Note that we can only paste a string containing a list of coordinates,
// nothing else.  We also offset the coordinates so that if a copy and paste is
// done, they pasted primitives do not obscure the existing ones.

// There appears to be an OS X specific bug that prevents this from working properly.
// The clipboard text gets mangled if the clipboard contents originates from another
// application.
// http://www.qtforum.org/article/28645/qclipboard-problem-mac-osx-tiger.html#post97862
void ViewerModel::pasteSelectionFromClipboard()
{
   QString xyz(QApplication::clipboard()->text(QClipboard::Clipboard));
   Parser::CartesianCoordinates parser;
   Parser::TextStream textStream(&xyz);
   Data::Geometry* geom(parser.parse(textStream));

   if (geom && activeMolecule()) {
      selectAll(); 
      // This should be added as an Undo action
      activeMolecule()->appendPrimitives(Layer::PrimitiveList(*geom));
      invertSelection(); 
      updateVisibleObjects();
      delete geom;
   }
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

   qDebug() << coordinates;
   QApplication::clipboard()->setText(coordinates);
}


void ViewerModel::saveAll()
{
   forAllMolecules(boost::bind(&Layer::Molecule::save, _1, false));
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
   connectMolecule(molecule);
   return molecule;
}


void ViewerModel::connectMolecule(Layer::Molecule* molecule)
{
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
}


void ViewerModel::disconnectMolecule(Layer::Molecule* molecule)
{
   disconnect(molecule, SIGNAL(updated()), 
      this, SLOT(updateVisibleObjects()));
   disconnect(molecule, SIGNAL(softUpdate()), 
     this, SIGNAL(updated()));
   disconnect(molecule, SIGNAL(postMessage(QString const&)), 
      this, SIGNAL(displayMessage(QString const&)));
   disconnect(molecule, SIGNAL(postCommand(QUndoCommand*)), 
      this, SIGNAL(postCommand(QUndoCommand*)));
   disconnect(molecule, SIGNAL(pushAnimators(AnimatorList const&)), 
      this, SIGNAL(pushAnimators(AnimatorList const&)));
   disconnect(molecule, SIGNAL(popAnimators(AnimatorList const&)), 
      this, SIGNAL(popAnimators(AnimatorList const&)));
   disconnect(molecule, SIGNAL(removeMolecule(Layer::Molecule*)), 
      this, SLOT(removeMolecule(Layer::Molecule*)));
   disconnect(molecule, SIGNAL(select(QModelIndex const&,QItemSelectionModel::SelectionFlags)), 
      this, SIGNAL(select(QModelIndex const&, QItemSelectionModel::SelectionFlags)));
}


void ViewerModel::removeMolecule(Layer::Molecule* molecule)
{
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
   //QLOG_TRACE() << "Updating visible objects";
   // We don't want nested objects as Fragments should appear as one object in
   // the Viewer.  This means the Fragment is respnsible for drawing its children
   m_visibleObjects = findLayers<Layer::GLObject>(Layer::Children | Layer::Visible | 
      Layer::Nested);

   // Make sure the selection only contains visible objects;
   GLObjectList::iterator object(m_selectedObjects.begin());
   while (object != m_selectedObjects.end()) {
       if ( m_visibleObjects.contains(*object)) {
          ++object;
       } else {
//!!!
          (*object)->deselect();
          object = m_selectedObjects.erase(object);
       }
   }

   // Sort our objects based on opacity, high to low
   qSort(m_visibleObjects.begin(), m_visibleObjects.end(), Layer::GLObject::AlphaSort);
   updated();
}


// --------------- Selection Routines ---------------

// This is the main selection routine which should only receive selection
// signals from m_viewerSelectionModel.
void ViewerModel::selectionChanged(QItemSelection const& selected, 
   QItemSelection const& deselected)
{
   QModelIndexList list;
   QModelIndexList::iterator iter; 
   Layer::Base* base;
   Layer::Mode* mode;
   Layer::Geometry* geometry;
   Layer::GeometryList* geometryList1(0);
   Layer::GeometryList* geometryList2(0);
   Layer::GLObject* glObject;
   bool setDefaultGeometry(false);

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
       }else if ( (geometry = qobject_cast<Layer::Geometry*>(base)) ) {
          QStandardItem* parent(geometry->QStandardItem::parent());
          base = QVariantPointer<Layer::Base>::toPointer(parent->data());
          // grab a handle on the geometry list here, but we only need to
          // reset the geometry if we don't select another geometry in the
          // next section
          if ( (geometryList1 = qobject_cast<Layer::GeometryList*>(base)) ) {
             setDefaultGeometry = true;
          }
       }
   }

   list = selected.indexes();
   for (iter = list.begin(); iter != list.end(); ++iter) {
       base = QVariantPointer<Layer::Base>::toPointer((*iter).data(Qt::UserRole+1));
       if ( (glObject = qobject_cast<Layer::GLObject*>(base)) ) {

          MoleculeList 
             parents(glObject->findLayers<Layer::Molecule>(Layer::Parents | Layer::Visible));
//        if (parents.size() > 0) {
              glObject->select();
              m_selectedObjects.append(glObject);
//          }
       }else if ( (mode = qobject_cast<Layer::Mode*>(base)) ) {
          QStandardItem* parent(mode->QStandardItem::parent());
          Layer::Frequencies* frequencies;
          base = QVariantPointer<Layer::Base>::toPointer(parent->data());
          if ( (frequencies = qobject_cast<Layer::Frequencies*>(base)) ) {
             frequencies->setActiveMode(*mode);
             Layer::Atom::setDisplayVibrationVector(true);
          }
       }else if ( (geometry = qobject_cast<Layer::Geometry*>(base)) ) {
          QStandardItem* parent(geometry->QStandardItem::parent());
          base = QVariantPointer<Layer::Base>::toPointer(parent->data());
          if ( (geometryList2 = qobject_cast<Layer::GeometryList*>(base)) ) {
             geometryList2->setCurrentGeometry(geometry->row());
             // ------- also select the corresponding MOs --------
             // ---------------------------------------------------
          }
          if (geometryList2 == geometryList1) setDefaultGeometry = false;
       }
   }

   if (setDefaultGeometry) geometryList1->resetGeometry();
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
   if (parents.isEmpty()) return;

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

   Layer::Molecule* molecule(parents.first());
   Layer::Constraint* constraint(molecule->findMatchingConstraint(atoms));

   if (constraint) {
      constraint->configure();
      if (constraint->accepted()) {
         selectNone();
         molecule->applyConstraint(constraint);
      }
   }else {
      constraint = new Layer::Constraint(atoms);
      constraint->configure();
      if (constraint->accepted()) {
         if (molecule->canAcceptConstraint(constraint)) {
            selectNone();
            molecule->addConstraint(constraint);
         }else {
            QMsgBox::information(0, "IQmol", 
               "A maximum of two scan coordinates are permitted");
            delete constraint;
         }
         
      }else {
         delete constraint;
      }
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


void ViewerModel::reperceiveBonds()
{
   forAllMolecules(boost::bind(&Layer::Molecule::reperceiveBonds, _1));
}


void ViewerModel::reperceiveBondsForAnimation()
{
   forAllMolecules(boost::bind(&Layer::Molecule::reperceiveBondsForAnimation, _1));
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
   bool updateCoordinates(true);
   forAllMolecules(boost::bind(&Layer::Molecule::symmetrize, _1, tolerance, updateCoordinates));
}


void ViewerModel::toggleAutoDetectSymmetry()
{
   Layer::Molecule::toggleAutoDetectSymmetry();
}


void ViewerModel::saveToCurrentGeometry()
{
   forAllMolecules(boost::bind(&Layer::Molecule::saveToCurrentGeometry, _1));
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


void ViewerModel::groupSelection()
{
   forAllMolecules(boost::bind(&Layer::Molecule::groupSelection, _1));
}


void ViewerModel::ungroupSelection()
{
   forAllMolecules(boost::bind(&Layer::Molecule::ungroupSelection, _1));
}


void ViewerModel::forAllMolecules(boost::function<void(Layer::Molecule&)> function)
{
   bool visibleOnly(true);
   MoleculeList molecules(moleculeList(visibleOnly));
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


void ViewerModel::insertMoleculeById(QString identifier)
{
   Parser::OpenBabel parser;
   QString ext;

   if (identifier.indexOf("SMILES:") == 0) {
      identifier.replace(0,7,"");
      ext = "smi";

   }else if (identifier.indexOf("INCHI:") == 0) {
      identifier.replace(0,6,"");
      ext = "inchi";

   }else if (identifier.indexOf("INCHIKEY:") == 0) {
      identifier.replace(0,9,"");
      ext = "inchikey";

   }else {
      QLOG_WARN() << "Unknown molecule identifier type:" << identifier;
      return;
   }

   qDebug() << "Identifier" << identifier;

   bool ok(parser.parse(identifier, ext));
   QStringList errors(parser.errors());
   Data::Bank& bank(parser.data());

   if (ok && errors.isEmpty() && !bank.isEmpty()) {
      if (activeMolecule()) activeMolecule()->appendData(bank);
   }else {
      QMsgBox::warning(m_parent, "IQmol", "Invalid string");
   }
}


double ViewerModel::sceneRadius(bool visibleOnly)
{
   double radius(DefaultSceneRadius);
   MoleculeList molecules(moleculeList(visibleOnly));
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

      Layer::Base* base;

      if (item->checkState() == Qt::Checked) {
         // This removes the star icon that appears when there are new results
         item->setIcon(QIcon()); 
      }else {
         // close all the child configurators
         if ((base = dynamic_cast<Layer::Base*>(item))) {
            QList<Layer::Base*> children(base->findLayers<Layer::Base>(Layer::Children));
            QList<Layer::Base*>::iterator child;
            for (child = children.begin(); child != children.end(); ++child) {
                if (*child) (*child)->closeConfigurator();
            }
         }     
      }

      if ((base = dynamic_cast<Layer::Base*>(item))) {

         Layer::Surface* layer;
         if ((layer = dynamic_cast<Layer::Surface*>(base))) {
            layer->setCheckStatus(item->checkState());
         }

         Layer::Molecule* molecule;
         if ((molecule = dynamic_cast<Layer::Molecule*>(base))) {
            displayMessage("");
         }

         Layer::Background* background;
         if ((background = dynamic_cast<Layer::Background*>(base))) {
            displayMessage("");
         }
      }

      connect(this, SIGNAL(itemChanged(QStandardItem*)), 
         this, SLOT(checkItemChanged(QStandardItem*)));
      updateVisibleObjects();
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
   // This is a bit of an overkill, we really only want to remove the star from
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
