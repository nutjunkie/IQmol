macx {
   CONFIG += release

   # Boost
   BOOST        = $(DEV)/boost_1_58_0
   INCLUDEPATH += $${BOOST}
   LIBS        += $${BOOST}/stage/lib/libboost_iostreams.a
   LIBS        += $${BOOST}/stage/lib/libboost_serialization.a
   LIBS        += $${BOOST}/stage/lib/libboost_exception.a

   # OpenBabel
   #INCLUDEPATH += /usr/local/include/openbabel-2.0
   #LIBS        += -L/usr/local/lib -lopenbabel
   OPENBABEL    = $(DEV)/openbabel-2.3.2
   INCLUDEPATH += $${OPENBABEL}/include
   LIBS        += $${OPENBABEL}/build/src/libopenbabel.a

   # SSH2
   INCLUDEPATH += $(DEV)/extlib/include
   LIBS        += $(DEV)/extlib/lib//libssh2.a

   # libssl/libcrypto
   INCLUDEPATH += $(DEV)/extlib/include
   LIBS        += $(DEV)/extlib/lib/libssl.a 
   LIBS        += $(DEV)/extlib/lib/libcrypto.a

   # gfortran
   LIBS += /usr/local/gfortran/lib/libgfortran.a
   LIBS += /usr/local/gfortran/lib/libquadmath.a
   LIBS += -L/usr/local/gfortran/lib -lgcc_ext.10.5

   # Misc
   LIBS += -L/usr/X11/lib  
   LIBS += -framework GLUT
   LIBS += -L/usr/lib -lz

   QMAKE_LFLAGS   += -Wl,-no_compact_unwind -stdlib=libstdc++
   QMAKE_RPATHDIR += @executable_path/../Frameworks
}
