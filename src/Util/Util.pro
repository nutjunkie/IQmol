LIB = Util
CONFIG += lib
include(../common.pri)

SOURCES = \
   $$PWD/Align.C \
   $$PWD/EulerAngles.C \
   $$PWD/Function.C \
   $$PWD/Matrix.C \
   $$PWD/Preferences.C \
   $$PWD/qcprot.C \
   $$PWD/ScanDirectory.C \
   $$PWD/SetButtonColor.C \
   $$PWD/Task.C \

HEADERS = \
   $$PWD/Align.h \
   $$PWD/Constants.h \
   $$PWD/EulerAngles.h \
   $$PWD/Function.h \
   $$PWD/Matrix.h \
   $$PWD/Numerical.h \
   $$PWD/OpenGL.h \
   $$PWD/Preferences.h \
   $$PWD/qcprot.h \
   $$PWD/ScanDirectory.h \
   $$PWD/SetButtonColor.h \
   $$PWD/Task.h \


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
