######################################################################
#
#  This is the IQmol project file.  To use this file type one of the 
#  following commands:
#
#     OS X:     qmake -spec macx-g++ -macx -o Makefile IQmol.pro
#     Linux:    qmake -unix -o Makefile IQmol.pro
#     Windows:  qmake.exe -win32 -o Makefile IQmol.pro
#
#  Note that the FORTRAN object files must be generated manually, for
#  example with the command:
#
#     gfortran -c symmol.f90
#
######################################################################

TEMPLATE     =  app
DEPENDPATH  +=  .
CONFIG      +=  no_keywords qt opengl qchem_ui
RESOURCES   +=  IQmol.qrc
ICON         =  resources/IQmol.icns
QT          +=  sql xml opengl
OBJECTS_DIR  = ../../build
MOC_DIR      = ../../build/moc
UI_DIR       = ../../build/ui
DESTDIR      = ../../build

QMAKE_CXXFLAGS += -O2
#QMAKE_CXXFLAGS += -O0 


OBJECTS += symmol.o

include(QsLog/QsLog.pri)
include(GL2PS/GL2PS.pri)
include(QMsgBox/QMsgBox.pri)

qchem_ui {
   include(QUI/QUI.pri)
}


#################################
# System specific configuration #
#################################

macx {
   INCLUDEPATH += /usr/local/include
   INCLUDEPATH += /usr/local/include/openbabel-2.0/
   INCLUDEPATH += /Library/Frameworks/QtSql.framework/Headers
   INCLUDEPATH += /Library/Frameworks/QGLViewer.framework/Headers
   INCLUDEPATH += $(DEV)/eigen
   INCLUDEPATH += $(DEV)/Boost/include
   INCLUDEPATH += $(DEV)/extlib/include

   LIBS += -framework GLUT -framework QGLViewer 
   LIBS += -L/usr/local/lib -L/usr/local/gfortran/lib -L$(DEV)/extlib/lib
   LIBS += -lopenbabel -lgfortran -lssl -lssh2 -lcrypto -lz

   #CONFIG += debug
   CONFIG += release
   QMAKE_INFO_PLIST = resources/Info.plist
   FORMS += PeriodicTableMac.ui
}

win32 {
   INCLUDEPATH += "C:\Program Files\boost\boost_1_36_0"
   INCLUDEPATH += C:\Users\atg\Development
   INCLUDEPATH += C:\Users\atg\Development\OpenBabel\include\openbabel-2.0
   INCLUDEPATH += C:\Users\atg\Development\MinGW\include\libxml2

   LIBS += -LC:\Users\atg\Development\OpenBabel-shared\bin -lopenbabel
   LIBS += -LC:\Users\atg\Development\QGLViewer\debug -lQGLViewerd2
   LIBS += -L"C:\Program Files\gfortran\bin" -lgfortran-3 -lgcc_s_dw2-1

   SOURCES += QUI/getpids.C
   ICON = resources/IQmol.ico
   CONFIG += debug exceptions rtti
   RC_FILE += resources/windows.rc
   include(OpenBabel/OpenBabel.pro)
   FORMS += PeriodicTable.ui
}

unix:!macx {
   INCLUDEPATH += /home/agilbert/development/libQGLViewer/include
   INCLUDEPATH += /home/agilbert/development/openbabel/include/openbabel-2.0
   LIBS += -L/home/agilbert/development/libQGLViewer/lib -lQGLViewer
   LIBS += -L~/development/libQGLViewer/lib -L/usr/lib -lopenbabel -L../build
   LIBS += -L/usr/local/gfortran/lib -lgfortran
   CONFIG += release
   FORMS += PeriodicTable.ui
}


###############
# Input Files #
###############

