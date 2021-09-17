CONFIG += DEPLOY


contains(CONFIG, DEPLOY) {
   #CONFIG += debug
   CONFIG += release

   # Boost
   BOOST        = $(DEV)/boost_1_64_0
   INCLUDEPATH += $${BOOST}
   LIBS        += $${BOOST}/stage/lib/libboost_iostreams.a
   LIBS        += $${BOOST}/stage/lib/libboost_serialization.a
   LIBS        += $${BOOST}/stage/lib/libboost_exception.a

   # libssl/libcrypto
   INCLUDEPATH += $(DEV)/openssl-1.1.1f/include
   LIBS        += $(DEV)/openssl-1.1.1f/libssl.a
   LIBS        += $(DEV)/openssl-1.1.1f/libcrypto.a

   # SSH2
   INCLUDEPATH += $(DEV)/libssh2-1.9.0/include
   LIBS        += $(DEV)/libssh2-1.9.0/build/src/libssh2.a

   # OpenBabel
   OPENBABEL    = $(DEV)/openbabel-2.4.1
   INCLUDEPATH += $${OPENBABEL}/include
   INCLUDEPATH += $${OPENBABEL}/build/include
   LIBS        += $${OPENBABEL}/build/src/libopenbabel.a
   LIBS        += $${OPENBABEL}/build/src/formats/libinchi/libinchi.a

   # gfortran
#  LIBS        += /usr/local/Cellar/gcc/10.2.0_4/lib/gcc/10/libquadmath.a 
#  LIBS        += /usr/local/Cellar/gcc/10.2.0_4/lib/gcc/10/libgfortran.a
#  #LIBS        += -L/usr/local/gfortran/lib -lgcc_ext.10.5
#  LIBS        += -L/usr/local/Cellar/gcc/10.2.0_4/lib/gcc/10 -lgcc_ext.10.5

   # /usr/local/gfortran/bin/gfortran  -c Main/symmol.f90 -o ../build/symmol.o
   LIBS        += /usr/local/gfortran/lib/libgfortran.a
   LIBS        += /usr/local/gfortran/lib/libquadmath.a
   LIBS        += -L/usr/local/gfortran/lib -lgcc_ext.10.5

   # Misc
   LIBS        += -L/usr/X11/lib  
   LIBS        += -framework GLUT
   LIBS        += -L/usr/lib -lz -lxml2

   QMAKE_LFLAGS   += -Wl,-no_compact_unwind -stdlib=libc++ 
   QMAKE_RPATHDIR += @executable_path/../Frameworks
}
