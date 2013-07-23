project(IQmol C CXX Fortran)
cmake_minimum_required(VERSION 2.8)

find_package(OpenGL REQUIRED)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtOpenGL QtSql QtXml
    REQUIRED)
set(Boost_USE_STATIC_LIBS TRUE)
find_package(Boost COMPONENTS iostreams serialization REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_MODULE_PATH ${IQmol_SOURCE_DIR}/cmake)
find_package(OpenBabel2 REQUIRED)
find_package(QGLViewer REQUIRED)
find_package(LibSsh2 REQUIRED)
unset(CMAKE_MODULE_PATH)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS} ${QGLVIEWER_DEFINITIONS})
include_directories(${OPENBABEL2_INCLUDE_DIR} ${QGLVIEWER_INCLUDE_DIR} ${LIBSSH2_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

add_subdirectory(src)

