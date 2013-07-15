######################################################################
#
#  This is the IQmol project file.  To use this file type one of the 
#  following commands:
#
#     OS X:     qmake -spec macx-g++ -macx -o Makefile IQmol.pro
#     Linux:    qmake -unix -o Makefile IQmol.pro
#     Windows:  qmake.exe -win32 -o Makefile IQmol.pro
#
######################################################################

TEMPLATE     =  app
DEPENDPATH  +=  .
CONFIG      +=  no_keywords qt opengl
RESOURCES   +=  IQmol.qrc
ICON         =  resources/IQmol.icns
QT          +=  sql xml opengl network
OBJECTS_DIR  = ../build
DESTDIR      = ../
MOC_DIR      = ../build/moc
UI_DIR       = ../build/ui

QMAKE_CXXFLAGS += -O2
#QMAKE_CXXFLAGS += -O0 -g

######################################################################
#
#  More recent versions of qmake/gcc can handle fortran source files.
#  If yours cannot, generate symmol.o via:
#
#   gfortran -c symmol.f90
#
#  and swap the comments in the following SOURCES and OBJECTS
#
######################################################################
#SOURCES += symmol.f90
OBJECTS += symmol.o


include(QsLog/QsLog.pri)
include(GL2PS/GL2PS.pri)
include(QMsgBox/QMsgBox.pri)
include(QUI/QUI.pri)
include(QCPRotation/QCPRotation.pri)

include(Data/Data.pri)
include(Parser/Parser.pri)


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
   LIBS += -L$(DEV)/Boost/lib -lboost_iostreams -lboost_serialization

   CONFIG += release
   QMAKE_INFO_PLIST = resources/Info.plist
   FORMS += PeriodicTableMac.ui
}

win32 {
   INCLUDEPATH += "C:\Users\qchem\Documents\extlib\include"
   INCLUDEPATH += "C:\Users\qchem\Documents\IQmol\src"
   INCLUDEPATH += C:\Users\qchem\Documents\boost_1_51_0

   LIBS += -LC:\Users\qchem\Documents\extlib\lib
   LIBS += -lQGLViewerd2 -lssh2 -lz -lssl -lcrypto -lgdi32
   LIBS += -lws2_32 -lopenbabel
   LIBS += -L"C:\Program Files\gfortran\bin" -lgfortran-3 -lgcc_s_dw2-1
   LIBS += C:\Users\qchem\Documents\boost_1_51_0\stage\lib\libboost_serialization-mgw34-mt-1_51.a
   LIBS += C:\Users\qchem\Documents\boost_1_51_0\stage\lib\libboost_iostreams-mgw34-mt-1_51.a  -lz

   SOURCES += QUI/getpids.C
   ICON = resources/IQmol.ico
   CONFIG += debug exceptions rtti
   RC_FILE += resources/windows.rc
   include(OpenBabel/OpenBabel.pro)
   FORMS += PeriodicTable.ui
   include(GLEW/GLEW.pri)
   DEFINES += GLEW_STATIC
}

