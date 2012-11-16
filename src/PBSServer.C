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

#include "PBSServer.h"
#include "ServerTask.h"
#include "ServerRegistry.h"
#include "Server.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include "PBSConfigurator.h"

#include <QDebug>


namespace IQmol {

PBSServer::PBSServer(Server* server, QVariantMap const& defaults) : ServerDelegate(server), 
   m_configurator(0), m_defaults(defaults)
{ 
}


PBSServer::~PBSServer()
{
   if (m_configurator) delete m_configurator;
}


ServerTask::Base* PBSServer::testConfiguration()
{
qDebug() << "Testing PBSServer configuration";
   if (!m_server->isConnected()) {
      throw Server::Exception("Test Configuration called on disconnected server");
   }

   ServerTask::TestConfiguration* test = new ServerTask::TestConfiguration(m_server);

   HostDelegate::FileFlags flags;
   QString file;

   flags = HostDelegate::Directory | HostDelegate::Readable;
   file  = m_server->qchemEnvironment();
   test->addFileTest(file, flags);

   flags = HostDelegate::Directory | HostDelegate::Writable;
   file  = m_server->workingDirectory();
   test->addFileTest(file, flags);
   
   flags = HostDelegate::Executable;
   file  = m_server->qchemEnvironment() + "/exe/" + m_server->executableName();
   test->addFileTest(file, flags);

   // The following are not tested as each test requires a separate test which
   // takes time.  We just assume the server is *nix based and they exist:
   // qstat  test  at  ps  rm  mkdir  grep  cd  sleep  awk  tail

   return test;
}


ServerTask::Base* PBSServer::submit(Process* process)
{
   return new ServerTask::PBSSubmit(m_server, process);
}

   
ServerTask::Base* PBSServer::kill(Process* process)
{
   return new ServerTask::KillProcess(m_server, process);
}


ServerTask::Base* PBSServer::query(Process* process)
{
   return new ServerTask::Query(m_server, process);
}


ServerTask::Base* PBSServer::cleanUp(Process* process)
{
   return new ServerTask::CleanUp(m_server, process);
}


ServerTask::CopyResults* PBSServer::copyResults(Process* process)
{
   return new ServerTask::CopyResults(m_server, process);
}


Process::Status PBSServer::parseQueryString(QString const& query, Process* process)
{
   if (query.isEmpty()) return Process::Unknown;

   QStringList lines(query.split(QRegExp("\\n"), QString::SkipEmptyParts));
   QStringList tokens;
   QStringList::iterator iter;
   Process::Status status(Process::Unknown);

   for (iter = lines.begin(); iter != lines.end(); ++iter) {
       //job_state = R
       if ((*iter).contains("job_state =")) {
          tokens = (*iter).split(QRegExp("\\s+"), QString::SkipEmptyParts);
          if (tokens.size() >= 3) {
             if (tokens[2] == "R" || tokens[2] == "E") {
                status = Process::Running;
             }else if (tokens[2] == "S" || tokens[2] == "H") {
                status = Process::Suspended;
             }else if (tokens[2] == "Q" || tokens[2] == "W") {
                status = Process::Queued;
             }
          }
       }

       if ((*iter).contains("resources_used.walltime =")) {
          QString time((*iter).split(QRegExp("\\s+"), QString::SkipEmptyParts).last());
          process->resetTimer(Timer::toSeconds(time));
       }else if ((*iter).contains("comment =")) {
          process->setComment((*iter).remove("comment = ").trimmed());
       }
   }

   return status;
}


bool PBSServer::configureJob(Process* process)
{

   if (m_queues.isEmpty()) {
      QMsgBox::warning(0, "IQmol", "No PBS queues found");
      return false;
   }

   if (!m_configurator) {
       m_configurator = new PBSConfigurator(m_queues, m_defaults);
   }

   bool accepted(false);

   if (m_configurator->exec() == QDialog::Accepted) {
      JobInfo* jobInfo(process->jobInfo());
      jobInfo->set(JobInfo::Queue, m_configurator->queue()); 
      jobInfo->set(JobInfo::Walltime, m_configurator->walltime()); 
      jobInfo->set(JobInfo::Memory, m_configurator->memory()); 
      jobInfo->set(JobInfo::Jobfs, m_configurator->jobfs()); 
      jobInfo->set(JobInfo::Ncpus, m_configurator->ncpus()); 

      if (m_configurator->saveAsDefaults()) {
         m_defaults.insert("Queue", m_configurator->queue());
         m_defaults.insert("Walltime", m_configurator->walltime());
         m_defaults.insert("Memory", m_configurator->memory());
         m_defaults.insert("Jobfs", m_configurator->jobfs());
         m_defaults.insert("Ncpus", m_configurator->ncpus());
         ServerRegistry::instance().saveToPreferences();
      }
      accepted = true;
   }

   return accepted;
}


} // end namespace IQmol
