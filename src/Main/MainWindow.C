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

#include "MainWindow.h"

#include "ServerListDialog.h"  //deprecate
#include "ProcessMonitor.h"  // deprecate
#include "ServerRegistry.h"   // deprecate
#include "ServerConfigurationListDialog.h"
#include "JobMonitor.h"
#include "ServerRegistry2.h" 

#include "QMsgBox.h"
#include "Animator.h"
#include "Preferences.h"
#include "JobInfo.h"
#include "ShaderDialog.h"
#include "Network.h"
#include "QUI/InputDialog.h"
#include <QResizeEvent>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QFileDialog>
#include <QtDebug>
#include "Bank.h"
#include "Geometry.h"
#include "AtomicProperty.h"
#include "Atom.h"
#include "QGLViewer/vec.h"
#include <fstream>


namespace IQmol {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
   m_toolBar(this),
   m_aboutDialog(this),
   m_helpBrowser(this),
   m_viewerModel(this),
   m_viewerView(this),
   m_undoStack(this),
   m_undoStackView(&m_undoStack, this),
   m_viewer(m_viewerModel, this),
   m_viewerSelectionModel(&m_viewerModel, this),
   m_logMessageDialog(0),
   m_preferencesBrowser(this),
   m_quiInputDialog(0),
   m_shaderDialog(0)
{
   setStatusBar(0);
   setWindowTitle("IQmol");
   setWindowModified(false);
   setAcceptDrops(false);
   setWindowIcon(QIcon(":/resources/icons/iqmol.png"));

   createMenus();
   createLayout();
   createConnections(); 

   m_undoStack.setUndoLimit(Preferences::UndoLimit());
   m_undoStackView.setEmptyLabel("History:");
   m_viewerView.setModel(&m_viewerModel);
   m_viewerView.setSelectionModel(&m_viewerSelectionModel);
   m_viewer.setActiveViewerMode(Viewer::BuildAtom);

   m_viewer.setDefaultSceneRadius();
   m_viewer.resetView();
}

MainWindow::~MainWindow()
{
   delete m_quiInputDialog;
   delete m_shaderDialog;
}

void MainWindow::createLayout()
{
   QVBoxLayout* mainLayout = new QVBoxLayout;
   mainLayout->setContentsMargins(0,0,0,0);
   QWidget* center = new QWidget(this);
   center->setLayout(mainLayout); 
   setCentralWidget(center);
   resize(Preferences::MainWindowSize());

   mainLayout->addWidget(&m_toolBar);
   m_helpBrowser.setWindowFlags(Qt::Tool);

   // sideSplitter (ha ha) is a data member as we need to control its visibility
   m_sideSplitter = new QSplitter(Qt::Vertical, this);
   m_sideSplitter->addWidget(&m_viewerView);

/*
   QWidget* progress = new QWidget(this);
   progress->setLayoutDirection(
   progress->addWidget();
   progress->addWidget(&m_status);
   progress->addWidget(&m_progressBar);
*/
   
   m_sideSplitter->addWidget(&m_undoStackView);
   m_sideSplitter->setCollapsible(0, true);
   m_sideSplitter->setCollapsible(1, true);

   QList<int> sizes;
   sizes << Preferences::MainWindowSize().height()-220 << 220;
   m_sideSplitter->setSizes(sizes);

   // Main splitter
   QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
   splitter->addWidget(m_sideSplitter);
   splitter->addWidget(&m_viewer);
   splitter->setCollapsible(0, true);
   splitter->setCollapsible(1, false);

   sizes.clear();
   if (Preferences::ShowModelView()) {
      sizes << 220 <<  Preferences::MainWindowSize().width()-200;
   }else {
      sizes << 0;
   }
   splitter->setSizes(sizes);

   mainLayout->addWidget(splitter);
   connect(splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(splitterMoved(int, int)));
}


void MainWindow::createConnections()
{
   connect(&m_toolBar, SIGNAL(newMolecule()),     &m_viewerModel, SLOT(newMoleculeMenu()));
   connect(&m_toolBar, SIGNAL(open()),            this,           SLOT(openFile()));
   connect(&m_toolBar, SIGNAL(save()),            &m_viewerModel, SLOT(saveAll()));
   connect(&m_toolBar, SIGNAL(addHydrogens()),    &m_viewerModel, SLOT(addHydrogens()));
   connect(&m_toolBar, SIGNAL(minimizeEnergy()),  &m_viewerModel, SLOT(minimizeEnergy()));
   connect(&m_toolBar, SIGNAL(deleteSelection()), &m_viewerModel, SLOT(deleteSelection()));
   connect(&m_toolBar, SIGNAL(takeSnapshot()),    &m_viewer,      SLOT(saveSnapshot()));
   connect(&m_toolBar, SIGNAL(record(bool)),      this,           SLOT(setRecord(bool)));
   connect(&m_toolBar, SIGNAL(fullScreen()),      this,           SLOT(fullScreen()));
   connect(&m_toolBar, SIGNAL(showHelp()),        this,           SLOT(showHelp()));

   connect(&m_toolBar, SIGNAL(viewerModeChanged(Viewer::Mode const)), 
      &m_viewer, SLOT(setActiveViewerMode(Viewer::Mode const)));
   connect(&m_toolBar, SIGNAL(buildElementSelected(unsigned int)), 
      &m_viewer, SLOT(setDefaultBuildElement(unsigned int)));
   connect(&m_toolBar, SIGNAL(buildFragmentSelected(QString const&, Viewer::Mode const)), 
      &m_viewer, SLOT(setDefaultBuildFragment(QString const&, Viewer::Mode const)));
      

   connect(this, SIGNAL(recordingActive(bool)), 
      &m_toolBar, SLOT(setRecordAnimationButtonChecked(bool)));
         
   connect(this, SIGNAL(recordingActive(bool)), 
      &m_viewer, SLOT(setRecord(bool)));

   connect(&m_viewer, SIGNAL(recordingCanceled()), 
      this, SLOT(recordingCanceled()));
 

   // ViewerModel 
   connect(&m_viewerModel, SIGNAL(updated()), 
      &m_viewer, SLOT(updateGL()));

   connect(&m_viewerView, SIGNAL(doubleClicked(QModelIndex const&)),
      &m_viewerModel, SLOT(itemDoubleClicked(QModelIndex const&))); 

   connect(&m_viewerView, SIGNAL(expanded(QModelIndex const&)),
      &m_viewerModel, SLOT(itemExpanded(QModelIndex const&))); 

   connect(&m_viewerModel, SIGNAL(sceneRadiusChanged(double const)), 
      &m_viewer, SLOT(setSceneRadius(double const)));

   connect(&m_viewerModel, SIGNAL(changeActiveViewerMode(Viewer::Mode const)), 
      &m_viewer, SLOT(setActiveViewerMode(Viewer::Mode const)));

   connect(&m_viewerModel, SIGNAL(displayMessage(QString const&)),
       &m_viewer, SLOT(displayMessage(QString const&)));

   connect(&m_viewerModel, SIGNAL(postCommand(QUndoCommand*)),
       this, SLOT(addCommand(QUndoCommand*)));

   connect(&m_viewerModel, SIGNAL(foregroundColorChanged(QColor const&)),
       &m_viewer, SLOT(setForegroundColor(QColor const&)));

   connect(&m_viewerModel, SIGNAL(pushAnimators(AnimatorList const&)),
      &m_viewer, SLOT(pushAnimators(AnimatorList const&)));

   connect(&m_viewerModel, SIGNAL(popAnimators(AnimatorList const&)),
      &m_viewer, SLOT(popAnimators(AnimatorList const&)));

   connect(&m_viewerModel, SIGNAL(fileOpened(QString const&)),
      this, SLOT(fileOpened(QString const&)));


   connect(&(ProcessMonitor::instance()), SIGNAL(resultsAvailable(JobInfo*)),
       &m_viewerModel, SLOT(open(JobInfo*)));


   // Viewer
   connect(&m_viewer, SIGNAL(openFileFromDrop(QString const&)),
      &m_viewerModel, SLOT(open(QString const&)));

   connect(&m_viewer, SIGNAL(enableUpdate(bool const)), 
      &m_viewerModel, SLOT(enableUpdate(bool const)));

   connect(&m_viewer, SIGNAL(postCommand(QUndoCommand*)),
      this, SLOT(addCommand(QUndoCommand*)));

   connect(&m_viewer, SIGNAL(activeViewerModeChanged(Viewer::Mode const)),
      &m_toolBar, SLOT(setToolBarMode(Viewer::Mode const)));

   connect(&m_viewer, SIGNAL(escapeFullScreen()), 
      this, SLOT(fullScreen()));

   connect(&m_viewer, SIGNAL(animationStep()), 
      &m_viewerModel, SLOT(reperceiveBondsForAnimation()));


   // Selection
   connect(&m_viewerSelectionModel, 
      SIGNAL(selectionChanged(QItemSelection const&, QItemSelection const&)),
      &m_viewerModel, SLOT(selectionChanged(QItemSelection const&, QItemSelection const&)) );

   connect(&m_viewerModel, 
      SIGNAL(selectionChanged(QItemSelection const&, QItemSelectionModel::SelectionFlags)),
      &m_viewerSelectionModel, 
      SLOT(select(QItemSelection const&, QItemSelectionModel::SelectionFlags)) );

   connect(&m_viewerModel, SIGNAL(clearSelection()),
      &m_viewerSelectionModel, SLOT(clearSelection()));

   connect(&m_viewer, SIGNAL(clearSelection()),
      &m_viewerSelectionModel, SLOT(clearSelection()));

   connect(&m_viewer, 
      SIGNAL(select(QModelIndex const&, QItemSelectionModel::SelectionFlags)),
      &m_viewerSelectionModel, 
      SLOT(select(QModelIndex const&, QItemSelectionModel::SelectionFlags)));

   connect(&m_viewerModel, 
      SIGNAL(select(QModelIndex const&, QItemSelectionModel::SelectionFlags)),
      &m_viewerSelectionModel, 
      SLOT(select(QModelIndex const&, QItemSelectionModel::SelectionFlags)));
}




/******************
 *
 *  Event Handlers
 *
 ******************/

void MainWindow::resizeEvent(QResizeEvent* event)
{
   Preferences::MainWindowSize(event->size());
}


void MainWindow::closeEvent(QCloseEvent* event)
{
   if (!m_viewerModel.saveRequired()) {
      event->accept();
      return;
   }

   QPixmap pixmap;
   pixmap.load(":/imageQuestion");
   QMessageBox messageBox(QMessageBox::NoIcon, "IQmol", "Save Changes?");
   QPushButton* saveButton    = messageBox.addButton("Save", QMessageBox::AcceptRole);
   QPushButton* discardButton = messageBox.addButton("Discard", QMessageBox::RejectRole);
   QPushButton* cancelButton  = messageBox.addButton("Cancel", QMessageBox::RejectRole);
   messageBox.setIconPixmap(pixmap);
   messageBox.exec();

   if (messageBox.clickedButton() == saveButton) {
      if (m_viewerModel.saveAllAndClose()) {
         event->accept();
      }else {
         event->ignore();
      }
   }else if (messageBox.clickedButton() == discardButton) {
     event->accept();
   }else if (messageBox.clickedButton() == cancelButton) {
     event->ignore();
   }


}


void MainWindow::splitterMoved(int pos, int)
{
   Preferences::ShowModelView(pos != 0);
}




/******************
 *
 *  Initialization 
 *
 ******************/

void MainWindow::createMenus()
{
   QAction* action;
   QString  name;
   QMenu*   menu;
   QMenu*   subMenu;

   // ----- File Menu -----
   menu = menuBar()->addMenu("File");

      name = "About";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showAbout()));

      name = "New Molecule";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(newMoleculeMenu()));
      action->setShortcut(QKeySequence::New);

      name = "New Viewer";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(newViewer()));

      name = "Open";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(openFile()));
      action->setShortcut(QKeySequence::Open);

      name = "Open Dir";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(openDir()));
      action->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_O);

      name = "Open Recent";
      m_recentFilesMenu = menu->addMenu(name);
      updateRecentFilesMenu();

      menu->addSeparator();

      name = "Close Viewer";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(close()));
      action->setShortcut(QKeySequence::Close);

      name = "Save";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(saveAll()));
      action->setShortcut(QKeySequence::Save);

      name = "Save As";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(saveAs()));
      action->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_S);

      menu->addSeparator();

      name = "Save Picture";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewer, SLOT(saveSnapshot()));
      action->setShortcut(Qt::CTRL + Qt::Key_P);

      name = "Record Animation";
      action = menu->addAction(name);
      action->setCheckable(true);
      action->setChecked(false);
      connect(action, SIGNAL(triggered()), this, SLOT(toggleRecordingActive()));
      action->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_P);
      m_recordAnimationAction = action;

      name = "Show Message Log";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showLogMessages()));
      action->setShortcut(Qt::CTRL + Qt::Key_L);

      name = "Quit";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(quit()));
      action->setShortcut(Qt::CTRL + Qt::Key_Q);


   // ----- Edit Menu -----
   menu = menuBar()->addMenu("Edit");
  
      name = "Undo";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_undoStack, SLOT(undo()));
      action->setShortcut(QKeySequence::Undo);
      connect(&m_undoStack, SIGNAL(canUndoChanged(bool)), action, SLOT(setEnabled(bool)));

      name = "Redo";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_undoStack, SLOT(redo()));
      action->setShortcut(QKeySequence::Redo);
      connect(&m_undoStack, SIGNAL(canRedoChanged(bool)), action, SLOT(setEnabled(bool)));

      menu->addSeparator();

      name = "Cut";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(cutSelection()));
      action->setShortcut(Qt::CTRL + Qt::Key_X);

      name = "Copy";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(copySelectionToClipboard()));
      action->setShortcut(Qt::CTRL + Qt::Key_C);

      name = "Paste";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(pasteSelectionFromClipboard()));
      action->setShortcut(Qt::CTRL + Qt::Key_V);

      menu->addSeparator();

      name = "Select All";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(selectAll()));
      action->setShortcut(Qt::CTRL + Qt::Key_A);

      name = "Select None";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SIGNAL(clearSelection()));
      action->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_A);

      name = "Invert Selection";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(invertSelection()));
      action->setShortcut(Qt::CTRL + Qt::Key_I);

      menu->addSeparator();

      name = "Reindex Atoms";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(reindexAtoms()));


      menu->addSeparator();

      name = "Preferences";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showPreferences()));


   // ----- Display Menu -----
   menu = menuBar()->addMenu("Display");

      name = "Full Screen";
      action = menu->addAction(name);
      action->setCheckable(true);
      action->setChecked(false);
      connect(action, SIGNAL(triggered()), this, SLOT(fullScreen()));
      action->setShortcut(Qt::CTRL + Qt::Key_0);
      m_fullScreenAction = action;

      name = "Reset View";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewer, SLOT(resetView()));
      action->setShortcut(Qt::CTRL + Qt::Key_R);

      name = "Show Axes";
      action = menu->addAction(name);
      action->setCheckable(true);
      action->setChecked(false);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(toggleAxes()));
      connect(&m_viewerModel, SIGNAL(axesOn(bool)), action, SLOT(setChecked(bool)));
      action->setShortcut(Qt::Key_A);


      menu->addSeparator();

      name = "Appearance";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(configureAppearance()));

      menu->addSeparator();

      name = "Atom Labels";
      subMenu = menu->addMenu(name);

         name = "Element";
         action = subMenu->addAction(name);
         connect(action, SIGNAL(triggered()), this, SLOT(setLabel()));
         action->setData(Layer::Atom::Element);
         action->setShortcut(Qt::Key_E);
         action->setCheckable(true);
         m_labelActions << action;

         name = "Index";
         action = subMenu->addAction(name);
         connect(action, SIGNAL(triggered()), this, SLOT(setLabel()));
         action->setData(Layer::Atom::Index);
         action->setShortcut(Qt::Key_I);
         action->setCheckable(true);
         m_labelActions << action;
   
         name = "Mass";
         action = subMenu->addAction(name);
         connect(action, SIGNAL(triggered()), this, SLOT(setLabel()));
         action->setData(Layer::Atom::Mass);
         action->setShortcut(Qt::Key_M);
         action->setCheckable(true);
         m_labelActions << action;

         name = "NMR Shifts";
         action = subMenu->addAction(name);
         connect(action, SIGNAL(triggered()), this, SLOT(setLabel()));
         action->setData(Layer::Atom::NmrShift);
         action->setShortcut(Qt::Key_N);
         action->setCheckable(true);
         m_labelActions << action;

         name = "Partial Charge";
         action = subMenu->addAction(name);
         connect(action, SIGNAL(triggered()), this, SLOT(setLabel()));
         action->setData(Layer::Atom::Charge);
         action->setShortcut(Qt::Key_Q);
         action->setCheckable(true);
         m_labelActions << action;

         name = "Spin Densities";
         action = subMenu->addAction(name);
         connect(action, SIGNAL(triggered()), this, SLOT(setLabel()));
         action->setData(Layer::Atom::Spin);
         action->setShortcut(Qt::Key_S);
         action->setCheckable(true);
         m_labelActions << action;





   // ----- Build Menu -----
   menu = menuBar()->addMenu("Build");

      name = "Fill Valencies With Hydrogens";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(addHydrogens()));
      action->setShortcut(Qt::CTRL + Qt::Key_F);

      name = "Reperceive Bonds";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(reperceiveBonds()));

      menu->addSeparator();

      name = "Set Geometric Constraint";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(setConstraint()));
      action->setShortcut(Qt::CTRL + Qt::Key_K);

      name = "Minimize Structure";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(minimizeEnergy()));
      action->setShortcut(Qt::CTRL + Qt::Key_M);

      name = "Select Force Field";

      QActionGroup* forceFieldGroup = new QActionGroup(this);
      subMenu = menu->addMenu(name);

         QString ff("MMFF94");
         action = subMenu->addAction(ff);
         action->setCheckable(true);
         action->setData(ff);
         forceFieldGroup->addAction(action);
         connect(action, SIGNAL(triggered()), this, SLOT(setForceField()));
         if (Preferences::DefaultForceField() == ff) action->setChecked(true);

         ff = "MMFF94s";
         action = subMenu->addAction(ff);
         action->setCheckable(true);
         action->setData(ff);
         forceFieldGroup->addAction(action);
         connect(action, SIGNAL(triggered()), this, SLOT(setForceField()));
         if (Preferences::DefaultForceField() == ff) action->setChecked(true);

         ff = "UFF";
         action = subMenu->addAction(ff);
         action->setCheckable(true);
         action->setData(ff);
         forceFieldGroup->addAction(action);
         connect(action, SIGNAL(triggered()), this, SLOT(setForceField()));
         if (Preferences::DefaultForceField() == ff) action->setChecked(true);

         ff = "Ghemical";
         action = subMenu->addAction(ff);
         action->setCheckable(true);
         action->setData(ff);
         forceFieldGroup->addAction(action);
         connect(action, SIGNAL(triggered()), this, SLOT(setForceField()));
         if (Preferences::DefaultForceField() == ff) action->setChecked(true);

         ff = "Gaff";
         action = subMenu->addAction(ff);
         action->setCheckable(true);
         action->setData(ff);
         forceFieldGroup->addAction(action);
         connect(action, SIGNAL(triggered()), this, SLOT(setForceField()));
         if (Preferences::DefaultForceField() == ff) action->setChecked(true);

      menu->addSeparator();

      name = "Translate To Center";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(translateToCenter()));
      action->setShortcut(Qt::CTRL + Qt::Key_T );

      name = "Symmetrize Molecule";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(symmetrize()));
      action->setShortcut(Qt::CTRL + Qt::Key_Y );

      name = "Set Symmetry Tolerance";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(adjustSymmetryTolerance()));

      name = "Auto-detect Symmetry";
      action = menu->addAction(name);
      action->setCheckable(true);
      action->setChecked(false);
      connect(action, SIGNAL(triggered()), &m_viewerModel, SLOT(toggleAutoDetectSymmetry()));



   // ----- Calculation Menu -----
   menu = menuBar()->addMenu("Calculation");

      name = "Q-Chem Setup";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showQChemUIold()));
      action->setShortcut(Qt::CTRL + Qt::Key_U );
      m_qchemSetupAction = action;

      name = "Job Monitor";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showProcessMonitor()));
      action->setShortcut(Qt::CTRL + Qt::Key_J );

      menu->addSeparator();

      name = "Test Internet Connection";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(testInternetConnection()));

      name = "Edit Servers";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(editServers()));

      name = "Remove All Processes";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), 
         &(ProcessMonitor::instance()), SLOT(clearProcessList()));

      menu->addSeparator();

      name = "New Q-Chem Setup";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showQChemUI()));
      //action->setShortcut(Qt::CTRL + Qt::Key_U );
      //m_qchemSetupAction = action;


      name = "New Job Monitor";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showJobMonitor()));
      //action->setShortcut(Qt::CTRL + Qt::Key_J );

      name = "Edit New Servers";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(editNewServers()));


   // ----- Help Menu -----
   menu = menuBar()->addMenu("Help");

      name = "Show Help";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(showHelp()));
}