unix:!macx {
   INCLUDEPATH += $(DEV)/extlib/include
   INCLUDEPATH += $(DEV)/openbabel/include/openbabel-2.0
   LIBS += -L/usr/lib64
   LIBS += -L/usr/lib -lopenbabel
   LIBS += -L/usr/local/gfortran/lib -lgfortran
   LIBS += -L$(DEV)/extlib/lib -lssh2 -lcrypto -lz -lssl
   LIBS += -lboost_iostreams -lboost_serialization
   LIBS += $(DEV)/extlib/lib/libQGLViewer.a
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
   ViewerModelView.h  Cursors.h \
   ManipulateHandler.h \
   SelectHandler.h \
   ManipulateSelectionHandler.h  \
   BuildHandler.h  \
   BuildAtomHandler.h  \
   BuildFunctionalGroupHandler.h \
   BuildEFPFragmentHandler.h \
   EFPFragmentListConfigurator.h \
   BuildMoleculeFragmentHandler.h \
   Shell.h  Grid.h \
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
   AtomicDensity.h  ReindexAtomsHandler.h \
   ConstraintLayer.h  ConstraintConfigurator.h  GLShape.h  LogMessageDialog.h \
   Snapshot.h  EnigmaMachine.h  PasswordVault.h  Server.h  ServerListDialog.h \
   JobInfo.h  ProcessMonitor.h \
   Process.h  Timer.h  System.h \
   SecureConnection.h  SSHFileConfigurator.h \
   ServerRegistry.h \
   ServerQueue.h  ServerQueueDialog.h  \
   Threaded.h  SecureConnectionThread.h \
   ServerDelegate.h \
   ServerTask.h \
   HostDelegate.h \
   LocalHost.h \
   RemoteHost.h \
   HttpServer.h \
   HttpThread.h \
   WebHost.h \
   BasicServer.h \
   PBSServer.h \
   SGEServer.h \
   FragmentTable.h \
   ServerDialog.h  ServerOptionsDialog.h \
#  Sanderson.h  \
   GLShapeLibrary.h \
   ContainerLayer.h \
   GroupLayer.h \
   EFPFragmentLayer.h \
   EFPFragmentListLayer.h \
   EFPFragmentParser.h \
   Task.h \
   ShaderLibrary.h \
   ShaderDialog.h \
   Lebedev.h \
   GLSLmath.h \
   GridInfoDialog.h \
   SurfaceAnimatorDialog.h \
   BoundingBoxDialog.h \
   LocalConnectionThread.h
           

SOURCES += \
   IQmolApplication.C  main.C  MainWindow.C  ToolBar.C  Preferences.C  Viewer.C \
   HelpBrowser.C  PeriodicTable.C  ViewerModel.C  MeshLayer.C \
   MeshConfigurator.C  AxesLayer.C  BackgroundLayer.C  MoleculeLayer.C \
   ManipulatedFrameSetConstraint.C  AtomLayer.C  BondLayer.C  ChargeLayer.C \
   UndoCommands.C  SymmetryToleranceDialog.C  ViewerModelView.C  Cursors.C \
   ManipulateHandler.C  SelectHandler.C  ManipulateSelectionHandler.C \
   BuildHandler.C  PrimitiveLayer.C \
   BuildAtomHandler.C  \
   BuildFunctionalGroupHandler.C \
   BuildEFPFragmentHandler.C \
   EFPFragmentListConfigurator.C \
   BuildMoleculeFragmentHandler.C \
   Shell.C  Grid.C  \
   ProgressDialog.C \
   SurfaceLayer.C  MarchingCubes.C  CubeDataLayer.C  QChemParser.C  InfoLayer.C \
   InfoConfigurator.C  BackgroundConfigurator.C  MoleculeConfigurator.C \
   SurfaceConfigurator.C  BaseParser.C  ExternalChargesParser.C \
   OpenBabelParser.C  FrequenciesConfigurator.C  FrequenciesLayer.C  Animator.C \
   ConformerListLayer.C  FormattedCheckpointParser.C   MolecularOrbitalsLayer.C \
   MolecularOrbitalsConfigurator.C  DipoleLayer.C  ConformerListConfigurator.C \
   CubeDataConfigurator.C  FileConfigurator.C  DipoleConfigurator.C  Gradient.C \
   ColorGrid.C  SpatialProperty.C  AtomicDensity.C  IQmol.C  \
   ReindexAtomsHandler.C  ConstraintLayer.C \
   ConstraintConfigurator.C  GLShape.C LogMessageDialog.C  Snapshot.C  \
   EnigmaMachine.C  PasswordVault.C  Server.C  ServerListDialog.C \
   ProcessMonitor.C \
   JobInfo.C Process.C  Timer.C  System.C  \
   SecureConnection.C  SSHFileConfigurator.C  \
   ServerRegistry.C \
   ServerQueue.C  ServerQueueDialog.C  \
   SecureConnectionThread.C \
   ServerDialog.C  ServerOptionsDialog.C \
   LocalHost.C \
   RemoteHost.C \
   HttpServer.C \
   WebHost.C \
   HttpThread.C \
   ServerTask.C \
   BasicServer.C \
   PBSServer.C \
   SGEServer.C \
   FragmentTable.C \
#  Sanderson.C  \
   GLShapeLibrary.C \
   GroupLayer.C \
   EFPFragmentLayer.C \
   EFPFragmentListLayer.C \
   EFPFragmentParser.C \
   ShaderLibrary.C \
   ShaderDialog.C \
   Lebedev.C \
   GLSLmath.C \
   GridInfoDialog.C \
   SurfaceAnimatorDialog.C \
   BoundingBoxDialog.C \
#  glloader.cpp \
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
   ProcessMonitor.ui  SSHFileConfigurator.ui   \
   ServerDialog.ui  ServerOptionsDialog.ui  EFPFragmentListConfigurator.ui \
   ServerQueueDialog.ui  ShaderDialog.ui  SurfaceAnimatorDialog.ui  GridInfoDialog.ui \
   BoundingBoxDialog.ui
   

   