HEADERS += \
   IQmolApplication.h  MainWindow.h  ToolBar.h  HelpBrowser.h  PeriodicTable.h \
   IQmol.h  Preferences.h Viewer.h  AboutDialog.h  ViewerModel.h \
   GLObjectLayer.h  BaseLayer.h  MeshLayer.h  AxesLayer.h BackgroundLayer.h \
   MoleculeLayer.h  ManipulatedFrameSetConstraint.h PrimitiveLayer.h  AtomLayer.h \
   BondLayer.h  ChargeLayer.h  UndoCommands.h SymmetryToleranceDialog.h \
   ViewerModelView.h  Cursors.h  ManipulateHandler.h SelectHandler.h \
   ManipulateSelectionHandler.h  BuildHandler.h  Shell.h  Grid.h MOCoefficients.h \
   ProgressDialog.h  SurfaceLayer.h  MarchingCubes.h CubeDataLayer.h \
   QChemParser.h  GlobalLayer.h MeshConfigurator.h  InfoLayer.h \
   InfoConfigurator.h  BaseConfigurator.h BackgroundConfigurator.h \
   MoleculeConfigurator.h  SurfaceConfigurator.h DataLayer.h  BaseParser.h \
   ExternalChargesParser.h  OpenBabelParser.h FrequenciesConfigurator.h \
   FrequenciesLayer.h  Animator.h ConformerListLayer.h \
   FormattedCheckpointParser.h  MolecularOrbitalsLayer.h \
   MolecularOrbitalsConfigurator.h  ConformerListConfigurator.h \
   CubeDataConfigurator.h  FileLayer.h FileConfigurator.h  DipoleLayer.h \
   DipoleConfigurator.h  Gradient.h  ColorGrid.h  SpatialProperty.h \
   AtomicDensity.h  Sanderson.h  FragmentLayer.h  ReindexAtomsHandler.h \
   ConstraintLayer.h  ConstraintConfigurator.h  GLShape.h  LogMessageDialog.h \
   Snapshot.h  EnigmaMachine.h  PasswordVault.h  Server.h  ServerListDialog.h \
   JobInfo.h  ProcessMonitor.h \
   Process.h  Timer.h  System.h \
   SecureConnection.h  SSHFileConfigurator.h \
   PBSQueue.h  PBSConfigurator.h  ServerRegistry.h \
   Threaded.h  SecureConnectionThread.h \
   ServerDelegate.h \
   ServerTask.h \
   HostDelegate.h \
   LocalHost.h \
   RemoteHost.h \
   BasicServer.h \
   PBSServer.h \
   FragmentTable.h \
   ServerDialog.h  ServerOptionsDialog.h \
   LocalConnectionThread.h
           

SOURCES += \
   IQmolApplication.C  main.C  MainWindow.C  ToolBar.C  Preferences.C  Viewer.C \
   HelpBrowser.C  PeriodicTable.C  ViewerModel.C  MeshLayer.C \
   MeshConfigurator.C  AxesLayer.C  BackgroundLayer.C  MoleculeLayer.C \
   ManipulatedFrameSetConstraint.C  AtomLayer.C  BondLayer.C  ChargeLayer.C \
   UndoCommands.C  SymmetryToleranceDialog.C  ViewerModelView.C  Cursors.C \
   ManipulateHandler.C  SelectHandler.C  ManipulateSelectionHandler.C \
   BuildHandler.C  Shell.C  Grid.C  MOCoefficients.C  ProgressDialog.C \
   SurfaceLayer.C  MarchingCubes.C  CubeDataLayer.C  QChemParser.C  InfoLayer.C \
   InfoConfigurator.C  BackgroundConfigurator.C  MoleculeConfigurator.C \
   SurfaceConfigurator.C  BaseParser.C  ExternalChargesParser.C \
   OpenBabelParser.C  FrequenciesConfigurator.C  FrequenciesLayer.C  Animator.C \
   ConformerListLayer.C  FormattedCheckpointParser.C   MolecularOrbitalsLayer.C \
   MolecularOrbitalsConfigurator.C  DipoleLayer.C  ConformerListConfigurator.C \
   CubeDataConfigurator.C  FileConfigurator.C  DipoleConfigurator.C  Gradient.C \
   ColorGrid.C  SpatialProperty.C  AtomicDensity.C  Sanderson.C  IQmol.C  \
   FragmentLayer.C  ReindexAtomsHandler.C  ConstraintLayer.C \
   ConstraintConfigurator.C  GLShape.C LogMessageDialog.C  Snapshot.C  \
   EnigmaMachine.C  PasswordVault.C  Server.C  ServerListDialog.C \
   ProcessMonitor.C \
   JobInfo.C Process.C  Timer.C  System.C  \
   SecureConnection.C  SSHFileConfigurator.C  \
   PBSQueue.C  PBSConfigurator.C  ServerRegistry.C \
   SecureConnectionThread.C \
   ServerDialog.C  ServerOptionsDialog.C \
   LocalHost.C \
   RemoteHost.C \
   ServerTask.C \
   BasicServer.C \
   PBSServer.C \
   FragmentTable.C \
   LocalConnectionThread.C
   

FORMS += \
   HelpBrowser.ui  PreferencesBrowser.ui  ToolBar.ui \
   AboutDialog.ui  MeshConfigurator.ui  BackgroundConfigurator.ui \
   MoleculeConfigurator.ui  SymmetryToleranceDialog.ui \
   ProgressDialog.ui  SurfaceConfigurator.ui CubeDataConfigurator.ui \
   InfoConfigurator.ui  FrequenciesConfigurator.ui \
   MolecularOrbitalsConfigurator.ui  ConformerListConfigurator.ui \
   FileConfigurator.ui  DipoleConfigurator.ui  GradientDialog.ui  \
   VectorConstraintConfigurator.ui  ScalarConstraintConfigurator.ui \
   LogMessageDialog.ui  SetPasswordDialog.ui  GetVaultKeyDialog.ui \
   ServerListDialog.ui   \
   FragmentTable.ui   \
   ProcessMonitor.ui  SSHFileConfigurator.ui   PBSConfigurator.ui \
   ServerDialog.ui  ServerOptionsDialog.ui
