unix:!macx {
   #CONFIG += release
 
   INCLUDEPATH += /usr/local/include
   LIBS        += -L/usr/local/lib

   # QGLViewer
   #QGLVIEWER    = $(DEV)/libQGLViewer-2.6.3
   #INCLUDEPATH += $${QGLVIEWER}
   #LIBS        += $${QGLVIEWER}/QGLViewer/libQGLViewer.a

   # Boost
   #BOOST        = /usr/lib
   #LIBS        += $${BOOST}/libboost_iostreams.a
   #LIBS        += $${BOOST}/libboost_serialization.a
   LIBS         += -lboost_iostreams -lboost_serialization
   
   # OpenMesh
   #OPENMESH     = $(DEV)/OpenMesh-3.3
   #INCLUDEPATH += $${OPENMESH}/src
   #LIBS        += $${OPENMESH}/build/Build/lib/libOpenMeshCore.a
   #LIBS        += $${OPENMESH}/build/Build/lib/libOpenMeshTools.a

   # OpenBabel
   #OPENBABEL    = $(DEV)/openbabel-2.3.2
   #INCLUDEPATH += $${OPENBABEL}/include
   INCLUDEPATH  += /usr/local/include/openbabel-2.0
   LIBS         += -lopenbabel

   # SSH2
   #LIBSSH2      = $(DEV)/libssh2-1.6.0
   #INCLUDEPATH += $${LIBSSH2}/include
   #LIBS        += $$LIBSSH2/src/.libs/libssh2.a

   # libssl/crypto
   #LIBSSL       = $(DEV)/openssl-1.0.2d
   #INCLUDEPATH += $${LIBSSL}/include
   #LIBS        += $${LIBSSL}/libssl.a $${LIBSSL}/libcrypto.a
   LIBS         += -lssh2 -lssl -lcrypto

   # gfortran
   LIBS        += /usr/lib/gcc/x86_64-linux-gnu/4.7.3/libgfortran.a

   # Misc
   LIBS        += -lz -ldl

   QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\'' 
}
