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
   $$PWD/KeyValueSection.C \
   $$PWD/LJParametersSection.C \
   $$PWD/MoleculeSection.C \
   $$PWD/Option.C \
   $$PWD/OptionDatabase.C \
   $$PWD/OptionDatabaseForm.C \
   $$PWD/OptionEditors.C \
   $$PWD/OptSection.C \
   $$PWD/PcmSection.C \
   $$PWD/QuiJob.C \
   $$PWD/Qui.C \
   $$PWD/QuiMolecule.C \
   $$PWD/RemSection.C \
   $$PWD/ReadInput.C \


HEADERS += \
   $$PWD/Actions.h \
   $$PWD/AdcTab.h \
   $$PWD/AimdTab.h \
   $$PWD/AttenuationParameterTab.h \
   $$PWD/AuxiliaryBasisTab.h \
   $$PWD/CisTab.h \
   $$PWD/Conditions.h \
   $$PWD/EomTab.h \
   $$PWD/ExternalChargesSection.h \
   $$PWD/FreezingStringTab.h \
   $$PWD/FrequenciesTab.h \
   $$PWD/GeometryConstraint.h \
   $$PWD/InputDialog.h \
   $$PWD/KeywordSection.h \
   $$PWD/KeyValueSection.h \
   $$PWD/LJParametersSection.h \
   $$PWD/MoleculeSection.h \
   $$PWD/Node.h \
   $$PWD/Option.h \
   $$PWD/OptionDatabase.h \
   $$PWD/OptionDatabaseForm.h \
   $$PWD/OptionEditors.h \
   $$PWD/OptSection.h \
   $$PWD/PcmSection.h \
   $$PWD/PrimaryBasisTab.h \
   $$PWD/PropertiesTab.h \
   $$PWD/QuiJob.h \
   $$PWD/Qui.h \
   $$PWD/QuiMolecule.h \
   $$PWD/QtNode.h \
   $$PWD/ReactionPathTab.h \
   $$PWD/Register.h \
   $$PWD/RemSection.h \
   $$PWD/TransitionStateTab.h \


FORMS += \
   $$PWD/AdcTab.ui \
   $$PWD/AimdTab.ui \
   $$PWD/AttenuationParameterTab.ui \
   $$PWD/AuxiliaryBasisTab.ui \
   $$PWD/CisTab.ui \
   $$PWD/EomTab.ui \
   $$PWD/FreezingStringTab.ui \
   $$PWD/FrequenciesTab.ui \
   $$PWD/GeometryConstraintDialog.ui \
   $$PWD/OptionDatabaseForm.ui \
   $$PWD/OptionListEditor.ui \
   $$PWD/OptionNumberEditor.ui \
   $$PWD/PrimaryBasisTab.ui \
   $$PWD/PropertiesTab.ui \
   $$PWD/QuiMainWindow.ui \
   $$PWD/ReactionPathTab.ui \
   $$PWD/TransitionStateTab.ui \
