CONFIG += no_keywords
QT     += xml opengl gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QMAKE_CXXFLAGS += -O2 -g
#QMAKE_CXXFLAGS += -O0 -g


win32 {
#  CONFIG += debug exceptions rtti
   CONFIG += release exceptions rtti

   INCLUDEPATH += C:\Qt\Qt5.3.0\Tools\mingw482_32\i686-w64-mingw32\include\c++\i686-w64-mingw32

   # Boost
   INCLUDEPATH += C:\Users\agilbert\Development\boost_1_54_0
   LIBS        += C:\Users\agilbert\Development\boost_1_54_0\stage\lib\libboost_serialization-mgw48-mt-1_54.a
   LIBS        += C:\Users\agilbert\Development\boost_1_54_0\stage\lib\libboost_iostreams-mgw48-mt-1_54.a

   # gfortran
   LIBS += C:\MinGW\lib\gcc\mingw32\4.8.1\libgfortran.a

   # OpenBabel
   LIBS += C:\Users\agilbert\Development\extlib\lib\libopenbabel.a 

   # OpenMesh
   INCLUDEPATH += C:\Users\agilbert\Development\OpenMesh-2.4\src
   LIBS        += C:\Users\agilbert\Development\extlib\lib\libOpenMeshToolsd.a 
   LIBS        += C:\Users\agilbert\Development\extlib\lib\libOpenMeshCored.a 

   # SSH2
   LIBS        += -LC:\Users\agilbert\Development\extlib\lib
   LIBS        += -lssh2 -lssl -lcrypto -lgdi32

   # QGLViewer
   INCLUDEPATH += C:\Users\agilbert\Development\libQGLViewer-2.5.2/QGLViewer
   LIBS       += C:\Users\agilbert\Development\extlib\lib\libQGLViewer2.a
   #LIBS        += -lQGLViewer 


   #LIBS += -lws2_32 -lOpenGL32 
   LIBS        += C:\MinGW\lib\libws2_32.a
   LIBS        += C:\MinGW\lib\libopengl32.a
   LIBS        += C:\MinGW\lib\libglu32.a
   LIBS        += C:\MinGW\lib\libz.a

   ICON     = C:\Users\agilbert\Development\IQmol\src\Main\resources\IQmol.ico
   RC_FILE += C:\Users\agilbert\Development\IQmol\src\Main\resources\windows.rc
}




macx {
   greaterThan(QT_MAJOR_VERSION, 4): CONFIG += home
   lessThan(QT_MAJOR_VERSION, 5):    CONFIG += work
   //CONFIG += release

   # QGLViewer
   INCLUDEPATH += $(DEV)/libQGLViewer-2.5.3
   LIBS        += $(DEV)/libQGLViewer-2.5.3/QGLViewer/libQGLViewer.a

   # gfortran
   LIBS += -L$(DEV)/extlib/lib -lgfortran 

   # OpenMesh
   INCLUDEPATH += $(DEV)/OpenMesh-2.4/src
   LIBS        += $(DEV)/OpenMesh-2.4/build/Build/lib/OpenMesh/libOpenMeshCored.a
   LIBS        += $(DEV)/OpenMesh-2.4/build/Build/lib/OpenMesh/libOpenMeshToolsd.a


   # OpenBabel
   INCLUDEPATH += /usr/local/include/openbabel-2.0
   LIBS        += -L/usr/local/lib -lopenbabel

   # Misc
   LIBS += -L/usr/X11/lib
   LIBS += -framework GLUT
   LIBS += -L/usr/local/lib -lssl -lz
}


home {
   # SSH2
   INCLUDEPATH += $(DEV)/libssh2-1.4.3/include
   LIBS        += $(DEV)/libssh2-1.4.3/src/.libs/libssh2.a

   #libcrypto
   LIBS += $(DEV)/extlib/lib/libcrypto.a

   # Boost
   INCLUDEPATH  += $(DEV)/extlib/include
   LIBS         += $(DEV)/extlib/lib/libboost_iostreams.a \
                   $(DEV)/extlib/lib/libboost_serialization.a \
                   $(DEV)/extlib/lib/libboost_exception.a
}


work {
   # SSH2
   INCLUDEPATH += $(DEV)/extlib/include/libssh2
   LIBS        += -L$(DEV)/extlib/lib/libssh2/ -lssh2 -lcrypto

   #libcrypto
   LIBS += -lcrypto

   # Boost
   INCLUDEPATH += $(DEV)/boost_1_56_0/build/include
   LIBS        += $(DEV)/boost_1_56_0/build/lib/libboost_iostreams.a
   LIBS        += $(DEV)/boost_1_56_0/build/lib/libboost_serialization.a
   LIBS        += $(DEV)/boost_1_56_0/build/lib/libboost_exception.a
}


unix:!macx {
   # QGLViewer
   INCLUDEPATH += $(DEV)/libQGLViewer-2.6.0
   LIBS        += $(DEV)/libQGLViewer-2.6.0/QGLViewer/libQGLViewer.a

   # Boost
   INCLUDEPATH += $(DEV)/boost_1_57_0
#  LIBS        += $(DEV)/boost_1_57_0/stage/lib/libboost_iostreams.a
   LIBS        += $(DEV)/boost_1_57_0/stage/lib/libboost_serialization.a
   LIBS        += $(DEV)/boost_1_57_0/stage/lib/libboost_exception.a
   
   # OpenMesh
   INCLUDEPATH += $(DEV)/OpenMesh-3.2/src
   LIBS        += $(DEV)/OpenMesh-3.2/build/Build/lib/OpenMesh/libOpenMeshCored.a
   LIBS        += $(DEV)/OpenMesh-3.2/build/Build/lib/OpenMesh/libOpenMeshToolsd.a

   # OpenBabel
   INCLUDEPATH += $(DEV)/openbabel-2.3.2/include
   LIBS        += -L$(DEV)/openbabel-2.3.2/build/lib -lopenbabel

   # SSH2
   INCLUDEPATH += $(DEV)/libssh2-1.4.3/include
   LIBS        += $(DEV)/extlib/lib/libssh2.a

   # libcrypto
   LIBS += $(DEV)/extlib/lib/libcrypto.a

   # gfortran
   LIBS        += /usr/lib/gcc/x86_64-linux-gnu/4.6/libgfortran.a

   QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\'' 
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
