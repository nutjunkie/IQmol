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

#include "SGEServer.h"
#include "ServerTask.h"
#include "ServerRegistry.h"
#include "ServerQueueDialog.h"
#include "Server.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include "TextStream.h"


namespace IQmol {

SGEServer::SGEServer(Server* server, QVariantMap const& defaults) : ServerDelegate(server), 
   m_dialog(0), m_defaults(defaults)
{ 
}


SGEServer::~SGEServer()
{
   if (m_dialog) delete m_dialog;
}


ServerTask::Base* SGEServer::testConfiguration()
{
   QLOG_TRACE() << "Testing SGEServer configuration";
   if (!m_server->isConnected()) {
      throw Server::Exception("Test Configuration called on disconnected server");
   }

   ServerTask::TestConfiguration* test = new ServerTask::TestSGEConfiguration(m_server);

   HostDelegate::FileFlags flags;
   QString file;

   flags = HostDelegate::Directory | HostDelegate::Writable;
   file  = m_server->workingDirectory();
   test->addFileTest(file, flags);
   
   //flags = HostDelegate::Directory | HostDelegate::Readable;
   //file  = m_server->qchemEnvironment();
   //test->addFileTest(file, flags);

   //flags = HostDelegate::Executable;
   //file  = m_server->qchemEnvironment() + "/exe/" + m_server->executableName();
   //test->addFileTest(file, flags);

   // The following are not tested as each test requires a separate test which
   // takes time.  We just assume the server is *nix based and they exist:
   // qstat  test  at  ps  rm  mkdir  grep  cd  sleep  awk  tail

   return test;
}


ServerTask::Base* SGEServer::submit(Process* process)
{
   return new ServerTask::SGESubmit(m_server, process);
}


Process::Status SGEServer::parseQueryString(QString const& query, Process* process)
{
   Process::Status status(Process::Unknown);
   if (query.isEmpty() || query.contains("not exist")) return status;

   int nTokens;
   QString input(query);
   QStringList tokens;
   Parser::TextStream textStream(&input);

   while (!textStream.atEnd()) {
      tokens = textStream.nextLineAsTokens();
      nTokens = tokens.size();
      
      if (nTokens >= 5 && tokens.first().contains(process->id())) {
         QString s(tokens[4]);
         if (s.contains("q", Qt::CaseSensitive)) {
            status = Process::Queued; 
         }else if (s.contains("s", Qt::CaseInsensitive)) {
            status = Process::Suspended; 
         }else if (s.contains("r", Qt::CaseSensitive)) {
            status = Process::Running; 
         }
      }else if (nTokens > 1 && tokens.first().contains("usage"), Qt::CaseInsensitive) {
         QRegExp rx("cpu=([\\d:]+)");
         for (int i = 1; i < tokens.size(); ++i) {
             if (rx.indexIn(tokens[i]) >=0) {
                qDebug() << "RegExp matched:" << rx.cap(1);
                process->resetTimer(Timer::toSeconds(rx.cap(1)));
                break;
             }
         }
      }
   }

   return status;
}


bool SGEServer::configureJob(Process* process)
{
   if (m_queues.isEmpty()) {
      QMsgBox::warning(0, "IQmol", "No queues found");
      return false;
   }

   if (!m_dialog) m_dialog = new ServerQueueDialog(m_queues, m_defaults);
   if (m_dialog->exec() == QDialog::Rejected) return false; 

   JobInfo* jobInfo(process->jobInfo());
   jobInfo->set(JobInfo::Queue, m_dialog->queue()); 
   jobInfo->set(JobInfo::Walltime, m_dialog->walltime()); 
   jobInfo->set(JobInfo::Memory, m_dialog->memory()); 
   jobInfo->set(JobInfo::Scratch, m_dialog->scratch()); 
   jobInfo->set(JobInfo::Ncpus, m_dialog->ncpus()); 

   if (m_dialog->saveAsDefaults()) {
      m_defaults.insert("Queue", m_dialog->queue());
      m_defaults.insert("Walltime", m_dialog->walltime());
      m_defaults.insert("Memory", m_dialog->memory());
      m_defaults.insert("Scratch", m_dialog->scratch());
      m_defaults.insert("Ncpus", m_dialog->ncpus());
      ServerRegistry::instance().saveToPreferences();
   }

   return true;
}


// I can't figure out how to get much information from the SGE qstat, so we
// just settle on a list of queue names and rely on the default limits given
// in ServerQueue.h
int SGEServer::setQueuesFromQueueInfo(QString const& queueInfo)
{
   m_queues.clear();
   QString info(queueInfo);
   QStringList tokens;
   Parser::TextStream textStream(&info);

   // The list of queues comes after the header line
   textStream.seek("--------------------");

   while (!textStream.atEnd()) {
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() > 1) m_queues.append(new ServerQueue(tokens.first()));
   }

   return m_queues.size();
}

} // end namespace IQmol