void MainWindow::editServers()
{
   ServerListDialog dialog(this);
   dialog.exec();  
}


void MainWindow::editNewServers()
{
   Process2::ServerConfigurationListDialog dialog(this);
   dialog.exec();  
}


void MainWindow::showLogMessages()
{ 
   if (m_logMessageDialog.isActive()) {
      m_logMessageDialog.close(); 
   }else {
      m_logMessageDialog.display(); 
   }
}


void MainWindow::showJobMonitor() { 
   Process2::JobMonitor::instance().show(); 
   Process2::JobMonitor::instance().raise();
}


void MainWindow::showProcessMonitor() { 
   ProcessMonitor::instance().show(); 
   ProcessMonitor::instance().raise();
}


void MainWindow::testInternetConnection()
{
   if (Network::TestNetworkConnection()) {
      QMsgBox::information(this, "IQmol", "Network access available");
   }
}


void MainWindow::configureAppearance()
{
   if (!m_shaderDialog) {
      m_shaderDialog = new ShaderDialog(this);
      connect(m_shaderDialog, SIGNAL(updated()), &m_viewer, SLOT(updateGL()));
   }
   m_shaderDialog->show();
}


void MainWindow::newViewer()
{
   MainWindow* mw(new MainWindow());
   mw->move(x()+30,y()+30);
   mw->show();
}


