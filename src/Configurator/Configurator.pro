LIB = Configurator
CONFIG += lib
include(../common.pri)

INCLUDEPATH += ../Layer ../Data ../Util ../IQmol
INCLUDEPATH += $$BUILD_DIR/IQmol

SOURCES = \
   $$PWD/GeometryListConfigurator.C \
   $$PWD/CubeDataConfigurator.C \
   $$PWD/DipoleConfigurator.C \
   $$PWD/EfpFragmentListConfigurator.C \
   $$PWD/FileConfigurator.C \
   $$PWD/FrequenciesConfigurator.C \
   $$PWD/InfoConfigurator.C \
   $$PWD/MolecularOrbitalsConfigurator.C \
   $$PWD/SurfaceConfigurator.C

HEADERS = \
   $$PWD/Configurator.h \
   $$PWD/CubeDataConfigurator.h \
   $$PWD/DipoleConfigurator.h \
   $$PWD/EfpFragmentListConfigurator.h \
   $$PWD/FileConfigurator.h \
   $$PWD/FrequenciesConfigurator.h \
   $$PWD/GeometryListConfigurator.h \
   $$PWD/InfoConfigurator.h \
   $$PWD/MolecularOrbitalsConfigurator.h \
   $$PWD/SurfaceConfigurator.h

FORMS = \
   $$PWD/CubeDataConfigurator.ui \
   $$PWD/DipoleConfigurator.ui \
   $$PWD/EfpFragmentListConfigurator.ui \
   $$PWD/FileConfigurator.ui \
   $$PWD/FrequenciesConfigurator.ui \
   $$PWD/GeometryListConfigurator.ui \
   $$PWD/InfoConfigurator.ui \
   $$PWD/MolecularOrbitalsConfigurator.ui \
   $$PWD/SurfaceConfigurator.ui
