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

#include "JobInfo.h"
#include "QsLog.h"


namespace IQmol {
namespace Process {


QString JobInfo::toString(Status const status) 
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


bool JobInfo::isActive(Status const status)
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


QVariantList JobInfo::toQVariantList() const
{
   QVariantList list;

   list << QVariant((int)m_jobStatus);
   list << QVariant(m_baseName);
   list << QVariant(m_serverName);
   list << QVariant(m_jobId);
   list << QVariant(m_message);
   list << QVariant(m_queueName);
   list << QVariant(m_wallTime);
   list << QVariant(m_submitTime);
   list << QVariant(m_memory);
   list << QVariant(m_scratch);
   list << QVariant(m_ncpus);

   return list;
}


bool JobInfo::fromQVariantList(QVariantList const& list)
{
   bool ok(list.size() == 11);      if (!ok) return false;

   ok = list[0].canConvert<int>();  if (!ok) return false;
   m_jobStatus = static_cast<Status>(list[0].toInt());

   ok = list[1].canConvert<QString>();  if (!ok) return false;
   m_baseName = list[1].toString();

   ok = list[2].canConvert<QString>();  if (!ok) return false;
   m_serverName = list[2].toString();

   ok = list[3].canConvert<QString>();  if (!ok) return false;
   m_jobId = list[3].toString();

   ok = list[4].canConvert<QString>();  if (!ok) return false;
   m_message = list[4].toString();

   ok = list[5].canConvert<QString>();  if (!ok) return false;
   m_queueName = list[5].toString();

   ok = list[6].canConvert<QString>();  if (!ok) return false;
   m_wallTime = list[6].toString();

   ok = list[7].canConvert<qint64>();   if (!ok) return false;
   m_submitTime = list[7].toLongLong();

   ok = list[8].canConvert<unsigned>(); if (!ok) return false;
   m_memory = list[8].toUInt();

   ok = list[9].canConvert<unsigned>(); if (!ok) return false;
   m_scratch = list[9].toUInt();

   ok = list[10].canConvert<unsigned>(); if (!ok) return false;
   m_ncpus = list[10].toUInt();

   return ok;
}


void JobInfo::copy(JobInfo const& that)
{
   m_jobStatus  = that.m_jobStatus;
   m_baseName   = that.m_baseName;
   m_serverName = that.m_serverName;
   m_jobId      = that.m_jobId;
   m_message    = that.m_message;
   m_queueName  = that.m_queueName;
   m_wallTime   = that.m_wallTime;
   m_submitTime = that.m_submitTime;
   m_memory     = that.m_memory;
   m_scratch    = that.m_scratch;
   m_ncpus      = that.m_ncpus;
}


void JobInfo::dump() const
{
   QLOG_DEBUG() << "--- JobInfo ---";
   QLOG_DEBUG() << "jobStatus  = " << m_jobStatus;
   QLOG_DEBUG() << "baseName   = " << m_baseName;
   QLOG_DEBUG() << "serverName = " << m_serverName;
   QLOG_DEBUG() << "job ID     = " << m_jobId;
   QLOG_DEBUG() << "message    = " << m_message;
   QLOG_DEBUG() << "queueName  = " << m_queueName;
   QLOG_DEBUG() << "wallTime   = " << m_wallTime;
   QLOG_DEBUG() << "submitTime = " << m_submitTime;
   QLOG_DEBUG() << "memory     = " << m_memory;
   QLOG_DEBUG() << "scratch    = " << m_scratch;
   QLOG_DEBUG() << "ncpus      = " << m_ncpus;
}

} } // end namespace IQmol::Process

