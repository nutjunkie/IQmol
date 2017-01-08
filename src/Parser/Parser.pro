LIB     = Parser
CONFIG += lib
include(../common.pri)

INCLUDEPATH += ../Util ../Data ../Yaml ../OpenMesh/src

SOURCES += \
   $$PWD/Parser.C \
   $$PWD/ParseFile.C \
   $$PWD/CartesianCoordinatesParser.C \
   $$PWD/CubeParser.C \
   $$PWD/EfpFragmentParser.C \
   $$PWD/ExternalChargesParser.C \
   $$PWD/FormattedCheckpointParser.C \
   $$PWD/GdmaParser.C \
   $$PWD/IQmolParser.C \
   $$PWD/MeshParser.C \
   $$PWD/OpenBabelParser.C \
   $$PWD/PovRayParser.C \
   $$PWD/QChemInputParser.C \
   $$PWD/QChemOutputParser.C \
   $$PWD/QChemPlotParser.C \
   $$PWD/XyzParser.C \
   $$PWD/YamlParser.C \
   $$PWD/ZMatrixCoordinatesParser.C

HEADERS += \
   $$PWD/Parser.h \
   $$PWD/ParseFile.h \
   $$PWD/CartesianCoordinatesParser.h \
   $$PWD/CubeParser.h \
   $$PWD/EfpFragmentParser.h \
   $$PWD/ExternalChargesParser.h \
   $$PWD/FormattedCheckpointParser.h \
   $$PWD/GdmaParser.h \
   $$PWD/IQmolParser.h \
   $$PWD/MeshParser.h \
   $$PWD/OpenBabelParser.h \
   $$PWD/PovRayParser.h \
   $$PWD/QChemInputParser.h \
   $$PWD/QChemOutputParser.h \
   $$PWD/QChemPlotParser.h \
   $$PWD/TextStream.h \
   $$PWD/XyzParser.h \
   $$PWD/YamlParser.h \
   $$PWD/ZMatrixCoordinatesParser.h
