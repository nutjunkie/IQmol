LIB = Process
CONFIG += lib
include(../common.pri)

INCLUDEPATH += ../Util ../Yaml ../Data  ../Parser ../Network

SOURCES = \
   $$PWD/Job.C \
   $$PWD/JobMonitor.C \
   $$PWD/QChemJobInfo.C \
   $$PWD/QueueOptionsDialog.C \
   $$PWD/QueueResources.C \
   $$PWD/QueueResourcesDialog.C \
   $$PWD/Server2.C \
   $$PWD/ServerConfiguration.C \
   $$PWD/ServerConfigurationDialog.C \
   $$PWD/ServerConfigurationListDialog.C \
   $$PWD/ServerRegistry2.C \
   $$PWD/SshFileDialog.C \
   $$PWD/SystemDependent.C \

HEADERS = \
   $$PWD/Job.h \
   $$PWD/JobMonitor.h \
   $$PWD/QChemJobInfo.h \
   $$PWD/QueueOptionsDialog.h \
   $$PWD/QueueResources.h \
   $$PWD/QueueResourcesDialog.h \
   $$PWD/Server2.h \
   $$PWD/ServerConfiguration.h \
   $$PWD/ServerConfigurationDialog.h \
   $$PWD/ServerConfigurationListDialog.h \
   $$PWD/ServerRegistry2.h \
   $$PWD/SshFileDialog.h \
   $$PWD/SystemDependent.h \

FORMS = \
   $$PWD/JobMonitor.ui \
   $$PWD/QueueOptionsDialog.ui \
   $$PWD/QueueResourcesDialog.ui \
   $$PWD/ServerConfigurationDialog.ui \
   $$PWD/ServerConfigurationListDialog.ui \
   $$PWD/SshFileDialog.ui \
