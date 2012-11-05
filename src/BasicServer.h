#ifndef IQMOL_BASICSERVER_H
#define IQMOL_BASICSERVER_H
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
#include "Process.h"
#include <QVariant>
#include <QQueue>


namespace IQmol {

   class Server;
   namespace ServerTask {
      class Base;
   }

   class BasicServer : public ServerDelegate {

      Q_OBJECT

      public:
         BasicServer(Server* server, QVariantMap const& defaults);
         void runQueue(); 
   
         void addToWatchList(Process*);
         Process::Status parseQueryString(QString const& query, Process*);
         bool configureJob(Process*) { return true; }
         QVariantMap delegateDefaults() const;
   
         ServerTask::Base* testConfiguration();
         ServerTask::Base* submit(Process*);
         ServerTask::Base* kill(Process*);
         ServerTask::Base* query(Process*);
         ServerTask::Base* cleanUp(Process*);
         ServerTask::CopyResults* copyResults(Process*);

      private Q_SLOTS:
         void submitFinished();

      private:
		 /// This is called by the in-built queue manager and actually submits the job
         /// to the host (as opposed to the queue).
         ServerTask::Base* submitToHost(Process*);

         // These processes have been submitted, but not yet started.
         QQueue<Process*> m_queue;

         int m_jobLimit;
   };

} // end namespace IQmol

#endif
