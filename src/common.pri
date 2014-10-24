CONFIG += no_keywords
QT     += xml opengl gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets 
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += home
lessThan(QT_MAJOR_VERSION, 5): CONFIG += work

#QMAKE_CXXFLAGS += -O2 -g
QMAKE_CXXFLAGS += -O0 -g

macx {
   CONFIG += release

   # QGLViewer
   INCLUDEPATH += $(DEV)/libQGLViewer-2.5.3
   LIBS        += $(DEV)/libQGLViewer-2.5.3/QGLViewer/libQGLViewer.a

   # gfortran
   LIBS += -L$(DEV)/extlib/lib -lgfortran 

   # OpenMesh
   INCLUDEPATH += $(DEV)/OpenMesh-2.4/src
   LIBS        += $(DEV)/OpenMesh-2.4/build/Build/lib/OpenMesh/libOpenMeshCored.a
   LIBS        += $(DEV)/OpenMesh-2.4/build/Build/lib/OpenMesh/libOpenMeshToolsd.a

   # SSH2
   # work
   #INCLUDEPATH += $(DEV)/extlib/include/libssh2
   #LIBS        += -L$(DEV)/extlib/lib/libssh2/ -lssh2 -lcrypto
   # home
   #INCLUDEPATH += $(DEV)/libssh2-1.4.3/include
   #LIBS        += $(DEV)/libssh2-1.4.3/src/.libs/libssh2.a

   #libcrypto
   # work
   #LIBS += -lcrypto
   # home
   #LIBS += $(DEV)/extlib/lib/libcrypto.a

   # Boost
   # work
   #INCLUDEPATH += $(DEV)/boost_1_56_0/build/include
   #LIBS        += $(DEV)/boost_1_56_0/build/lib/libboost_iostreams.a
   #LIBS        += $(DEV)/boost_1_56_0/build/lib/libboost_serialization.a
   #LIBS        += $(DEV)/boost_1_56_0/build/lib/libboost_exception.a
   # home
   #INCLUDEPATH  += $(DEV)/extlib/include
   #LIBS         += $(DEV)/extlib/lib/libboost_iostreams.a \
   #                $(DEV)/extlib/lib/libboost_serialization.a \
   #                $(DEV)/extlib/lib/libboost_exception.a

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
