CONFIG += main
TARGET  = IQmol

include(../common.pri)

LIBS += $$BUILD_DIR/libQui.a \
        $$BUILD_DIR/libParser.a \
        $$BUILD_DIR/libData.a \
        $$BUILD_DIR/libLayer.a \
        $$BUILD_DIR/libConfigurator.a \
        $$BUILD_DIR/libNetwork.a \
        $$BUILD_DIR/libYaml.a \
        $$BUILD_DIR/libOld.a \
        $$BUILD_DIR/libProcess.a \
        $$BUILD_DIR/libUtil.a

# Windows requires this
# include(../common.pri)

INCLUDEPATH += . ../Util ../Data ../Parser ../Qui ../Layer \
                ../Configurator ../Network ../Yaml ../Process ../Old
INCLUDEPATH += $$BUILD_DIR/Qui   # Required for the ui_QuiMainWindow.h header

macx:FORMS       += $$PWD/PeriodicTableMac.ui
win32:FORMS      += $$PWD/PeriodicTable.ui
unix:!macx:FORMS += $$PWD/PeriodicTable.ui

OBJECTS += $(DEV)/extlib/obj/symmol.o

SOURCES += \
   $$PWD/FragmentTable.C \
   $$PWD/HelpBrowser.C \
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
   $$PWD/IQmolApplication.h \
   $$PWD/MainWindow.h \
   $$PWD/PeriodicTable.h \
   $$PWD/PreferencesBrowser.h \
   $$PWD/ToolBar.h \

FORMS += \
   $$PWD/AboutDialog.ui \
   $$PWD/FragmentTable.ui \
   $$PWD/HelpBrowser.ui \
   $$PWD/PeriodicTable.ui \
   $$PWD/PeriodicTableMac.ui \
   $$PWD/PreferencesBrowser.ui \
   $$PWD/ToolBar.ui \
