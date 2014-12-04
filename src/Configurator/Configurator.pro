LIB = Configurator
CONFIG += lib
include(../common.pri)

INCLUDEPATH += ../Layer ../Data ../Util ../Old
#INCLUDEPATH += $$BUILD_DIR/IQmol

SOURCES = \
   $$PWD/AxesMeshConfigurator.C \
   $$PWD/BackgroundConfigurator.C \
   $$PWD/ConstraintConfigurator.C \
   $$PWD/CubeDataConfigurator.C \
   $$PWD/DipoleConfigurator.C \
   $$PWD/EfpFragmentListConfigurator.C \
   $$PWD/FileConfigurator.C \
   $$PWD/FrequenciesConfigurator.C \
   $$PWD/GeometryListConfigurator.C \
   $$PWD/InfoConfigurator.C \
   $$PWD/MoleculeConfigurator.C \
   $$PWD/MolecularOrbitalsConfigurator.C \
   $$PWD/SurfaceConfigurator.C

HEADERS = \
   $$PWD/AxesMeshConfigurator.h \
   $$PWD/BackgroundConfigurator.h \
   $$PWD/Configurator.h \
   $$PWD/ConstraintConfigurator.h \
   $$PWD/CubeDataConfigurator.h \
   $$PWD/DipoleConfigurator.h \
   $$PWD/EfpFragmentListConfigurator.h \
   $$PWD/FileConfigurator.h \
   $$PWD/FrequenciesConfigurator.h \
   $$PWD/GeometryListConfigurator.h \
   $$PWD/InfoConfigurator.h \
   $$PWD/MoleculeConfigurator.h \
   $$PWD/MolecularOrbitalsConfigurator.h \
   $$PWD/SurfaceConfigurator.h

FORMS = \
   $$PWD/AxesMeshConfigurator.ui \
   $$PWD/BackgroundConfigurator.ui \
   $$PWD/CubeDataConfigurator.ui \
   $$PWD/DipoleConfigurator.ui \
   $$PWD/EfpFragmentListConfigurator.ui \
   $$PWD/FileConfigurator.ui \
   $$PWD/FrequenciesConfigurator.ui \
   $$PWD/GeometryListConfigurator.ui \
   $$PWD/InfoConfigurator.ui \
   $$PWD/MoleculeConfigurator.ui \
   $$PWD/MolecularOrbitalsConfigurator.ui \
   $$PWD/ScalarConstraintConfigurator.ui \
   $$PWD/SurfaceConfigurator.ui \
   $$PWD/VectorConstraintConfigurator.ui \
