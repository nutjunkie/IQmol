LIB = Viewer
CONFIG += lib 
include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer \
                ../Configurator ../Network ../Yaml ../Process ../Main ../Old \
                ../OpenMesh/src
INCLUDEPATH +=  $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header


SOURCES += \
   $$PWD/Animator.C \
   $$PWD/BuildAtomHandler.C \
   $$PWD/BuildEfpFragmentHandler.C \
   $$PWD/BuildFunctionalGroupHandler.C \
   $$PWD/BuildHandler.C \
   $$PWD/BuildMoleculeFragmentHandler.C \
   $$PWD/CameraDialog.C \
   $$PWD/Cursors.C \
   $$PWD/GLSLmath.C \
   $$PWD/GLShape.C \
   $$PWD/GLShapeLibrary.C \
   $$PWD/ManipulateHandler.C \
   $$PWD/ManipulateSelectionHandler.C \
   $$PWD/ManipulatedFrameSetConstraint.C \
   $$PWD/PovRayGen.C \
   $$PWD/ReindexAtomsHandler.C \
   $$PWD/SelectHandler.C \
   $$PWD/ShaderDialog.C \
   $$PWD/ShaderLibrary.C \
   $$PWD/Snapshot.C \
   $$PWD/Viewer.C \
   $$PWD/ViewerModel.C \
   $$PWD/ViewerModelView.C \


HEADERS += \
   $$PWD/Animator.h \
   $$PWD/BaseHandler.h \
   $$PWD/BuildAtomHandler.h \
   $$PWD/BuildEfpFragmentHandler.h \
   $$PWD/BuildFunctionalGroupHandler.h \
   $$PWD/BuildHandler.h \
   $$PWD/BuildMoleculeFragmentHandler.h \
   $$PWD/CameraDialog.h \
   $$PWD/Cursors.h \
   $$PWD/GLSLmath.h \
   $$PWD/GLShape.h \
   $$PWD/GLShapeLibrary.h \
   $$PWD/ManipulateHandler.h \
   $$PWD/ManipulateSelectionHandler.h \
   $$PWD/ManipulatedFrameSetConstraint.h \
   $$PWD/PovRayGen.h \
   $$PWD/ReindexAtomsHandler.h \
   $$PWD/SelectHandler.h \
   $$PWD/ShaderDialog.h \
   $$PWD/ShaderLibrary.h \
   $$PWD/Snapshot.h \
   $$PWD/Viewer.h \
   $$PWD/ViewerModel.h \
   $$PWD/ViewerModelView.h \

FORMS += \
   $$PWD/CameraDialog.ui \
   $$PWD/Snapshot.ui \
   $$PWD/ShaderDialog.ui \
