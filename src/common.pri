#CONFIG += no_keywords

QT     += xml opengl gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QMAKE_CXXFLAGS += -O2 -g -ggdb


macx {
   #CONFIG += release

   # Set the $DEV environment variable to the top directory used to compile all
   # the packages.  To simplify distribution, we use static libraries wherever
   # possible.

   # SSL/libcrypto
   LIBS        += $(DEV)/openssl-1.0.1p/libssl.a
   LIBS        += $(DEV)/openssl-1.0.1p/libcrypto.a

   # SSH2
   INCLUDEPATH += $(DEV)/libssh2-1.6.0/include
   LIBS        += $(DEV)/libssh2-1.6.0/src/.libs/libssh2.a

   # QGLViewer
   INCLUDEPATH += $(DEV)/libQGLViewer-2.6.3
   LIBS        += $(DEV)/libQGLViewer-2.6.3/QGLViewer/libQGLViewer.a

   # Boost
   INCLUDEPATH += $(DEV)/boost_1_56_0/
   LIBS        += $(DEV)/boost_1_56_0/stage/lib/libboost_iostreams.a
   LIBS        += $(DEV)/boost_1_56_0/stage/lib/libboost_serialization.a
   LIBS        += $(DEV)/boost_1_56_0/stage/lib/libboost_exception.a

   # OpenMesh
   INCLUDEPATH += $(DEV)/OpenMesh-4.0/src
   LIBS        += $(DEV)/OpenMesh-4.0/build/Build/lib/libOpenMeshCore.a
   LIBS        += $(DEV)/OpenMesh-4.0/build/Build/lib/libOpenMeshTools.a

   # OpenBabel
   INCLUDEPATH += /usr/local/include/openbabel-2.0
   LIBS        += -L/usr/local/lib -lopenbabel

   # gfortran
   LIBS += /usr/local/gfortran/lib/libgfortran.a
   LIBS += /usr/local/gfortran/lib/libquadmath.a
   LIBS += -L/usr/local/gfortran/lib -lgcc_ext.10.5


   # Misc
   LIBS += -L/usr/X11/lib  
   LIBS += -framework GLUT
   LIBS += -L/usr/lib -lz

   # QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/../Frameworks
   QMAKE_LFLAGS   += -Wl,-no_compact_unwind -stdlib=libstdc++
   QMAKE_RPATHDIR += /Applications/Qt5.5.0/5.5/clang_64/lib/
}






unix:!macx {
   #CONFIG += release
 
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
   INCLUDEPATH += C:\Users\agilbert\Development\openbabel-2.3.2\include
   LIBS        += C:\Users\agilbert\Development\extlib\lib\libopenbabel.a 

   # OpenMesh
   INCLUDEPATH += C:\Users\agilbert\Development\OpenMesh-2.4\src
   LIBS        += C:\Users\agilbert\Development\extlib\lib\libOpenMeshToolsd.a 
   LIBS        += C:\Users\agilbert\Development\extlib\lib\libOpenMeshCored.a 

   # SSH2
   INCLUDEPATH += C:\Users\agilbert\Development\extlib\include
   LIBS        += -LC:\Users\agilbert\Development\extlib\lib
   LIBS        += -lssh2 -lssl -lcrypto -lgdi32

   # QGLViewer
   INCLUDEPATH += C:\Users\agilbert\Development\libQGLViewer-2.5.2
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
