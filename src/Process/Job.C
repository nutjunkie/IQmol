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

#include "Job.h"
#include "QsLog.h"
#include <QVariant>


namespace IQmol {
namespace Process2 {

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
   m_jobName    = m_qchemJobInfo.get(QChemJobInfo::BaseName);
   m_serverName = m_qchemJobInfo.get(QChemJobInfo::ServerName);
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
      resetTimer(map.value("RunTime").toUInt()); 
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
   output.replace("${JOB_ID}", m_jobId);

   output.replace("${JOB_NAME}", m_qchemJobInfo.get(QChemJobInfo::BaseName));
   output.replace("${QUEUE}",    m_qchemJobInfo.get(QChemJobInfo::Queue));
   output.replace("${WALLTIME}", m_qchemJobInfo.get(QChemJobInfo::Walltime));
   output.replace("${MEMORY}",   m_qchemJobInfo.get(QChemJobInfo::Memory));
   output.replace("${JOBFS}",    m_qchemJobInfo.get(QChemJobInfo::Scratch));
   output.replace("${SCRATCH}",  m_qchemJobInfo.get(QChemJobInfo::Scratch));
   output.replace("${NCPUS}",    m_qchemJobInfo.get(QChemJobInfo::Ncpus));

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


void Job::setStatus(Status const status)
{
   if (m_status == status) return;
   m_status = status;

   switch (m_status) {
      case NotRunning:
         m_message = toString(NotRunning);
         updated();
         break;

      case Queued:
         m_message = toString(Queued);
         m_submitTime = QTime::currentTime().toString("h:mm:ss");
         updated();
         break;

      case Running:
         m_message = toString(Running);
         m_timer.start();
         updated();
         break;

      case Suspended:
         m_message = toString(Suspended);
         m_timer.stop();
         updated();
         break;

      case Killed:
         m_message = toString(Killed);
         m_timer.stop();
         updated();
         finished();
         break;

      case Error:
         m_message = toString(Error);
         m_timer.stop();
         updated();
         finished();
         break;

      case Finished:
         m_message = toString(Finished);
         m_timer.stop();
         updated();
         finished();
         break;

      case Copying:
         m_message = toString(Copying);
         updated();
         break;

      case Unknown:
         m_message = toString(Copying);
         m_timer.stop();
         updated();
         break;
   }
}


bool Job::isActive() const
{
   bool active(true);

   switch (m_status) {
      case NotRunning:
      case Queued:
      case Running:
      case Suspended:
      case Unknown:
         active = true;
         break;

      case Killed:
      case Error:
      case Finished:
      case Copying:
         active = false;
         break;
   }

   return active;
}

} } // end namespaces IQmol::Process