void MainWindow::openFile()
{
   QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), 
      Preferences::LastFileAccessed());
   if (!fileName.isEmpty()) open(fileName);
}


void MainWindow::openDir()
{
   QString dirName = QFileDialog::getExistingDirectory(this, tr("Open Job Directory"), 
      Preferences::LastFileAccessed());
   if (!dirName.isEmpty()) open(dirName);
}


void MainWindow::openRecentFile()
{
   QAction* action = qobject_cast<QAction*>(sender());
   if (action) open(action->data().toString());
}


void MainWindow::fileOpened(QString const& filePath)
{
   Preferences::AddRecentFile(filePath);
   Preferences::LastFileAccessed(filePath);
   updateRecentFilesMenu();
}


void MainWindow::reindexAtoms()
{
   bool all(false); // we only want the visible molecules;
   MoleculeList molecules(m_viewerModel.moleculeList(all));

   if (molecules.size() == 0) return;
   if (molecules.size() > 1) {
      QString msg("Only one molecule can be visible when reindexing atoms.");
      QMsgBox::warning(this, "IQmol", msg);
      return;
   }

   QList<QAction*>::iterator iter;
   for (iter = m_labelActions.begin(); iter != m_labelActions.end(); ++iter) {
       (*iter)->setChecked(false); 
   }

   m_viewer.reindexAtoms(molecules.first());
}


