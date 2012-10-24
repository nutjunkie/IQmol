#ifndef IQMOL_PBSSERVER_H
#define IQMOL_PBSSERVER_H
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

#include "ServerDelegate.h"
#include "PBSQueue.h"
#include "Process.h"


namespace IQmol {

   class Server;
   class PBSConfigurator;
   namespace ServerTask {
      class Base;
   }

   class PBSServer : public ServerDelegate {

      Q_OBJECT

      public:
         PBSServer(Server* server, QVariantMap const& defaults);
         ~PBSServer();

         Process::Status parseQueryString(QString const& query, Process*);
         bool configureJob(Process*);
         void setQueues(QueueList const& queues) { m_queues = queues; }
         QVariantMap delegateDefaults() const { return m_defaults; }
         
         ServerTask::Base* testConfiguration();
         ServerTask::Base* submit(Process*);
         ServerTask::Base* kill(Process*);
         ServerTask::Base* query(Process*);
         ServerTask::Base* cleanUp(Process*);
         ServerTask::CopyResults* copyResults(Process*);

      private:
         QueueList m_queues;
         PBSConfigurator* m_configurator;
         QVariantMap m_defaults;
   };

} // end namespace IQmol

#endif
