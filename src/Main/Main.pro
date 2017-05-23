CONFIG += main
TARGET  = IQmol

# This is redefined in common.pri, but for linux we need to worry about the
# ordering of the libraries.


BUILD_DIR  = $$PWD/../../build

LIBS += $$BUILD_DIR/libQui.a \
        $$BUILD_DIR/libViewer.a \
        $$BUILD_DIR/libLayer.a \
        $$BUILD_DIR/libParser.a \
        $$BUILD_DIR/libConfigurator.a \
        $$BUILD_DIR/libData.a \
        $$BUILD_DIR/libProcess.a \
        $$BUILD_DIR/libNetwork.a \
        $$BUILD_DIR/libYaml.a \
        $$BUILD_DIR/libPlot.a \
        $$BUILD_DIR/libOld.a \
        $$BUILD_DIR/libGrid.a \
        $$BUILD_DIR/libUtil.a \

win32:  
else:   LIBS += $$BUILD_DIR/libQGLViewer.a

# Cannot get the embbed OpenMesh working under windows.
!win32 {
LIBS += $$PWD/../OpenMesh/lib/libOpenMeshCore.a \
        $$PWD/../OpenMesh/lib/libOpenMeshTools.a
}

include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer \
                ../Configurator ../Network ../Yaml ../Process ../Old ../Viewer \
                ../OpenMesh/src
INCLUDEPATH += $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header


symmol.target = $$BUILD_DIR/symmol.o
symmol.commands = gfortran -c $$PWD/symmol.f90 -o $$BUILD_DIR/symmol.o
OBJECTS += $$BUILD_DIR/symmol.o
QMAKE_EXTRA_TARGETS += symmol

macx:FORMS       += $$PWD/PeriodicTableMac.ui
win32:FORMS      += $$PWD/PeriodicTable.ui
unix:!macx:FORMS += $$PWD/PeriodicTable.ui


SOURCES += \
   $$PWD/FragmentTable.C \
   $$PWD/HelpBrowser.C \
   $$PWD/InsertMoleculeDialog.C \
   $$PWD/IQmol.C \
   $$PWD/IQmolApplication.C \
   $$PWD/MainWindow.C \
   $$PWD/PeriodicTable.C \
   $$PWD/PreferencesBrowser.C \
   $$PWD/ToolBar.C \
   $$PWD/main.C \

HEADERS += \
   $$PWD/AboutDialog.h \
   $$PWD/FragmentTable.h \
   $$PWD/HelpBrowser.h \
   $$PWD/InsertMoleculeDialog.h \
   $$PWD/IQmol.h \
   $$PWD/IQmolApplication.h \
   $$PWD/MainWindow.h \
   $$PWD/PeriodicTable.h \
   $$PWD/PreferencesBrowser.h \
   $$PWD/ToolBar.h \

FORMS += \
   $$PWD/AboutDialog.ui \
   $$PWD/FragmentTable.ui \
   $$PWD/HelpBrowser.ui \
   $$PWD/InsertMoleculeDialog.ui \
   $$PWD/PeriodicTable.ui \
   $$PWD/PeriodicTableMac.ui \
   $$PWD/PreferencesBrowser.ui \
   $$PWD/ToolBar.ui \