void MainWindow::toggleRecordingActive()
{
   setRecord(m_recordAnimationAction->isChecked());
}


void MainWindow::setRecord(bool active)
{
   // Synchronizes the menu and ToolBar
   m_recordAnimationAction->setChecked(active);
   recordingActive(active);
}


void MainWindow::setLabel()
{
   QAction* action(qobject_cast<QAction*>(sender()));

   bool turnOn(action->isChecked());

   if (turnOn) {
      m_viewer.setLabelType(action->data().toInt());
   }else {
      m_viewer.setLabelType(Layer::Atom::None);
   }

   QList<QAction*>::iterator iter;
   for (iter = m_labelActions.begin(); iter != m_labelActions.end(); ++iter) {
       (*iter)->setChecked(false);
   }
   if (turnOn) action->setChecked(true);
}



void MainWindow::setForceField()
{
   QAction* action = qobject_cast<QAction*>(sender());
   if (action) m_viewerModel.setForceField(action->data().toString());
}


void MainWindow::updateRecentFilesMenu()
{     
   if (m_recentFilesMenu) {
      m_recentFilesMenu->clear();
      QStringList files(Preferences::RecentFiles());
      QAction* action;
      QFileInfo info;
      
      for (int i = 0; i < files.count(); ++i) {
          info.setFile(files[i]);
          if (info.exists() && info.isReadable()) {
             action = m_recentFilesMenu->addAction(info.fileName());
             action->setData(files[i]);
             connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
          }else {
             Preferences::RemoveRecentFile(files[i]);
          }
      }
      
      m_recentFilesMenu->addSeparator();
      action = m_recentFilesMenu->addAction("Clear List");
      connect(action, SIGNAL(triggered()), this, SLOT(clearRecentFilesMenu()));
   }
}


