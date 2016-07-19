LIB = Qui
CONFIG += lib
include(../common.pri)
QT += sql

INCLUDEPATH += ../Util ../QMsgBox ../Old ../Process ../Parser ../Data

SOURCES += \
   $$PWD/Actions.C \
   $$PWD/ExternalChargesSection.C \
   $$PWD/GeometryConstraint.C \
   $$PWD/InputDialog.C \
   $$PWD/InputDialogLogic.C \
   $$PWD/KeywordSection.C \
   $$PWD/LJParametersSection.C \
   $$PWD/MoleculeSection.C \
   $$PWD/Option.C \
   $$PWD/OptionDatabase.C \
   $$PWD/OptionDatabaseForm.C \
   $$PWD/OptionEditors.C \
   $$PWD/OptSection.C \
   $$PWD/QCJob.C \
   $$PWD/Qui.C \
   $$PWD/QuiMolecule.C \
   $$PWD/RemSection.C \
   $$PWD/ReadInput.C \

HEADERS += \
   $$PWD/Actions.h \
   $$PWD/Conditions.h \
   $$PWD/ExternalChargesSection.h \
   $$PWD/GeometryConstraint.h \
   $$PWD/InputDialog.h \
   $$PWD/KeywordSection.h \
   $$PWD/LJParametersSection.h \
   $$PWD/MoleculeSection.h \
   $$PWD/Node.h \
   $$PWD/Option.h \
   $$PWD/OptionDatabase.h \
   $$PWD/OptionDatabaseForm.h \
   $$PWD/OptionEditors.h \
   $$PWD/OptSection.h \
   $$PWD/QCJob.h \
   $$PWD/Qui.h \
   $$PWD/QuiMolecule.h \
   $$PWD/QtNode.h \
   $$PWD/Register.h \
   $$PWD/RemSection.h \

FORMS += \
   $$PWD/GeometryConstraintDialog.ui \
   $$PWD/OptionDatabaseForm.ui \
   $$PWD/OptionListEditor.ui \
   $$PWD/OptionNumberEditor.ui \
   $$PWD/QuiMainWindow.ui
