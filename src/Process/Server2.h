#ifndef IQMOL_PROCESS_SERVER_H
#define IQMOL_PROCESS_SERVER_H
/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
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

         bool open();

         bool isLocal() const { 
            return m_configuration.isLocal();
         }

         bool isWebBased() const { 
            return m_configuration.isWebBased();
         }

         //ServerConfiguration const& configuration() const { return m_configuration; }
         ServerConfiguration& configuration() { return m_configuration; }

         // Unthreaded test for the existance of a directory on the server
         bool exists(QString const& directoryPath);

         // Unthreaded mkdir command
         bool makeDirectory(QString const& directoryPath);

         void submit(Job*);

         void query(Job*);
         void kill(Job*);
         void copy(Job*);

         void watchJob(Job*);
         void setUpdateInterval(int const seconds);

      public Q_SLOTS:
         void unwatchJob(Job*);

      Q_SIGNALS:
         void jobSubmissionSuccessful(Job*);
         void jobSubmissionFailed(Job*);

      protected:
         Server(ServerConfiguration const&);
         Server();
         ~Server();


      private Q_SLOTS:
         void copyRunFile();
         void queueJob();

         void submitFinished();
         void queryFinished();
         void killFinished();
         void copyFinished();

         void queryAllJobs();

      private:
        QString substituteMacros(QString const&);

        ServerConfiguration  m_configuration;
        Network::Connection* m_connection;

        // The Server class watches jobs, but is not responsible for them.
        QList<Job*> m_watchedJobs;
        QMap<Network::Reply*, Job*> m_activeRequests;

        QTimer m_updateTimer;
   };

} } // end namespace IQmol::Process

#endif