void MainWindow::clearRecentFilesMenu()
{
   Preferences::ClearRecentFiles();
   updateRecentFilesMenu();
}


void MainWindow::fullScreen()
{
   QString msg;

   if (m_viewer.isFullScreen()) {
      // From full screen
      m_toolBar.show();
      m_sideSplitter->show();
      m_viewer.setFullScreen(false);
      m_fullScreenAction->setChecked(false);
      msg = "";
   } else {
      // To full screen
      m_toolBar.hide();
      m_sideSplitter->hide();
      m_viewer.setFullScreen(true);
      m_fullScreenAction->setChecked(true);
      msg = "Use <esc> to exit full screen mode";
   }

   m_viewer.QGLViewer::displayMessage(msg, 5000);
}


void MainWindow::showQChemUIold() 
{
   if (!m_quiInputDialog) {
      m_quiInputDialog = new Qui::InputDialog(this);
      if (!m_quiInputDialog->init()) {
         m_qchemSetupAction->setEnabled(false);
         delete m_quiInputDialog;
         m_quiInputDialog = 0;
         return;
      }

      // deprecate
      connect(m_quiInputDialog, SIGNAL(submitJobRequest(IQmol::JobInfo*)),
         &(ProcessMonitor::instance()), SLOT(submitJob(IQmol::JobInfo*)));

      // deprecate
      connect(&(ProcessMonitor::instance()), SIGNAL(jobAccepted()),
         m_quiInputDialog, SLOT(close()));

      // deprecate
      connect(&(ProcessMonitor::instance()), SIGNAL(postStatusMessage(QString const&)),
         m_quiInputDialog, SLOT(showMessage(QString const&)));
   }

   Layer::Molecule* mol(m_viewerModel.activeMolecule());
   if (!mol) return;

   // (Re-)Load the servers here in case the user has made any modifications
   QStringList serverList(ServerRegistry::instance().availableServers());
   
   if (serverList.isEmpty())  serverList.append("(none)");

   m_quiInputDialog->setServerList(serverList);
   m_quiInputDialog->showMessage("");
   m_quiInputDialog->setJobInfo(mol->jobInfo());
   m_quiInputDialog->setWindowModality(Qt::WindowModal);
   m_quiInputDialog->show();

   m_viewer.setActiveViewerMode(Viewer::Manipulate);
}


