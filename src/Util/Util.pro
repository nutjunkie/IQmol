LIB = Util
CONFIG += lib
include(../common.pri)

SOURCES = \
   $$PWD/Align.C \
   $$PWD/ColorGradient.C \
   $$PWD/ColorGradientDialog.C \
   $$PWD/EulerAngles.C \
   $$PWD/Function.C \
   $$PWD/GLShape.C \
   $$PWD/GLShapeLibrary.C \
   $$PWD/Matrix.C \
   $$PWD/Preferences.C \
   $$PWD/qcprot.C \
   $$PWD/RemoveDirectory.C \
   $$PWD/ScanDirectory.C \
   $$PWD/SetButtonColor.C \
   $$PWD/Task.C \
   $$PWD/Timer.C \
   $$PWD/WriteToTemporaryFile.C \

HEADERS = \
   $$PWD/Align.h \
   $$PWD/Axes.h \
   $$PWD/ColorGradient.h \
   $$PWD/ColorGradientDialog.h \
   $$PWD/Constants.h \
   $$PWD/EulerAngles.h \
   $$PWD/Exception.h \
   $$PWD/Function.h \
   $$PWD/GLShape.h \
   $$PWD/GLShapeLibrary.h \
   $$PWD/Matrix.h \
   $$PWD/Numerical.h \
   $$PWD/OpenGL.h \
   $$PWD/Preferences.h \
   $$PWD/qcprot.h \
   $$PWD/RemoveDirectory.h \
   $$PWD/ScanDirectory.h \
   $$PWD/SetButtonColor.h \
   $$PWD/StringFormat.h \
   $$PWD/Task.h \
   $$PWD/Timer.h \
   $$PWD/WriteToTemporaryFile.h \

FORMS += \
   $$PWD/ColorGradientDialog.ui \



# QMsgBox
SOURCES += \
   $$PWD/QMsgBox.C

HEADERS += \
   $$PWD/QMsgBox.h



# QsLog files
DEFINES += QS_LOG_LINE_NUMBERS

SOURCES += \
    $$PWD/QsLogDest.C \
    $$PWD/QsLog.C \
    $$PWD/QsDebugOutput.C

HEADERS += \
    $$PWD/QSLogDest.h \
    $$PWD/QsLog.h \
    $$PWD/QsDebugOutput.h \
    $$PWD/QsLogLevel.h
