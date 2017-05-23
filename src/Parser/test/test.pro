CONFIG -= lib
CONFIG += app
TARGET  = Parser

#include(../../common.pri)

QT     += xml opengl gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QMAKE_CXXFLAGS += -O2 -g -ggdb

BUILD_DIR       = $$PWD/build
INCLUDEPATH    += $$PWD  $$PWD/../../../build

app {
   CONFIG      -= staticlib
   TEMPLATE     = app
   DESTDIR      = $$BUILD_DIR/..
   OBJECTS_DIR  = $$BUILD_DIR
   MOC_DIR      = $$BUILD_DIR
   UI_DIR       = $$BUILD_DIR
}

INCLUDEPATH += ../../Util ../../Data ../
LIBS += $$PWD/../../../build/libParser.a \
        $$PWD/../../../build/libData.a \
	$$PWD/../../../build/libYaml.a \
	$$PWD/../../../build/libUtil.a \
        $$PWD/../../../build/libQGLViewer.a

LIBS += $$PWD/../../OpenMesh/lib/libOpenMeshCore.a \
        $$PWD/../../OpenMesh/lib/libOpenMeshTools.a

include(../../linux.pri)

SOURCES      = main.C


