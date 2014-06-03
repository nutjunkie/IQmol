CONFIG += main
TARGET  = IQmol

include(../common.pri)

LIBS        += $$BUILD_DIR/libQui.a \
               $$BUILD_DIR/libParser.a \
               $$BUILD_DIR/libData.a \
               $$BUILD_DIR/libLayer.a \
               $$BUILD_DIR/libConfigurator.a \
               $$BUILD_DIR/libUtil.a

# Windows requires this
# include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer ../Configurator
INCLUDEPATH += $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header

macx:FORMS       += $$PWD/PeriodicTableMac.ui
win32:FORMS      += $$PWD/PeriodicTable.ui
unix:!macx:FORMS += $$PWD/PeriodicTable.ui

SOURCES += $$PWD/gl2ps.C 
HADERS  += $$PWD/gl2ps.h 


OBJECTS += $$PWD/symmol.o

SOURCES += \
   $$PWD/main.C \
   $$PWD/Animator.C \
   $$PWD/AtomLayer.C \
   $$PWD/AtomicDensity.C \
   $$PWD/BasicServer.C \
   $$PWD/BondLayer.C \
   $$PWD/BoundingBoxDialog.C \
   $$PWD/BuildAtomHandler.C \
   $$PWD/BuildEfpFragmentHandler.C \
   $$PWD/BuildFunctionalGroupHandler.C \
   $$PWD/BuildHandler.C \
   $$PWD/BuildMoleculeFragmentHandler.C \
   $$PWD/ChargeLayer.C \
   $$PWD/ColorGrid.C \
   $$PWD/Cursors.C \
   $$PWD/EnigmaMachine.C \
   $$PWD/FragmentTable.C \
   $$PWD/GLSLmath.C \
   $$PWD/GLShape.C \
   $$PWD/GLShapeLibrary.C \
   $$PWD/Gradient.C \
   $$PWD/GridEvaluator.C \
   $$PWD/GridInfoDialog.C \
   $$PWD/HelpBrowser.C \
   $$PWD/HttpServer.C \
   $$PWD/HttpThread.C \
   $$PWD/IQmol.C \
   $$PWD/IQmolApplication.C \
   $$PWD/JobInfo.C \
   $$PWD/Lebedev.C \
   $$PWD/LocalConnectionThread.C \
   $$PWD/LocalHost.C \
   $$PWD/LogMessageDialog.C \
   $$PWD/MainWindow.C \
   $$PWD/ManipulateHandler.C \
   $$PWD/ManipulateSelectionHandler.C \
   $$PWD/ManipulatedFrameSetConstraint.C \
   $$PWD/MarchingCubes.C \
   $$PWD/MeshDecimator.C \
   $$PWD/MoleculeConfigurator.C \
   $$PWD/MoleculeLayer.C \
   $$PWD/PBSServer.C \
   $$PWD/ParseJobFiles.C \
   $$PWD/PasswordVault.C \
   $$PWD/PeriodicTable.C \
   $$PWD/PreferencesBrowser.C \
   $$PWD/PrimitiveLayer.C \
   $$PWD/Process.C \
   $$PWD/ProcessMonitor.C \
   $$PWD/ProgressDialog.C \
   $$PWD/ReindexAtomsHandler.C \
   $$PWD/RemoteHost.C \
   $$PWD/SGEServer.C \
   $$PWD/SSHFileDialog.C \
   $$PWD/SecureConnection.C \
   $$PWD/SecureConnectionThread.C \
   $$PWD/SelectHandler.C \
   $$PWD/Server.C \
   $$PWD/ServerDialog.C \
   $$PWD/ServerListDialog.C \
   $$PWD/ServerOptionsDialog.C \
   $$PWD/ServerQueue.C \
   $$PWD/ServerQueueDialog.C \
   $$PWD/ServerRegistry.C \
   $$PWD/ServerTask.C \
   $$PWD/ShaderDialog.C \
   $$PWD/ShaderLibrary.C \
   $$PWD/Shell.C \
   $$PWD/Snapshot.C \
   $$PWD/SpatialProperty.C \
   $$PWD/SurfaceAnimatorDialog.C \
   $$PWD/SymmetryToleranceDialog.C \
   $$PWD/System.C \
   $$PWD/Timer.C \
   $$PWD/ToolBar.C \
   $$PWD/UndoCommands.C \
   $$PWD/Viewer.C \
   $$PWD/ViewerModel.C \
   $$PWD/ViewerModelView.C \
   $$PWD/WebHost.C


