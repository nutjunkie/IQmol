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

#include "Process.h"
#include "QMsgBox.h"
#include "QsLog.h"
#include "FileLayer.h"
#include "Preferences.h"
#include "Server.h"
#include "ServerRegistry.h"
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QProcess>


namespace IQmol {
namespace Process2 {

QString Process::toString(Status const& state) 
{
   QString s;

   switch (state) {
      case NotRunning:  s = "Not Running";  break;
      case Queued:      s = "Queued";       break;
      case Running:     s = "Running";      break;
      case Suspended:   s = "Suspended";    break;
      case Killed:      s = "Killed";       break;
      case Error:       s = "Error";        break;
      case Finished:    s = "Finished";     break;
      case Copying:     s = "Copying";      break;
      default:          s = "Unknown";      break;
   }
   return s;
}


Job::Job(QString const& jobName, QString const& serverName) : m_jobName(jobName),
   m_serverName(serverName), m_status(NotRunning)
{
}


Job::~Job()
{
  // Let the server know we don't exist any more.
qDebug() << "Let the server know we are finished";
   //Server* server = ServerRegistry::instance().get(serverName());
   //if (server) server->removeFromWatchList(this);
}


QVariant Process::toQVariant()
{
   QVariantMap map;

   map.insert("JobName",    m_jobName);  
   map.insert("ServerName", m_serverName);  
   map.insert("Message",    m_message);  
   map.insert("JobId",      m_jobId);  
   map.insert("SubmitTime", m_submitTime);  
   map.insert("RunTime",    runTime());  
   map.insert("Status",     (int)m_status);  

   return QVariant(map);
}


bool Job::fromQVariant(QVariant const& qvar)
{
   QVariantMap map(qvar.toMap());

   if (!map.contains("JobName")) return false;
   m_jobName = map.value("JobName").toString(); 
   if (m_jobName.isEmpty()) return false;

   if (!map.contains("ServerName")) return false;
   m_serverName = map.value("ServerName").toString(); 
   if (m_serverName.isEmpty()) return false;

   if (!map.contains("Status")) return false;
   m_status = static_cast<Status>(map.value("Status").toInt()); 

   if (map.contains("Message")) {
      m_message = map.value("Message").toString(); 
   }

   if (map.contains("SubmitTime")) {
      m_submitTime = map.value("SubmitTime").toString(); 
   }

   if (map.contains("JobId")) {
      m_jobId = map.value("JobId").toString(); 
   }

   if (map.contains("RunTime")) {
      m_runTime = map.value("RunTime").toString(); 
   }

   return true;
}



Process* Process::deserialize(QVariant const& qvariant)
{
   QList<QVariant> list(qvariant.toList());
   bool ok = (list.size() == 6)            &&
           //list[0] is the JobInfo which we test separately
             list[1].canConvert<int>()     &&
             list[2].canConvert<QString>() &&
             list[3].canConvert<QString>() &&
             list[4].canConvert<QString>() &&
             list[5].canConvert<QString>();

   if (!ok) {
      QLOG_ERROR() << "Process deserialization error: Invalid format";
      return 0;
   }

   JobInfo* jobInfo(JobInfo::deserialize(list[0]));
   if (!jobInfo) {
      QLOG_ERROR() << "Process deserialization error: Invalid JobInfo";
      return 0;
   }

   int s(list[1].toInt(&ok));
   Status status;
   if (0 <= s && s <= Unknown) {
      status = (Status)s; 
   }else {
      QLOG_ERROR() << "Process deserialization error: Invalid status";
      return 0;
   }

   // Now that the JobInfo has loaded, check we have a valid server
   QString serverName(jobInfo->get(JobInfo::ServerName));
   Server* server = ServerRegistry::instance().get(serverName);
   if (!server) {
      QLOG_ERROR() << "Process deserialization error: Server " << serverName << " not found ";
      return 0;
   }

   Process* process = new Process(jobInfo);
   process->m_comment    = list[2].toString();
   process->m_id         = list[3].toString();
   process->m_submitTime = list[4].toString();
   process->m_runTime    = list[5].toUInt();
   process->m_status     = status;

   process->m_timer.reset(process->m_runTime);

   return process;
}


void Process::resetTimer(int const seconds)
{ 
   m_timer.reset(seconds); 
}


unsigned Job::runTime() const 
{ 
   if (m_runTime) return m_runTime;
   return m_timer.time(); 
}


QStringList Process::monitorItems() const 
{
   QString status;
   QStringList items;
   items << name() << serverName() << m_submitTime;
   items <<  m_timer.toString();
   status = toString(m_status);
   if (m_status == Copying) {
      status += ": " + QString::number(int(100.0*m_copyProgress/m_copyTarget)) +"%";
   }
   items << status;
   if (!m_comment.isEmpty()) items << m_comment;
   return items;
}


void Process::setPID(unsigned int const pid)
{
   m_id = QString::number(pid);
   updated();
}


unsigned int Process::getPID() const
{
   bool ok;
   unsigned int pid(m_id.toUInt(&ok));
   if (!ok) pid = 0;
   return pid;
}


void Job::setStatus(Status const status)
{
   if (m_status == status) return;
   m_status = status;

   switch (m_status) {
      case NotRunning:
         break;

      case Queued:
         m_submitTime = QTime::currentTime().toString("h:mm:ss");
         break;

      case Running:
         m_timer.start();
         break;

      case Suspended:
         m_timer.stop();
         break;

      case Killed:
         m_timer.stop();
         break;

      case Error:
         m_timer.stop();
         break;

      case Finished:
         m_timer.stop();
         break;

      case Copying:
         QLOG_ERROR() << "Process::setStatus called while Copying";
         break;

      case Unknown:
         m_timer.stop();
         break;
   }

   updated();
}

} } // end namespaces IQmol::Process
