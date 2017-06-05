######################################################################
#
#  This is the main IQmol project file.  To use this file it should be
#  sufficient to type:
#
#     qmake IQmol.pro
#     make  (mingw32-make under windows)
#
#  Several libraries are required and the compliation of these is
#  discussed in the doc/build* files.
#
#  This top-level file should only contain the project subdirectories.
#  Global configuration should be done in common.pri and platform-
#  specific configuration should be done in {mac,windows,linux}.pri
#
######################################################################

CONFIG  += ordered
TEMPLATE = subdirs


# Cannot get the included OpenMesh working under Windows.  For
# win32 the OpenMesh libraries are specifed in windows.pri.  See
# also Main.pri for the final link order for the subdirectory 
# libraries (this is important for resolving symbol dependancies).

!win32: {
SUBDIRS += OpenMesh \
           OpenMesh/src/OpenMesh/Core \
           OpenMesh/src/OpenMesh/Tools \
}

SUBDIRS += \
   QGLViewer \
   Util \
   Data \
   Qui \
   Parser \
   Configurator \
   Grid \
   Old \
   Layer \
   Network \
   Yaml \
   Plot \
   Process \
   Viewer \
   Main \