HEADERS += \
   $$PWD/AboutDialog.h \
   $$PWD/Animator.h \
   $$PWD/AtomLayer.h \
   $$PWD/AtomicDensity.h \
   $$PWD/BasicServer.h \
   $$PWD/BondLayer.h \
   $$PWD/BoundingBoxDialog.h \
   $$PWD/BuildAtomHandler.h \
   $$PWD/BuildEfpFragmentHandler.h \
   $$PWD/BuildFunctionalGroupHandler.h \
   $$PWD/BuildHandler.h \
   $$PWD/BuildMoleculeFragmentHandler.h \
   $$PWD/ChargeLayer.h \
   $$PWD/ColorGrid.h \
   $$PWD/Cursors.h \
   $$PWD/EnigmaMachine.h \
   $$PWD/FragmentTable.h \
   $$PWD/GLSLmath.h \
   $$PWD/GLShape.h \
   $$PWD/GLShapeLibrary.h \
   $$PWD/GlobalLayer.h \
   $$PWD/Gradient.h \
   $$PWD/GridEvaluator.h \
   $$PWD/GridInfoDialog.h \
   $$PWD/HelpBrowser.h \
   $$PWD/HostDelegate.h \
   $$PWD/HttpServer.h \
   $$PWD/HttpThread.h \
   $$PWD/IQmol.h \
   $$PWD/IQmolApplication.h \
   $$PWD/JobInfo.h \
   $$PWD/Lebedev.h \
   $$PWD/LocalConnectionThread.h \
   $$PWD/LocalHost.h \
   $$PWD/LogMessageDialog.h \
   $$PWD/MainWindow.h \
   $$PWD/ManipulateHandler.h \
   $$PWD/ManipulateSelectionHandler.h \
   $$PWD/ManipulatedFrameSetConstraint.h \
   $$PWD/MarchingCubes.h \
   $$PWD/MeshDecimator.h \
   $$PWD/MoleculeConfigurator.h \
   $$PWD/MoleculeLayer.h \
   $$PWD/PBSServer.h \
   $$PWD/ParseJobFiles.h \
   $$PWD/PasswordVault.h \
   $$PWD/PeriodicTable.h \
   $$PWD/PreferencesBrowser.h \
   $$PWD/PrimitiveLayer.h \
   $$PWD/Process.h \
   $$PWD/ProcessMonitor.h \
   $$PWD/ProgressDialog.h \
   $$PWD/ReindexAtomsHandler.h \
   $$PWD/RemoteHost.h \
   $$PWD/SGEServer.h \
   $$PWD/SshFileDialog.h \
   $$PWD/SecureConnection.h \
   $$PWD/SecureConnectionThread.h \
   $$PWD/SelectHandler.h \
   $$PWD/Server.h \
   $$PWD/ServerDelegate.h \
   $$PWD/ServerDialog.h \
   $$PWD/ServerListDialog.h \
   $$PWD/ServerOptionsDialog.h \
   $$PWD/ServerQueue.h \
   $$PWD/ServerQueueDialog.h \
   $$PWD/ServerRegistry.h \
   $$PWD/ServerTask.h \
   $$PWD/ShaderDialog.h \
   $$PWD/ShaderLibrary.h \
   $$PWD/Shell.h \
   $$PWD/Snapshot.h \
   $$PWD/SpatialProperty.h \
   $$PWD/SurfaceAnimatorDialog.h \
   $$PWD/SymmetryToleranceDialog.h \
   $$PWD/System.h \
   $$PWD/Threaded.h \
   $$PWD/Timer.h \
   $$PWD/ToolBar.h \
   $$PWD/UndoCommands.h \
   $$PWD/Viewer.h \
   $$PWD/ViewerModel.h \
   $$PWD/ViewerModelView.h \
   $$PWD/WebHost.h


FORMS += \
   $$PWD/AboutDialog.ui \
   $$PWD/BoundingBoxDialog.ui \
   $$PWD/FragmentTable.ui \
   $$PWD/GetVaultKeyDialog.ui \
   $$PWD/GradientDialog.ui \
   $$PWD/GridInfoDialog.ui \
   $$PWD/HelpBrowser.ui \
   $$PWD/LogMessageDialog.ui \
   $$PWD/MoleculeConfigurator.ui \
   $$PWD/PreferencesBrowser.ui \
   $$PWD/ProcessMonitor.ui \
   $$PWD/ProgressDialog.ui \
   $$PWD/SSHFileDialog.ui \
   $$PWD/ServerDialog.ui \
   $$PWD/ServerListDialog.ui \
   $$PWD/ServerOptionsDialog.ui \
   $$PWD/ServerQueueDialog.ui \
   $$PWD/SetPasswordDialog.ui \
   $$PWD/ShaderDialog.ui \
   $$PWD/SurfaceAnimatorDialog.ui \
   $$PWD/SymmetryToleranceDialog.ui \
   $$PWD/ToolBar.ui \
