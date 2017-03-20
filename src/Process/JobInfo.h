#ifndef IQMOL_PROCESS_JOBINFO_H
#define IQMOL_PROCESS_JOBINFO_H
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

#include <QVariant>

namespace IQmol {
namespace Process {

   /// ABC for objects that hold information about a Server job.  It 
   /// is essentially a data class that contains everything needed to
   /// serialize the Job.
   class JobInfo {

      public:
         enum Status { Unknown = 0,
                       NotRunning,
                       Queued, 
                       Running, 
                       Suspended, 
                       Killed,
                       Error, 
                       Finished, 
                       Copying 
                     };

         static QString toString(Status const);

         JobInfo() : m_jobStatus(NotRunning), m_submitTime(0), m_memory(0), 
            m_scratch(0), m_ncpus(1) { }

         JobInfo(JobInfo const& that) { copy(that); }

         JobInfo& operator=(JobInfo const& that) {
            if (this != &that) copy(that);  return *this;
         }

         /// Used to flag jobs for update when loaded from file
         static bool isActive(Status const);
         bool isActive() const { return isActive(m_jobStatus); }

         Status jobStatus() const { return m_jobStatus; }
         void setJobStatus(Status const jobStatus) { m_jobStatus = jobStatus; }

         QString const& jobId() const { return m_jobId; }
         void setJobId(QString const& jobId) { m_jobId = jobId; }

         QString const& baseName() const { return m_baseName; }
         void setBaseName(QString const& baseName) { m_baseName = baseName; }

         QString const& serverName() const { return m_serverName; }
         void setServerName(QString const& serverName) { m_serverName = serverName; }

         QString const& message() const { return m_message; }
         void setMessage(QString const& message) { m_message = message; }

         QString const& queueName() const { return m_queueName; }
         void setQueueName(QString const& queueName) { m_queueName = queueName; }

         qint64 submitTime() const { return m_submitTime; }
         void setSubmitTime(qint64 submitTime) { m_submitTime = submitTime; }

         QString wallTime() const { return m_wallTime; }
         void setWallTime(QString const& wallTime) { m_wallTime = wallTime; }

         unsigned memory() const { return m_memory; }
         void setMemory(unsigned const memory) { m_memory = memory; }

         unsigned scratch() const { return m_scratch; }
         void setScratch(unsigned const scratch) { m_scratch = scratch; }

         unsigned ncpus() const { return m_ncpus; }
         void setNcpus(unsigned const ncpus) { m_ncpus = ncpus; }


         /// Serialization functions used to reconstruct the contents of the 
         /// ProcessMonitor on restarting IQmol. 
         virtual QVariantList toQVariantList() const;
         virtual bool fromQVariantList(QVariantList const&);

         virtual void dump() const;

      protected:
         void copy(JobInfo const&);


      private:
         Status    m_jobStatus;                        // 0
         QString   m_baseName;                         // 1
         QString   m_serverName;                       // 2
         QString   m_jobId;                            // 3
         QString   m_message;                          // 4
         QString   m_queueName;                        // 5
         QString   m_wallTime;    // hh:mm:ss          // 6
         qint64    m_submitTime;  // msec since epoch  // 7
         unsigned  m_memory;      // in Mb             // 8
         unsigned  m_scratch;     // in Mb             // 9
         unsigned  m_ncpus;                            // 10
   };


} } // end namespace IQmol::Process

#endif
