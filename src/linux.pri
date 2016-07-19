unix:!macx {

   CONFIG += DISTRIB

   contains(CONFIG, DEVELOP){
      #message("---- DEVELOP set ----")

      INCLUDEPATH += /usr/local/include
      LIBS        += -L/usr/local/lib

      # Boost
      LIBS        += -lboost_iostreams -lboost_serialization
   
      # OpenBabel
      INCLUDEPATH += /usr/local/include/openbabel-2.0
      LIBS        += -lopenbabel

      # SSH2/libssl/crypto
      LIBS        += -lssh2 -lssl -lcrypto

      # gfortran
      LIBS        += /usr/lib/gcc/x86_64-linux-gnu/4.7.3/libgfortran.a

      # Misc
      LIBS        += -lz -ldl

      #QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\''
   }


   contains(CONFIG, DISTRIB) {
      #message("---- DISTRIB set ----")

      CONFIG += release
 
      INCLUDEPATH += /usr/local/include
      LIBS        += -L/usr/local/lib

      # Boost
      LIBS        += /usr/lib/libboost_serialization.a
      LIBS        += /usr/lib/libboost_iostreams.a
   
      # OpenBabel
      OPENBABEL    = $(DEV)/openbabel-2.3.2
      INCLUDEPATH += $${OPENBABEL}/include
      LIBS        += -L$${OPENBABEL}/build/lib/ -lopenbabel

      # SSH2
      LIBSSH2      = $(DEV)/libssh2-1.6.0
      INCLUDEPATH += $${LIBSSH2}/include
      LIBS        += $$LIBSSH2/src/.libs/libssh2.a

      # libssl/crypto
      LIBSSL       = $(DEV)/openssl-1.0.2d
      INCLUDEPATH += $${LIBSSL}/include
      LIBS        += $${LIBSSL}/libssl.a $${LIBSSL}/libcrypto.a

      # gfortran
      LIBS        += /usr/lib/gcc/x86_64-linux-gnu/4.7.3/libgfortran.a

      # Misc
      LIBS        += -lz -ldl

      QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\'' 
   }
 
}
