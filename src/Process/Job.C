/*******************************************************************************
       
  Copyright (C) 2011-2015 Andrew Gilbert
           
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

#include "Job.h"
#include "QsLog.h"
#include <QVariant>


namespace IQmol {
namespace Process {

QString Job::toString(Status const& status) 
{
   QString s;

   switch (status) {
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


Job::Job(QChemJobInfo const& qchemJobInfo) : m_qchemJobInfo(qchemJobInfo)
{
   m_julianDay  = QDate::currentDate().toJulianDay();
   m_jobName    = m_qchemJobInfo.baseName();
   m_serverName = m_qchemJobInfo.serverName();
   m_status     = NotRunning;
}


Job::~Job()
{
  // Let the server know we don't exist any more.
  deleted(this);
}


QVariant Job::toQVariant() const
{
   QVariantMap map;

   map.insert("JobName",      m_jobName);  
   map.insert("ServerName",   m_serverName);  
   map.insert("Message",      m_message);  
   map.insert("JobId",        m_jobId);  
   map.insert("SubmitTime",   m_submitTime);  
   map.insert("RunTime",      runTime());  
   map.insert("Status",       (int)m_status);  
   map.insert("JulianDay",    m_julianDay);
   map.insert("QChemJobInfo", m_qchemJobInfo.toQVariantList());

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
      resetTimer(map.value("RunTime").toUInt()); 
   }

   if (map.contains("JulianDay")) {
      m_julianDay = map.value("JulianDay").toUInt(); 
   }

   if (map.contains("QChemJobInfo")) {
      if (!m_qchemJobInfo.fromQVariantList(map.value("QChemJobInfo").toList())) {
         QLOG_WARN() << "Faild to convert QChemJobInfo from serialized Job";
      }
   }

   return true;
}


void Job::resetTimer(unsigned const seconds)
{ 
   m_timer.reset(seconds); 
}


unsigned Job::runTime() const 
{ 
   return m_timer.time(); 
}


QString Job::substituteMacros(QString const& input) const
{
   QString output(input);
   output.replace("${JOB_ID}",   m_jobId);
   output.replace("${JOB_DIR}",  m_qchemJobInfo.get(QChemJobInfo::RemoteWorkingDirectory));
   output.replace("${JOB_NAME}", m_qchemJobInfo.baseName());

   output.replace("${QUEUE}",    m_qchemJobInfo.queueName());
   output.replace("${WALLTIME}", m_qchemJobInfo.wallTime());
   output.replace("${MEMORY}",   QString::number(m_qchemJobInfo.memory()));
   output.replace("${JOBFS}",    QString::number(m_qchemJobInfo.scratch()));
   output.replace("${SCRATCH}",  QString::number(m_qchemJobInfo.scratch()));
   output.replace("${NCPUS}",    QString::number(m_qchemJobInfo.ncpus()));

   // Check all the macros have been expanded
   if (output.contains("${")) {
      QLOG_WARN() << "Unmatched macros found in string:";
      QLOG_WARN() << input;
   }

   return output;
}


void Job::parseQueryOutput(QString const&)
{
   qDebug() << "Need to parse query string";
}


/*
void Job::copyProgress()
{
   m_copyProgress += ".";
   if (m_copyProgress == "Copying....") m_copyProgress = "Copying";
qDebug() << "CopyProgress string:" << m_copyProgress;
   updated();
}
*/


void Job::copyProgress(double fraction)
{
   int pc(100*fraction);
   m_copyProgress = "Copying (" + QString::number(pc) + "%)";
   updated();
}


void Job::setStatus(Status const status, QString const& message)
{
   if (m_status == status) {
      if (!message.isEmpty()) m_message = message;
      updated();
      return;
   }
   m_status = status;

   switch (m_status) {
      case NotRunning:
         m_message = "Job has not yet started";
         break;

      case Queued:
         m_message = "Job ID: " + jobId();
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
         error();
         break;

      case Finished:
         m_timer.stop();
         finished();
         break;

      case Copying:
         m_copyProgress = "Copying";
         break;

      case Unknown:
         m_timer.stop();
         break;
   }

   if (!message.isEmpty()) m_message = message;
   qDebug() << "Setting Job status to" << toString(m_status) << m_message;
   updated();
}


bool Job::isActive(Status const status)
{
   bool active(true);

   switch (status) {
      case NotRunning:
      case Queued:
      case Running:
      case Suspended:
      case Unknown:
      case Copying:
         active = true;
         break;

      case Killed:
      case Error:
      case Finished:
         active = false;
         break;
   }

   return active;
}


bool Job::localFilesExist() const 
{ 
   return m_qchemJobInfo.localFilesExist();
}

} } // end namespaces IQmol::Process
