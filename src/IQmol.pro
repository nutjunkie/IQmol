######################################################################
#
#  This is the main IQmol project file.  To use this file type one of
#  the following commands:
#
#     OS X:     qmake -spec macx-g++ -o Makefile IQmol.pro
#     Linux:    qmake -unix -o Makefile IQmol.pro
#     Windows:  qmake.exe -win32 -o Makefile IQmol.pro
#
#  Note changes to settings should be made to the common.pri file
#
######################################################################

CONFIG  += ordered

TEMPLATE = subdirs

SUBDIRS  = QGLViewer \
#          OpenBabel \
           OpenMesh \
           OpenMesh/src/OpenMesh/Core \
           OpenMesh/src/OpenMesh/Tools \
           Util \
           Data \
           Qui \
           Parser \
           Configurator \
           Old \
           Layer \
           Network \
           Yaml \
           Plot \
           Process \
           Viewer \
           Main \
