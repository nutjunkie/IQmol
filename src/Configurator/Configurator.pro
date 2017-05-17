LIB = Configurator
CONFIG += lib
include(../common.pri)

INCLUDEPATH += ../Layer ../Data ../Util ../Plot ../Viewer \
               ../Main ../Grid ../Old ../OpenMesh/src

SOURCES = \
   $$PWD/AxesMeshConfigurator.C \
   $$PWD/BackgroundConfigurator.C \
   $$PWD/ClippingPlaneConfigurator.C \
   $$PWD/ConstraintConfigurator.C \
   $$PWD/CubeDataConfigurator.C \
   $$PWD/DipoleConfigurator.C \
   $$PWD/EfpFragmentListConfigurator.C \
   $$PWD/ExcitedStatesConfigurator.C \
   $$PWD/FileConfigurator.C \
   $$PWD/FrequenciesConfigurator.C \
   $$PWD/GeminalOrbitalsConfigurator.C \
   $$PWD/GeometryListConfigurator.C \
   $$PWD/InfoConfigurator.C \
   $$PWD/MoleculeConfigurator.C \
   $$PWD/MolecularOrbitalsConfigurator.C \
   $$PWD/MolecularSurfacesConfigurator.C \
   $$PWD/NmrConfigurator.C \
   $$PWD/OrbitalsConfigurator.C \
   $$PWD/SurfaceConfigurator.C

HEADERS = \
   $$PWD/AxesMeshConfigurator.h \
   $$PWD/BackgroundConfigurator.h \
   $$PWD/ClippingPlaneConfigurator.h \
   $$PWD/Configurator.h \
   $$PWD/ConstraintConfigurator.h \
   $$PWD/CubeDataConfigurator.h \
   $$PWD/DipoleConfigurator.h \
   $$PWD/EfpFragmentListConfigurator.h \
   $$PWD/ExcitedStatesConfigurator.h \
   $$PWD/FileConfigurator.h \
   $$PWD/FrequenciesConfigurator.h \
   $$PWD/GeminalOrbitalsConfigurator.h \
   $$PWD/GeometryListConfigurator.h \
   $$PWD/InfoConfigurator.h \
   $$PWD/MoleculeConfigurator.h \
   $$PWD/MolecularOrbitalsConfigurator.h \
   $$PWD/MolecularSurfacesConfigurator.h \
   $$PWD/NmrConfigurator.h \
   $$PWD/OrbitalsConfigurator.h \
   $$PWD/SurfaceConfigurator.h

FORMS = \
   $$PWD/AxesMeshConfigurator.ui \
   $$PWD/BackgroundConfigurator.ui \
   $$PWD/ClippingPlaneConfigurator.ui \
   $$PWD/CubeDataConfigurator.ui \
   $$PWD/DipoleConfigurator.ui \
   $$PWD/EfpFragmentListConfigurator.ui \
   $$PWD/ExcitedStatesConfigurator.ui \
   $$PWD/FileConfigurator.ui \
   $$PWD/FrequenciesConfigurator.ui \
   $$PWD/GeminalOrbitalsConfigurator.ui \
   $$PWD/GeometryListConfigurator.ui \
   $$PWD/InfoConfigurator.ui \
   $$PWD/MoleculeConfigurator.ui \
   $$PWD/MolecularOrbitalsConfigurator.ui \
   $$PWD/MolecularSurfacesConfigurator.ui \
   $$PWD/NmrConfigurator.ui \
   $$PWD/OrbitalsConfigurator.ui \
   $$PWD/ScalarConstraintConfigurator.ui \
   $$PWD/SurfaceConfigurator.ui \
   $$PWD/VectorConstraintConfigurator.ui \
   $$PWD/../Old/SurfaceAnimatorDialog.ui \
