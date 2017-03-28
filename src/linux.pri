unix:!macx {

   CONFIG += DISTRIB
#  CONFIG += DEVELOP

   contains(CONFIG, DEVELOP){
      #message("---- DEVELOP set ----")

      INCLUDEPATH += /usr/local/include
      LIBS        += -L/usr/local/lib

      # Boost
      LIBS        += -lboost_iostreams -lboost_serialization
   
      # OpenBabel
      INCLUDEPATH += /usr/include/openbabel-2.0
      LIBS        += -lopenbabel

      # SSH2/libssl/crypto
      LIBS        += -lssh2 -lssl -lcrypto

      # gfortran
      LIBS        += /usr/lib/gcc/x86_64-linux-gnu/5/libgfortran.a
      LIBS        += /usr/lib/gcc/x86_64-linux-gnu/5/libquadmath.a # required for gfortran 5

      # Misc
      LIBS        += -lz -ldl

      QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\''
   }


   contains(CONFIG, DISTRIB) {
      #message("---- DISTRIB set ----")

      CONFIG += release
 
      INCLUDEPATH += /usr/local/include
      LIBS        += -L/usr/local/lib

      # Boost
      LIBS        += /usr/lib/x86_64-linux-gnu/libboost_serialization.a
      LIBS        += /usr/lib/x86_64-linux-gnu/libboost_iostreams.a
   
      # OpenBabel
      INCLUDEPATH += /usr/include/openbabel-2.0
      LIBS        += -lopenbabel

      # libssl/crypto
      LIBS        += /usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libssl.a 

      # SSH2/gcrypt
      LIBS        += /usr/lib/x86_64-linux-gnu/libssh2.a
      LIBS        += /usr/lib/x86_64-linux-gnu/libgcrypt.a /usr/lib/x86_64-linux-gnu/libgpg-error.a

      # gfortran
      LIBS        += /usr/lib/gcc/x86_64-linux-gnu/5/libgfortran.a
      LIBS        += /usr/lib/gcc/x86_64-linux-gnu/5/libquadmath.a # required for gfortran 5

      # Misc
      LIBS        += -lz -ldl

      QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib\'' 
   }
 
}
