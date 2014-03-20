CONFIG -= lib
CONFIG += app
TARGET  = Parser
include(../../common.pri)

INCLUDEPATH += ../../Util ../../Data ../
LIBS        += $$BUILD_DIR/libData.a $$BUILD_DIR/libUtil.a $$BUILD_DIR/libParser.a

SOURCES      = main.C
