#ifndef IQMOL_SERVER_H
#define IQMOL_SERVER_H
/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
         
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software  
  Foundation, either version 3 of the License, or (at your option) any later  
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.
   
********************************************************************************/

#include "QsLog.h"
#include "Process.h"
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <exception>


namespace IQmol {

   class JobInfo;
   class ServerDelegate;
   namespace ServerTask {
      class Base;
      class CopyResults;
   }

   /// Interface class to Server on which jobs can be run.   Note that any
   /// functionality that depends on the type of server is passed on to the
   /// ServerDelegate.
   class Server : public QObject {

      Q_OBJECT

      friend class ServerRegistry;
      friend class ServerDialog;
      friend class ServerOptionsDialog;
      friend class ServerDelegate;
      friend class HostDelegate;
      friend class ServerTask::Base;

      friend class BasicServer;

      public:
         enum Type { Basic = 0, PBS, Custom };
         enum Host { Local = 0, Remote };
         enum Authentication { None = 0, Agent, PublicKey, HostBased, KeyboardInteractive,
           Vault, Prompt   };
           

         /// These are not threaded as they are (potentially) interactive
         bool connectServer();
         void disconnectServer();
         bool isConnected();

		 /// Returns true if the user provides a potential candidate for the
		 /// working directory.  For remote servers we must still check that the
		 /// directory does not exists (or at least if it does can we overwrite
		 /// it).
         bool getWorkingDirectoryFromUser(Process*);

		 /// Offers the user a chance to configure the job, e.g. setup PBS.
		 /// Returns false if the job is cancelled.
         bool configureJob(Process*);

		 /// Takes the input string and substitutes the macros with appropriate
		 /// values based on the given process.  Note this should be the only
		 /// place that the macro substitution occurs.
         QString replaceMacros(QString const& input, Process*);

         void startTimer();

		 /// These are all high level commands that form the functional
		 /// interface to the Server.  They return Threaded objects that
         /// actually do the work in a separate thread to avoid potential
         /// hanging of the GUI.  Note that the ServerDelegate is actually 
         /// responsible for creating the threaded action as the exact
         /// behaviour depends on on the Server Type.
         ServerTask::Base* testConfiguration();
         ServerTask::Base* setup(Process* process);
         ServerTask::Base* submit(Process* process);
         ServerTask::Base* kill(Process* process);
         ServerTask::Base* query(Process* process);
         ServerTask::Base* cleanUp(Process* process);
         ServerTask::Base* copyResults(Process* process);

         int  jobLimit() const;
         void setJobLimit(int const);

         QString name()                  const { return m_name; }
         Host host()                     const { return m_host; }
         QString hostAddress()           const { return m_hostAddress; }
         QString userName()              const { return m_userName; }

         /// These should be revisited to make sure they are actually needed
         Type type()                     const { return m_type; }
         QString qchemEnvironment()      const { return m_qchemEnvironment; }
         Authentication authentication() const { return m_authentication; }
         int port()                      const { return m_port; }
         QString workingDirectory()      const { return m_workingDirectory; }
         QString executableName()        const { return m_executableName; }
         QString submitCommand()         const { return m_submitCommand; }
         QString queryCommand()          const { return m_queryCommand; }
         QString queueInfo()             const { return m_queueInfo; }
         QString killCommand()           const { return m_killCommand; }
         QString runFileTemplate()       const { return m_runFileTemplate; }
         int updateInterval()            const { return m_updateInterval; }

		 /// This function can be used to add a recreated process to the
		 /// servers's watch list rather than submitting it.  This is useful 
		 /// when recreating the ProcessMonitor table after restarting IQmol.
         void addToWatchList(Process*);

		 /// Removes the Process from the list of processes that are polled at
         /// regular intervals to determine their status.
         void removeFromWatchList(Process*);

		 /// Returns the number of processes in the watch list with the given
		 /// status.  Note that this returns the cached status, avoiding any
         /// potential network activity.  If you want the updated status, then 
         /// call updateProcesses() first.
         int watchedWithStatus(Process::Status const);

         /// Provides a list of fields for the ServerListDialog
         QStringList tableFields() const;


      Q_SIGNALS:
         void sendStatusMessage(QString const&);
         void resultsAvailable(JobInfo*);


      public Q_SLOTS:
         void updateProcesses();


      protected:
		 // The ctors are protected as all Servers should be obtained through
		 // the ServerRegistry to ensure there is only one Server per physical
		 // machine.
         Server();
         ~Server();

         void testedAndValid(bool tf) { m_testedAndValid = tf; }

         // Sets default values appropriate for the given Host and Type
         void setDefaults(Host const, Type const);

         /// A Server is basically just a POD class until this has been called.
         void setDelegate(Host const, Type const);

		 /// Decodes the data fields from a QVariant read from the Preferences
         void fromQVariant(QVariant const&);

		 /// Encodes the data fields for writing to the Preferences
         QVariant toQVariant();

         ServerDelegate* m_serverDelegate;

 		 /// This is a list of active processes that should be polled
		 /// periodically to check their status.  Note that for Local
         /// Servers this may not include all the active processes as 
         /// they are submitted under a QProcess and do not have to be
         /// watched explicitly.
         QList<Process*> m_watchList;
        
		 /// The following fields are stored as a QVariantMap in the 
         /// Preferences so the Server can be reloaded.  Note the name 
         /// is just a moniker and does not have to match the actual
         ///  machine name.
         QString m_name;
         Host m_host;
         Type m_type;
         QString m_qchemEnvironment;
         QString m_hostAddress;
         QString m_userName;
         Authentication m_authentication;
         int m_port;
         QString m_workingDirectory;
         QString m_executableName;
         QString m_submitCommand;
         QString m_queryCommand;
         QString m_queueInfo;
         QString m_killCommand;
         QString m_runFileTemplate;
         int m_updateInterval;
         QVariantMap m_delegateDefaults;


      private Q_SLOTS:
         void queryFinished();
         void cleanUpFinished();


      private:
         static QString toString(Type const&);
         static QString toString(Host const&);
         static QString toString(Authentication const&);

         void setUpdateInterval(int const seconds);
         void setLocalDefaults();
         void setRemoteDefaults();
         void setBasicDefaults(Host const);
         void setPBSDefaults(Host const);
         void setCustomDefaults(Host const);
         
         QTimer m_updateTimer;  // Triggers update for active processes.
         bool m_testedAndValid;


      public:
         class Exception : public std::exception {

            public:
               Exception(QString const& server, QString const& message) {
                  QString msg("Server error on ");
                  msg += server + "\n" + message;
                  setMessage(msg);
               }

               Exception(QString const& message) {
                  setMessage(message);
               }
 
               ~Exception() throw () { }

               virtual const char* what() const throw() {
                  return m_message.toLatin1();
               }

            private:
               void setMessage(QString const& message) {
                  m_message = message;
                  QLOG_ERROR() << m_message;
               }
               QString m_message;
         };

   };

} // end namespace IQmol


#endif
