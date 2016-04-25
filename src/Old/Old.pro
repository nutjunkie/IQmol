LIB = Old
CONFIG += lib
include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer \
                ../Configurator ../Network ../Yaml ../Process ../Main ../Viewer
#INCLUDEPATH +=  $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header

SOURCES += $$PWD/gl2ps.C 
HADERS  += $$PWD/gl2ps.h 


SOURCES += \
   $$PWD/AtomicDensity.C \
   $$PWD/BoundingBoxDialog.C \
   $$PWD/ColorGrid.C \
   $$PWD/GridEvaluator.C \
   $$PWD/GridInfoDialog.C \
   $$PWD/Lebedev.C \
   $$PWD/LogMessageDialog.C \
   $$PWD/MarchingCubes.C \
   $$PWD/MeshDecimator.C \
   $$PWD/ParseJobFiles.C \
   $$PWD/ProgressDialog.C \
   $$PWD/SpatialProperty.C \
   $$PWD/SurfaceAnimatorDialog.C \
   $$PWD/SymmetryToleranceDialog.C \
   $$PWD/UndoCommands.C \


HEADERS += \
   $$PWD/AtomicDensity.h \
   $$PWD/BoundingBoxDialog.h \
   $$PWD/ColorGrid.h \
   $$PWD/GridEvaluator.h \
   $$PWD/GridInfoDialog.h \
   $$PWD/Lebedev.h \
   $$PWD/LogMessageDialog.h \
   $$PWD/MarchingCubes.h \
   $$PWD/MeshDecimator.h \
   $$PWD/ParseJobFiles.h \
   $$PWD/ProgressDialog.h \
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
   $$PWD/SurfaceAnimatorDialog.ui \
   $$PWD/SymmetryToleranceDialog.ui \
