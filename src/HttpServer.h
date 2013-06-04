#ifndef IQMOL_HTTPSERVER_H
#define IQMOL_HTTPSERVER_H
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

   class HttpServer : public ServerDelegate {

      Q_OBJECT

      public:
         HttpServer(Server* server, QVariantMap const& defaults);
         ~HttpServer() { }
   
         Process::Status parseQueryString(QString const& query, Process*);
         bool configureJob(Process*) { return true; }
         void parseLimits(QString const&);
         QVariantMap delegateDefaults() const { return m_defaults; }
   
         ServerTask::Base* setup(Process* process);
         ServerTask::Base* testConfiguration();
         ServerTask::Base* submit(Process*);
         ServerTask::Base* kill(Process*);
         ServerTask::Base* query(Process*);
         ServerTask::Base* cleanUp(Process*);
         ServerTask::CopyResults* copyResults(Process*);

      private:
         //Limits on job types
         QVariantMap m_defaults;
         QMap<QString,QString> m_limits;
   };

} // end namespace IQmol

#endif
