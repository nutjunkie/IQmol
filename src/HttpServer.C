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

#include "HttpServer.h"
#include "ServerTask.h"
#include "Server.h"
#include "QsLog.h"

#include <QDebug>


namespace IQmol {

HttpServer::HttpServer(Server* server, QVariantMap const& defaults)
 : ServerDelegate(server), m_defaults(defaults)
{ 
}


void HttpServer::parseLimits(QString const& limits)
{
   QStringList lines(limits.split(QRegExp("\\s+"), QString::SkipEmptyParts));
   QStringList::iterator iter;
   for (iter = lines.begin(); iter != lines.end(); ++iter) {
       QStringList tokens((*iter).split("=", QString::SkipEmptyParts));
       if (tokens.size() >= 2) {
          QString key(tokens.takeFirst());
          QString value = (tokens.size() == 1) ? tokens.first() : tokens.join("=");
          m_limits.insert(key, value);
       }else {
          qDebug() << "Invalid limit format:" << *iter;
       }
   }
}


ServerTask::Base* HttpServer::testConfiguration()
{
   qDebug() << "Testing HttpServer configuration";
   if (!m_server->isConnected()) {
      throw Server::Exception("Test Configuration called on disconnected server");
   }

   return new ServerTask::TestHttpConfiguration(m_server);
}


ServerTask::Base* HttpServer::setup(Process* process)
{
   // test limts here?
   return new ServerTask::SetupHttp(m_server, process);
}


ServerTask::Base* HttpServer::submit(Process* process)
{
   QString msg(process->jobInfo()->get(JobInfo::BaseName));
   msg += " submitted to server ";
   msg += m_server->name();
   QLOG_INFO() << msg;
   process->setStatus(Process::Queued);
   
   return new ServerTask::DoNothing(m_server, msg);
}


ServerTask::Base* HttpServer::kill(Process* process)
{
   return new ServerTask::KillProcess(m_server, process);
}


ServerTask::Base* HttpServer::query(Process* process)
{
   return new ServerTask::Query(m_server, process);
}


Process::Status HttpServer::parseQueryString(QString const& query, Process* process)
{
   // qDebug() << "HttpServer::parseQueryString handed" << query;
   QStringList lines(query.split(QRegExp("\\n"), QString::SkipEmptyParts));
   Process::Status status(Process::Unknown);

   QString id(process->id());
   id = " " + id + " ";
   QStringList::iterator line;
   for (line = lines.begin(); line != lines.end(); ++line) {
       if ((*line).contains(m_server->executableName()) && (*line).contains(id)) {
          QString time((*line).split(QRegExp("\\s+"), QString::SkipEmptyParts).last());
          process->resetTimer(Timer::toSeconds(time));
          status = Process::Running;
       }
   }
   
   if (query.isEmpty()) return Process::Unknown;

   return status;
}


ServerTask::Base* HttpServer::cleanUp(Process* process)
{
   return new ServerTask::CleanUp(m_server, process);
}


ServerTask::CopyResults* HttpServer::copyResults(Process* process)
{
   return new ServerTask::CopyResults(m_server, process);
}


} // end namespace IQmol
