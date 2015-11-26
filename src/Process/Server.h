#ifndef IQMOL_PROCESS_SERVER_H
#define IQMOL_PROCESS_SERVER_H
/*******************************************************************************
         
  Copyright (C) 2011-2015 Andrew Gilbert
      
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

#include "ServerConfiguration.h"
#include "Connection.h"
#include "Job.h"
#include <QMap>

namespace IQmol {

namespace Network {
   class Connection;
   class Reply;
}

namespace Process2 {

   class Server : public QObject {

      Q_OBJECT

      friend class ServerRegistry;
      friend class ServerConfigurationListDialog;

      public:
         QString name() const;
         QStringList tableFields() const;

         void open();
         void closeConnection();

         bool isLocal() const { 
            return m_configuration.isLocal();
         }

         bool isWebBased() const { 
            return m_configuration.isWebBased();
         }

         bool needsResourceLimits() const { 
            return m_configuration.needsResourceLimits();
         }

         //ServerConfiguration const& configuration() const { return m_configuration; }
         ServerConfiguration& configuration() { return m_configuration; }

         // Unthreaded test for the existance of a directory on the server
         bool exists(QString const& directoryPath);

         // Unthreaded mkdir command
         bool makeDirectory(QString const& directoryPath);

         // Unthreaded rmdir command
         bool removeDirectory(QString const& directoryPath);

         // Unthreaded command to get queue information
         QString queueInfo();

         void submit(Job*);
         void query(Job*);
         void kill(Job*);
         void copyResults(Job*);

         void setUpdateInterval(int const seconds);
         void stopUpdates()  { m_updateTimer.stop(); }
         void startUpdates() { m_updateTimer.start(); }

      public Q_SLOTS:
         void watchJob(Job*);
         void unwatchJob(Job*);
         void cancelCopy(Job*);

      protected:
         Server(ServerConfiguration const&);
         Server();
         ~Server();

      private Q_SLOTS:
         void copyRunFile();
         void queueJob();
         void listFinished();
         void submitFinished();
         void queryFinished();
         void killFinished();
         void copyResultsFinished();
         void queryAllJobs();


      private:
         QString substituteMacros(QString const&);
         bool parseSubmitMessage(Job* job, QString const& message);
         bool parseQueryMessage(Job* job, QString const& message);
         QStringList parseListMessage(Job* job, QString const& message); 

         ServerConfiguration  m_configuration;
         Network::Connection* m_connection;

         // The Server class watches jobs, but is not responsible for them.
         QList<Job*> m_watchedJobs;
         QMap<Network::Reply*, Job*> m_activeRequests;

         QTimer m_updateTimer;
   };


   class BlockServerUpdates {
      public:
         BlockServerUpdates(Server* server) : m_server(server) { m_server->stopUpdates(); }
         ~BlockServerUpdates() { m_server->startUpdates(); }
      private:
         Server* m_server;
   };


} } // end namespace IQmol::Process

#endif
