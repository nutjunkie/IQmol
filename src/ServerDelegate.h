#ifndef IQMOL_SERVERDELEGATE_H
#define IQMOL_SERVERDELEGATE_H
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

#include "Server.h"
#include "LocalHost.h"
#include "RemoteHost.h"
#include "ServerTask.h"
#include "Process.h"
#include <QObject>


namespace IQmol {

   /// The ServerDelegate class provides a level of indirection for the
   /// different types of Server available (Basic, Custom, PBS etc.).  This
   /// allows the Server class, which forms the interface for Servers, to
   /// deal with high-level actions such as submit and kill while the 
   /// ServerDelegate actually takes care of the details (e.g. do we use
   /// qsub or some other command.

   class ServerDelegate : public QObject {

      Q_OBJECT

      friend class ServerTask::Base;

      public:
         ServerDelegate(Server* server) : m_server(server) { }
         virtual ~ServerDelegate() { }

         void setHostDelegate(Server::Host const host) {
            switch (host) {
               case Server::Local:
                  m_hostDelegate = new LocalHost(m_server);
                  break;
               case Server::Remote:
                  m_hostDelegate = new RemoteHost(m_server);
                  break;
            }
         }

         bool connectServer() {
            return m_hostDelegate->connectServer();
         }

         void disconnectServer() {
            return m_hostDelegate->disconnectServer();
         }

         bool isConnected() {
            bool connected(false);
            if (m_hostDelegate) connected = m_hostDelegate->isConnected();
            return connected;
         }

         bool getWorkingDirectoryFromUser(Process* process) {
            return m_hostDelegate->getWorkingDirectoryFromUser(process->jobInfo());
         }

         virtual void addToWatchList(Process* process) {
            m_server->m_watchList.append(process);
         }

         virtual Process::Status parseQueryString(QString const& query, Process*) = 0;
         virtual bool configureJob(Process*) = 0;
         virtual QVariantMap delegateDefaults() const = 0;

         virtual ServerTask::Base* testConfiguration() = 0;
         virtual ServerTask::Base* submit(Process*) = 0;
         virtual ServerTask::Base* kill(Process*) = 0;
         virtual ServerTask::Base* query(Process*) = 0;
         virtual ServerTask::Base* cleanUp(Process*) = 0;
         virtual ServerTask::CopyResults* copyResults(Process*) = 0;

         virtual ServerTask::Base* setup(Process* process) {
            return new ServerTask::Setup(m_server, process);
         }

		 // Convenience function that returns a threaded no-op with optional
		 // message.
         ServerTask::Base* doNothing(QString const& msg = QString()) 
         {
            ServerTask::Base* task(new ServerTask::Base(m_server));
            task->setOutputMessage(msg);
            return task;
         }

      Q_SIGNALS:
         void delegateDefaultsUpdated();

      protected:
         Server* m_server;
         HostDelegate* m_hostDelegate;
   };


} // end namespace IQmol

#endif
