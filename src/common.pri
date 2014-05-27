CONFIG += no_keywords
QT     += xml opengl

QMAKE_CXXFLAGS += -O2 -g
#QMAKE_CXXFLAGS += -O0 -g

macx {
   CONFIG += release

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


win23 {
   CONFIG += debug exceptions rtti

   INCLUDEPATH += "C:\Users\qchem\Documents\extlib\include"
   INCLUDEPATH += "C:\Users\qchem\Documents\IQmol\src"
   INCLUDEPATH +=  C:\Users\qchem\Documents\boost_1_51_0
   INCLUDEPATH +=  C:\Users\qchem\Documents\OpenMesh-2.4\src

   LIBS += -LC:\Users\qchem\Documents\extlib\lib
   LIBS += -lQGLViewerd2 -lssh2 -lz -lssl -lcrypto -lgdi32
   LIBS += -lws2_32 -lOpenGL32 
   LIBS +=  C:\Users\qchem\Documents\extlib\lib\libOpenMeshToolsd.a 
   LIBS +=  C:\Users\qchem\Documents\extlib\lib\libOpenMeshCored.a 
   LIBS +=  C:\Users\qchem\Documents\extlib\lib\libopenbabel.a 

   LIBS += -L"C:\Program Files\gfortran\bin" -lgfortran-3 -lgcc_s_dw2-1
   #LIBS +=  C:\Users\qchem\Documents\extlib\lib\libboost_serialization.a
   #LIBS +=  C:\Users\qchem\Documents\extlib\lib\libboost_iostreams.a
   LIBS += C:\Users\qchem\Documents\boost_1_51_0\stage\lib\libboost_serialization-mgw34-mt-1_51.a
   LIBS += C:\Users\qchem\Documents\boost_1_51_0\stage\lib\libboost_iostreams-mgw34-mt-1_51.a
   LIBS += -lz

   ICON = \
       "C:\Users\qchem\Documents\IQmol\IQmol-master\src\IQmol\resources\IQmol.ico"
   RC_FILE += \
      "C:\Users\qchem\Documents\IQmol\IQmol-master\src\IQmol\resources\windows.rc"
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
