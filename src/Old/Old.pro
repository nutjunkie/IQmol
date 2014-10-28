LIB = Old
CONFIG += lib
include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer \
                ../Configurator ../Network ../Yaml ../Process
INCLUDEPATH += ../Process $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header

SOURCES += $$PWD/gl2ps.C 
HADERS  += $$PWD/gl2ps.h 


SOURCES += \
   $$PWD/Animator.C \
   $$PWD/AtomicDensity.C \
   $$PWD/BasicServer.C \
   $$PWD/BoundingBoxDialog.C \
   $$PWD/BuildAtomHandler.C \
   $$PWD/BuildEfpFragmentHandler.C \
   $$PWD/BuildFunctionalGroupHandler.C \
   $$PWD/BuildHandler.C \
   $$PWD/BuildMoleculeFragmentHandler.C \
   $$PWD/ColorGrid.C \
   $$PWD/Cursors.C \
   $$PWD/GLSLmath.C \
   $$PWD/GLShape.C \
   $$PWD/GLShapeLibrary.C \
   $$PWD/GridEvaluator.C \
   $$PWD/GridInfoDialog.C \
   $$PWD/IQmol.C \
   $$PWD/JobInfo.C \
   $$PWD/Lebedev.C \
   $$PWD/LocalConnectionThread.C \
   $$PWD/LocalHost.C \
   $$PWD/LogMessageDialog.C \
   $$PWD/ManipulateHandler.C \
   $$PWD/ManipulateSelectionHandler.C \
   $$PWD/ManipulatedFrameSetConstraint.C \
   $$PWD/MarchingCubes.C \
   $$PWD/MeshDecimator.C \
   $$PWD/PBSServer.C \
   $$PWD/ParseJobFiles.C \
   $$PWD/Process.C \
   $$PWD/ProcessMonitor.C \
   $$PWD/ProgressDialog.C \
   $$PWD/ReindexAtomsHandler.C \
   $$PWD/RemoteHost.C \
   $$PWD/SGEServer.C \
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
   $$PWD/Snapshot.C \
   $$PWD/SpatialProperty.C \
   $$PWD/SurfaceAnimatorDialog.C \
   $$PWD/SymmetryToleranceDialog.C \
   $$PWD/UndoCommands.C \
   $$PWD/Viewer.C \
   $$PWD/ViewerModel.C \
   $$PWD/ViewerModelView.C \


HEADERS += \
   $$PWD/Animator.h \
   $$PWD/AtomicDensity.h \
   $$PWD/BasicServer.h \
   $$PWD/BoundingBoxDialog.h \
   $$PWD/BuildAtomHandler.h \
   $$PWD/BuildEfpFragmentHandler.h \
   $$PWD/BuildFunctionalGroupHandler.h \
   $$PWD/BuildHandler.h \
   $$PWD/BuildMoleculeFragmentHandler.h \
   $$PWD/ColorGrid.h \
   $$PWD/Cursors.h \
   $$PWD/GLSLmath.h \
   $$PWD/GLShape.h \
   $$PWD/GLShapeLibrary.h \
   $$PWD/GridEvaluator.h \
   $$PWD/GridInfoDialog.h \
   $$PWD/HostDelegate.h \
   $$PWD/IQmol.h \
   $$PWD/JobInfo.h \
   $$PWD/Lebedev.h \
   $$PWD/LocalConnectionThread.h \
   $$PWD/LocalHost.h \
   $$PWD/LogMessageDialog.h \
   $$PWD/ManipulateHandler.h \
   $$PWD/ManipulateSelectionHandler.h \
   $$PWD/ManipulatedFrameSetConstraint.h \
   $$PWD/MarchingCubes.h \
   $$PWD/MeshDecimator.h \
   $$PWD/PBSServer.h \
   $$PWD/ParseJobFiles.h \
   $$PWD/Process.h \
   $$PWD/ProcessMonitor.h \
   $$PWD/ProgressDialog.h \
   $$PWD/ReindexAtomsHandler.h \
   $$PWD/RemoteHost.h \
   $$PWD/SGEServer.h \
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
   $$PWD/Snapshot.h \
   $$PWD/SpatialProperty.h \
   $$PWD/SurfaceAnimatorDialog.h \
   $$PWD/SymmetryToleranceDialog.h \
   $$PWD/Threaded.h \
   $$PWD/UndoCommands.h \
   $$PWD/Viewer.h \
   $$PWD/ViewerModel.h \
   $$PWD/ViewerModelView.h \

FORMS += \
   $$PWD/BoundingBoxDialog.ui \
   $$PWD/GetVaultKeyDialog.ui \
   $$PWD/GridInfoDialog.ui \
   $$PWD/LogMessageDialog.ui \
   $$PWD/ProcessMonitor.ui \
   $$PWD/ProgressDialog.ui \
   $$PWD/ServerDialog.ui \
   $$PWD/ServerListDialog.ui \
   $$PWD/ServerOptionsDialog.ui \
   $$PWD/ServerQueueDialog.ui \
   $$PWD/ShaderDialog.ui \
   $$PWD/SurfaceAnimatorDialog.ui \
   $$PWD/SymmetryToleranceDialog.ui \
