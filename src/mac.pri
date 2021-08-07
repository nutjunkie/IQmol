CONFIG += DEPLOY

contains(CONFIG, DEPLOY) {
   #CONFIG += debug
   CONFIG += release

   # Boost
   BOOST        = /usr/local/Cellar/boost/1.76.0/
   INCLUDEPATH += $${BOOST}/include
   LIBS        += $${BOOST}/lib/libboost_iostreams.a
   LIBS        += $${BOOST}/lib/libboost_serialization.a
   LIBS        += $${BOOST}/lib/libboost_exception.a

   # libssl/libcrypto
   INCLUDEPATH += /usr/local/Cellar/openssl\@1.1/1.1.1k/include/
   LIBS        += /usr/local/Cellar/openssl@1.1/1.1.1k/lib/libssl.a
   LIBS        += /usr/local/Cellar/openssl@1.1/1.1.1k/lib/libcrypto.a

   # SSH2
   INCLUDEPATH += /opt/anaconda3/envs/iqmol-dev/include
   LIBS        += /opt/anaconda3/envs/iqmol-dev/lib/libssh2.dylib

   # OpenBabel
   OPENBABEL    = /opt/anaconda3/envs/iqmol-dev/include/openbabel-2.0/
   INCLUDEPATH += /opt/anaconda3/envs/iqmol-dev/include/openbabel-2.0/
   LIBS        += /opt/anaconda3/envs/iqmol-dev/lib/libopenbabel.dylib
   LIBS        += /opt/anaconda3/envs/iqmol-dev/lib/libopenbabel.5.dylib
   LIBS        += /opt/anaconda3/envs/iqmol-dev/lib/libinchi.dylib

   # gfortran
   LIBS        += /usr/local/Cellar/gcc/11.2.0/lib/gcc/11/libgfortran.a
   LIBS        += /usr/local/Cellar/gcc/11.2.0/lib/gcc/11/libquadmath.a
   LIBS        += -L/usr/local/Cellar/gcc/11.2.0/lib/gcc/11/ -lgcc_ext.10.5

   # Misc
   LIBS        += -L/usr/X11/lib  
   LIBS        += -framework GLUT
   LIBS        += -L/usr/lib -lz -lxml2

   QMAKE_LFLAGS   += -Wl,-no_compact_unwind -stdlib=libc++ 
   QMAKE_RPATHDIR += @executable_path/../Frameworks
}
