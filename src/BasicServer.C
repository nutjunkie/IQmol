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

#include "BasicServer.h"
#include "ServerTask.h"
#include "Server.h"
#include "QsLog.h"

#include <QDebug>


namespace IQmol {

BasicServer::BasicServer::BasicServer(Server* server, QVariantMap const& defaults)
 : ServerDelegate(server), m_jobLimit(INT_MAX) 
{ 
   if (defaults.contains("JobLimit")) {
      bool ok;
      int limit(defaults.value("JobLimit").toInt(&ok));
      if (ok) m_jobLimit = (limit == 0 ? INT_MAX : limit);
   }
}


QVariantMap BasicServer::delegateDefaults() const
{
   QVariantMap map;
   int limit(m_jobLimit == INT_MAX ? 0 : m_jobLimit);
   map.insert("JobLimit", limit);
   return map;
}


ServerTask::Base* BasicServer::testConfiguration()
{
qDebug() << "Testing BasicServer configuration";
   if (!m_server->isConnected()) {
      throw Server::Exception("Test Configuration called on disconnected server");
   }

   ServerTask::TestConfiguration* test = new ServerTask::TestConfiguration(m_server);

   HostDelegate::FileFlags flags;
   QString file;

   //flags = HostDelegate::Directory | HostDelegate::Readable;
   //file  = m_server->qchemEnvironment();
   //test->addFileTest(file, flags);

   flags = HostDelegate::Directory | HostDelegate::Writable;
   file  = m_server->workingDirectory();
   if (!file.isEmpty()) test->addFileTest(file, flags);
   
#ifndef Q_WS_WIN
//   flags = HostDelegate::Executable;
//   file  = m_server->qchemEnvironment() + "/exe/" + m_server->executableName();
//   test->addFileTest(file, flags);
#endif

   // These may not be required if we just assume the remote server is *nix based
   // also use: test  at  ps  rm  mkdir  grep  cd  sleep  awk  tail
   if (m_server->host() == Server::Remote) {
      flags = HostDelegate::Executable;
      file  = "nohup";
      test->addFileTest(file, flags);
      file  = "grep";
      test->addFileTest(file, flags);
      file  = "sleep";
      test->addFileTest(file, flags);
   }

   return test;
}


ServerTask::Base* BasicServer::submit(Process* process)
{
   QString msg(process->jobInfo()->get(JobInfo::BaseName));
   msg += " submitted to server ";
   msg += m_server->name();
   QLOG_INFO() << msg;

   m_queue.enqueue(process);
   process->setStatus(Process::Queued);
   runQueue();
   
   return new ServerTask::DoNothing(m_server, msg);
}


void BasicServer::addToWatchList(Process* process)
{
   if (process->status() == Process::Queued) {
      m_queue.enqueue(process);
   }else {
      m_server->m_watchList.append(process);
   }
}

   
void BasicServer::runQueue()
{  
   if (!isConnected()) {
      QLOG_DEBUG() << "BasicServer::runQueue server not connected";
      return;
   }
   int running(m_server->watchedWithStatus(Process::Running));

   qDebug() << "runQueue() jobLimit:" << m_jobLimit;
   qDebug() << "runQueue() queued:  " << m_queue.size();
   qDebug() << "runQueue() running: " << running;

   while (!m_queue.isEmpty() && running < m_jobLimit) {
      Process* process(m_queue.dequeue());
      ServerTask::Base* task = submitToHost(process);
      if (task) {
         connect(task, SIGNAL(finished()), this, SLOT(submitFinished()));
         task->start();
         ++running;
      }
   }
}


void BasicServer::submitFinished()
{
   ServerTask::Base* task(qobject_cast<ServerTask::Base*>(sender()));
   if (task) task->deleteLater();
}


ServerTask::Base* BasicServer::submitToHost(Process* process)
{
   return new ServerTask::BasicSubmit(m_server, process);
}


ServerTask::Base* BasicServer::query(Process* process)
{
   ServerTask::Base* task(0);

   if (m_queue.contains(process)){
      if (process->status() == Process::Queued) {
         QString msg("Process " + QString::number(m_queue.indexOf(process)+1)
                    + " of " + QString::number(m_queue.size()) + " in queue");
        task = new ServerTask::DoNothing(m_server, msg);
      }else {
         QLOG_WARN() << "Unqueued process found in queue";
      }
   }else {
      task = new ServerTask::Query(m_server, process);
   }
   return task;
}


Process::Status BasicServer::parseQueryString(QString const& query, Process* process)
{
//   qDebug() <<"BasicServer::parseQueryString handed" << query;
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

} // end namespace IQmol
