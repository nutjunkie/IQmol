LIB = Old
CONFIG += lib
include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer \
                ../Configurator ../Network ../Yaml ../Process ../Main
INCLUDEPATH +=  $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header

SOURCES += $$PWD/gl2ps.C 
HADERS  += $$PWD/gl2ps.h 


SOURCES += \
   $$PWD/Animator.C \
   $$PWD/AtomicDensity.C \
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
   $$PWD/Lebedev.C \
   $$PWD/LogMessageDialog.C \
   $$PWD/ManipulateHandler.C \
   $$PWD/ManipulateSelectionHandler.C \
   $$PWD/ManipulatedFrameSetConstraint.C \
   $$PWD/MarchingCubes.C \
   $$PWD/MeshDecimator.C \
   $$PWD/ParseJobFiles.C \
   $$PWD/ProgressDialog.C \
   $$PWD/ReindexAtomsHandler.C \
   $$PWD/SelectHandler.C \
   $$PWD/ShaderDialog.C \
   $$PWD/ShaderLibrary.C \
   $$PWD/Snapshot.C \
   $$PWD/SpatialProperty.C \
   $$PWD/SurfaceAnimatorDialog.C \
   $$PWD/SymmetryToleranceDialog.C \
   $$PWD/UndoCommands.C \


HEADERS += \
   $$PWD/Animator.h \
   $$PWD/AtomicDensity.h \
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
   $$PWD/IQmol.h \
   $$PWD/Lebedev.h \
   $$PWD/LogMessageDialog.h \
   $$PWD/ManipulateHandler.h \
   $$PWD/ManipulateSelectionHandler.h \
   $$PWD/ManipulatedFrameSetConstraint.h \
   $$PWD/MarchingCubes.h \
   $$PWD/MeshDecimator.h \
   $$PWD/ParseJobFiles.h \
   $$PWD/ProgressDialog.h \
   $$PWD/ReindexAtomsHandler.h \
   $$PWD/SelectHandler.h \
   $$PWD/ShaderDialog.h \
   $$PWD/ShaderLibrary.h \
   $$PWD/Snapshot.h \
   $$PWD/SpatialProperty.h \
   $$PWD/SurfaceAnimatorDialog.h \
   $$PWD/SymmetryToleranceDialog.h \
   $$PWD/UndoCommands.h \

FORMS += \
   $$PWD/BoundingBoxDialog.ui \
   $$PWD/GetVaultKeyDialog.ui \
   $$PWD/GridInfoDialog.ui \
   $$PWD/LogMessageDialog.ui \
   $$PWD/ProgressDialog.ui \
   $$PWD/ShaderDialog.ui \
   $$PWD/SurfaceAnimatorDialog.ui \
   $$PWD/SymmetryToleranceDialog.ui \
