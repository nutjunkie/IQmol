LIB = Grid
CONFIG += lib
include(../common.pri)

INCLUDEPATH += ../Util ../Data ../OpenMesh/src  ../Old
               

SOURCES += \
   $$PWD/BasisEvaluator.C \
   $$PWD/BoundingBoxDialog.C \
   $$PWD/DensityEvaluator.C \
   $$PWD/GridEvaluator.C \
   $$PWD/GridInfoDialog.C \
   $$PWD/Lebedev.C \
   $$PWD/MarchingCubes.C \
   $$PWD/MeshDecimator.C \
   $$PWD/MolecularGridEvaluator.C \
   $$PWD/OrbitalEvaluator.C \
   $$PWD/SurfaceGenerator.C \
  


HEADERS += \
   $$PWD/BasisEvaluator.h \
   $$PWD/BoundingBoxDialog.h \
   $$PWD/DensityEvaluator.h \
   $$PWD/GridEvaluator.h \
   $$PWD/GridInfoDialog.h \
   $$PWD/Lebedev.h \
   $$PWD/MarchingCubes.h \
   $$PWD/MeshDecimator.h \
   $$PWD/MolecularGridEvaluator.h \
   $$PWD/OrbitalEvaluator.h \
   $$PWD/SurfaceGenerator.h \

FORMS += \
   $$PWD/BoundingBoxDialog.ui \
   $$PWD/GridInfoDialog.ui \