void MainWindow::showQChemUI() 
{
   if (!m_quiInputDialog) {
      m_quiInputDialog = new Qui::InputDialog(this);
      if (!m_quiInputDialog->init()) {
         m_qchemSetupAction->setEnabled(false);
         delete m_quiInputDialog;
         m_quiInputDialog = 0;
         return;
      }

      connect(m_quiInputDialog, SIGNAL(submitJobRequest(IQmol::Process2::QChemJobInfo&)),
         &(IQmol::Process2::JobMonitor::instance()), 
            SLOT(submitJob(IQmol::Process2::QChemJobInfo&)));
 
      connect(&(Process2::JobMonitor::instance()), SIGNAL(jobAccepted()),
         m_quiInputDialog, SLOT(close()));

      connect(&(Process2::JobMonitor::instance()), SIGNAL(postUpdateMessage(QString const&)),
         m_quiInputDialog, SLOT(showMessage(QString const&)));
   }

   Layer::Molecule* mol(m_viewerModel.activeMolecule());
   if (!mol) return;
   
   Process2::QChemJobInfo jobInfo(mol->qchemJobInfo());
   m_quiInputDialog->setQChemJobInfo(jobInfo);

   // (Re-)Load the servers here in case the user has made any modifications
   QStringList serverList(Process2::ServerRegistry::instance().availableServers());
   if (serverList.isEmpty()) serverList.append("(none)");
   m_quiInputDialog->setServerList(serverList);

   m_quiInputDialog->showMessage("");
   m_quiInputDialog->setWindowModality(Qt::WindowModal);
   m_quiInputDialog->show();

   m_viewer.setActiveViewerMode(Viewer::Manipulate);
}

} // end namespace IQmol
