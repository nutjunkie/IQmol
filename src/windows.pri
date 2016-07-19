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
   #INCLUDEPATH += C:\Users\agilbert\Development\OpenMesh-2.4\src
   #LIBS        += C:\Users\agilbert\Development\extlib\lib\libOpenMeshToolsd.a 
   #LIBS        += C:\Users\agilbert\Development\extlib\lib\libOpenMeshCored.a 

   # SSH2
   INCLUDEPATH += C:\Users\agilbert\Development\extlib\include
   LIBS        += -LC:\Users\agilbert\Development\extlib\lib
   LIBS        += -lssh2 -lssl -lcrypto -lgdi32

   # QGLViewer
   #INCLUDEPATH += C:\Users\agilbert\Development\libQGLViewer-2.5.2
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
