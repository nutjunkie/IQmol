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
            return m_configuration.connection() == ServerConfiguration::Local; 
         }

		 // these should be void, the connections should be done internally to
		 // the server and the job updated when the request returns
         virtual void test();
         virtual void setup(Job*);
         virtual void submit(Job*);
         virtual void query(Job*);
         virtual void kill(Job*);
         virtual void copy(Job*);

         void watchJob(Job*);
         void unwatchJob(Job*);
         void setUpdateInterval(int const seconds);

      protected:
         Server(ServerConfiguration const&);
         Server();
         ~Server();

         ServerConfiguration& configuration() { return m_configuration; }

      private Q_SLOTS:
         void queryAllJobs();

         void testFinished();
         void registerFinished();
         void setupFinished();
         void submitFinished();
         void queryFinished();
         void killFinished();
         void copyFinished();

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
