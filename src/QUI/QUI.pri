INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/..
DEFINES += QCHEM_UI

HEADERS += \
   $$PWD/OptionDatabaseForm.h \
   $$PWD/OptionEditors.h \
   $$PWD/InputDialog.h \
   $$PWD/Node.h \
   $$PWD/QtNode.h \
   $$PWD/Register.h \
   $$PWD/OptionDatabase.h \
   $$PWD/Conditions.h \
   $$PWD/Option.h \
   $$PWD/Actions.h \
   $$PWD/Qui.h \
   $$PWD/Job.h \
   $$PWD/FileDisplay.h \
   $$PWD/KeywordSection.h \
   $$PWD/RemSection.h \
   $$PWD/MoleculeSection.h \
   $$PWD/GeometryConstraint.h \
   $$PWD/OptSection.h \
   $$PWD/ExternalChargesSection.h \
   $$PWD/LJParametersSection.h \
   $$PWD/FindDialog.h \
   $$PWD/System.h \
   $$PWD/QuiMolecule.h \
   $$PWD/QuiReadFile.h
           
SOURCES += \
   $$PWD/OptionDatabaseForm.C \
   $$PWD/Option.C \
   $$PWD/OptionDatabase.C \
   $$PWD/OptionEditors.C \
   $$PWD/Conditions.C \
   $$PWD/Actions.C \
   $$PWD/InputDialogLogic.C \
   $$PWD/Job.C \
   $$PWD/Qui.C \
   $$PWD/FileDisplay.C \
   $$PWD/KeywordSection.C \
   $$PWD/ReadInput.C \
   $$PWD/RemSection.C \
   $$PWD/MoleculeSection.C \
   $$PWD/InputDialog.C \
   $$PWD/GeometryConstraint.C \
   $$PWD/OptSection.C \
   $$PWD/ExternalChargesSection.C \
   $$PWD/LJParametersSection.C \
   $$PWD/FindDialog.C \
   $$PWD/KillProcess.C \
   $$PWD/RunCommand.C \
   $$PWD/GetParentProcessChain.C \
   $$PWD/GetMatchingProcesses.C \
   $$PWD/IQmolInterface.C \
   $$PWD/QuiMolecule.C \
   $$PWD/QuiReadFile.C

FORMS += \
   $$PWD/OptionDatabaseForm.ui \
   $$PWD/OptionListEditor.ui \
   $$PWD/OptionNumberEditor.ui \
   $$PWD/FileDisplay.ui \
   $$PWD/QuiMainWindow.ui \
   $$PWD/GeometryConstraintDialog.ui \
   $$PWD/FindDialog.ui
