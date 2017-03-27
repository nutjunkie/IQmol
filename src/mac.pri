macx {
   CONFIG += release

   # Boost
   #BOOST        = /Users/iqmol/Dev/boost_1_58_0
   #INCLUDEPATH += $${BOOST}
   #LIBS        += $${BOOST}/stage/lib/libboost_iostreams.a
   #LIBS        += $${BOOST}/stage/lib/libboost_serialization.a
   #LIBS        += $${BOOST}/stage/lib/libboost_exception.a
   BOOST        = /usr/local/opt/boost@1.57
   INCLUDEPATH += $${BOOST}/include
   LIBS        += $${BOOST}/lib/libboost_iostreams.a
   LIBS        += $${BOOST}/lib/libboost_serialization.a
   LIBS        += $${BOOST}/lib/libboost_exception.a

   # OpenBabel
   OPENBABEL    = /Users/iqmol/Dev/openbabel-2.4.1
   INCLUDEPATH += $${OPENBABEL}/include
   INCLUDEPATH += $${OPENBABEL}/build/include
   LIBS        += $${OPENBABEL}/build/src/libopenbabel.a

   # SSH2
   INCLUDEPATH += /usr/local/include
   LIBS        += /usr/local/lib/libssh2.a

   # libssl/libcrypto
   INCLUDEPATH += /usr/local/include
   LIBS        += /usr/local/lib/libssl.a 
   LIBS        += /usr/local/lib/libcrypto.a

   # gfortran
   LIBS += /usr/local/gfortran/lib/libgfortran.a
   LIBS += /usr/local/gfortran/lib/libquadmath.a
   LIBS += -L/usr/local/gfortran/lib -lgcc_ext.10.5

   # Misc
   LIBS += -L/usr/X11/lib  
   LIBS += -framework GLUT
   LIBS += -L/usr/lib -lz

   QMAKE_LFLAGS   += -Wl,-no_compact_unwind
   #QMAKE_LFLAGS   += -Wl,-no_compact_unwind -stdlib=libstdc++
   QMAKE_RPATHDIR += @executable_path/../Frameworks
}
