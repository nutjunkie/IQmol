LIB = Old
CONFIG += lib
include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer \
                ../Configurator ../Network ../Yaml ../Process ../Main ../Viewer \
                ../OpenMesh/src ../Grid
#INCLUDEPATH +=  $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header

SOURCES += $$PWD/gl2ps.C 
HADERS  += $$PWD/gl2ps.h 


SOURCES += \
   $$PWD/AtomicDensity.C \
   $$PWD/ColorGrid.C \
   $$PWD/LogMessageDialog.C \
   $$PWD/ParseJobFiles.C \
   $$PWD/ProgressDialog.C \
   $$PWD/SpatialProperty.C \
   $$PWD/SurfaceAnimatorDialog.C \
   $$PWD/SymmetryToleranceDialog.C \
   $$PWD/UndoCommands.C \


HEADERS += \
   $$PWD/AtomicDensity.h \
   $$PWD/ColorGrid.h \
   $$PWD/LogMessageDialog.h \
   $$PWD/ParseJobFiles.h \
   $$PWD/ProgressDialog.h \
   $$PWD/SpatialProperty.h \
   $$PWD/SurfaceAnimatorDialog.h \
   $$PWD/SymmetryToleranceDialog.h \
   $$PWD/UndoCommands.h \

FORMS += \
   $$PWD/GetVaultKeyDialog.ui \
   $$PWD/LogMessageDialog.ui \
   $$PWD/ProgressDialog.ui \
   $$PWD/SurfaceAnimatorDialog.ui \
   $$PWD/SymmetryToleranceDialog.ui \
