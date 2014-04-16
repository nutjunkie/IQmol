CONFIG += no_keywords release
QT     += xml opengl

QMAKE_CXXFLAGS += -O0 -g

macx {
   # OpenBabel
   INCLUDEPATH += /usr/local/include/openbabel-2.0
   LIBS        += -L/usr/local/lib -lopenbabel
   #INCLUDEPATH += $(DEV)/extlib/include
   #LIBS        += -L$(DEV)/extlib/lib -lopenbabel
   
   # Boost
   INCLUDEPATH += $(DEV)/Boost/include
   LIBS        += -L$(DEV)/Boost/lib -lboost_iostreams -lboost_serialization
   
   # QGLViewer
   INCLUDEPATH += /Library/Frameworks/QGLViewer.framework/Headers
   LIBS        += -framework QGLViewer

   # OpenMesh
   INCLUDEPATH += /Users/agilbert/Development/OpenMesh-2.4/src
   LIBS        += -L/Users/agilbert/Development/OpenMesh-2.4/build/Build/lib/OpenMesh
   LIBS        += -lOpenMeshCored -lOpenMeshToolsd

   # SSH2
   INCLUDEPATH += $(DEV)/extlib/include
   LIBS        += -L$(DEV)/extlib/lib
   
   INCLUDEPATH += /usr/local/include

   LIBS += -framework GLUT
   LIBS += -L/usr/local/lib -lssl -lssh2 -lcrypto -lz   
   LIBS += -L/usr/local/gfortran/lib -lgfortran 
}




# Path to the build directory ($$PWD contains IQmol.pro)
BUILD_DIR       = $$PWD/../build
INCLUDEPATH    += $$PWD  $$BUILD_DIR

lib {
   CONFIG      += staticlib
   TEMPLATE     = lib
   DESTDIR      = $$BUILD_DIR
   OBJECTS_DIR  = $$BUILD_DIR
   MOC_DIR      = $$BUILD_DIR
   UI_DIR       = $$BUILD_DIR
   INCLUDEPATH += $$PWD/$$LIB
}


app {
   CONFIG      -= staticlib
   TEMPLATE     = app
   DESTDIR      = $$BUILD_DIR/..
   OBJECTS_DIR  = $$BUILD_DIR
   MOC_DIR      = $$BUILD_DIR
   UI_DIR       = $$BUILD_DIR
}


main {
   CONFIG      -= staticlib
   TEMPLATE     = app
   DESTDIR      = $$BUILD_DIR/..
   OBJECTS_DIR  = $$BUILD_DIR
   MOC_DIR      = $$BUILD_DIR
   UI_DIR       = $$BUILD_DIR
   RESOURCES   += IQmol.qrc
   ICON         = resources/IQmol.icns
   QT          += network sql

   macx:QMAKE_INFO_PLIST = resources/Info.plist
}
