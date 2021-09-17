#CONFIG += no_keywords

QT     += xml opengl gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QMAKE_CXXFLAGS += -O2 -g -ggdb

# Set the $DEV environment variable to the top directory used to compile all
# the packages.  To simplify distribution, we use static libraries wherever
# possible.  Platform dependent settings are made in the following files:

win32:     { include(windows.pri) }
macx:      { include(mac.pri)   }
unix:!macx { include(linux.pri) }


# Path to the build directory ($$PWD contains IQmol.pro)
BUILD_DIR     = $$PWD/../build
INCLUDEPATH  += $$PWD  $$BUILD_DIR


lib {
   CONFIG      += staticlib
   CONFIG      -= shared
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
