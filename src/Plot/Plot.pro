LIB = Plot
CONFIG += lib
include(../common.pri)

INCLUDEPATH += . ../Util 

SOURCES = \
   $$PWD/CustomPlot.C \
   $$PWD/qcustomplot.cpp

HEADERS = \
   $$PWD/CustomPlot.h \
   $$PWD/qcustomplot.h
