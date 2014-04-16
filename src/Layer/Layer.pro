LIB = Layer
CONFIG += lib
include(../common.pri)

INCLUDEPATH += ../Data ../Util ../IQmol ../Configurator ../Parser
INCLUDEPATH += $$BUILD_DIR/IQmol

SOURCES = \
   $$PWD/AxesLayer.C \
   $$PWD/AxesMeshLayer.C \
   $$PWD/BackgroundLayer.C \
   $$PWD/ConstraintLayer.C \
   $$PWD/ContainerLayer.C \
   $$PWD/CubeDataLayer.C \
   $$PWD/DipoleLayer.C \
   $$PWD/GeometryLayer.C \
   $$PWD/GeometryListLayer.C \
   $$PWD/EfpFragmentLayer.C \
   $$PWD/EfpFragmentListLayer.C \
   $$PWD/FileLayer.C \
   $$PWD/FrequenciesLayer.C \
   $$PWD/GroupLayer.C \
   $$PWD/InfoLayer.C \
   $$PWD/Layer.C \
   $$PWD/LayerFactory.C \
   $$PWD/MolecularOrbitalsLayer.C \
   $$PWD/SurfaceLayer.C

HEADERS = \
   $$PWD/AxesLayer.h \
   $$PWD/AxesMeshLayer.h \
   $$PWD/BackgroundLayer.h \
   $$PWD/ConstraintLayer.h \
   $$PWD/ContainerLayer.h \
   $$PWD/CubeDataLayer.h \
   $$PWD/DipoleLayer.h \
   $$PWD/GeometryLayer.h \
   $$PWD/GeometryListLayer.h \
   $$PWD/EfpFragmentLayer.h \
   $$PWD/EfpFragmentListLayer.h \
   $$PWD/FileLayer.h \
   $$PWD/FrequenciesLayer.h \
   $$PWD/GLObjectLayer.h \
   $$PWD/GroupLayer.h \
   $$PWD/InfoLayer.h \
   $$PWD/Layer.h \
   $$PWD/LayerFactory.h \
   $$PWD/MolecularOrbitalsLayer.h \
   $$PWD/SurfaceLayer.h
