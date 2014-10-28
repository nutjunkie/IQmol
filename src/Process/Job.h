#ifndef IQMOL_PROCESS_JOB_H
#define IQMOL_PROCESS_JOB_H
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

#include "Timer.h"
#include "JobInfo.h"
#include <QStringList>


class QProcess;

namespace IQmol {
namespace Process2 {

   /// The Job class is used to interface beteen the ProcessMonitor 
   /// and the Servers.  
   class Job : public QObject {

      Q_OBJECT

      public:
         enum Status { NotRunning = 0, Queued, Running, Suspended, Killed,  
                       Error, Finished, Copying, Unknown };

         static QString toString(Status const& status);

         Job(QString const& jobName = QString(), QString const& serverName = QString());

		 /// Note that deleting a Job will not result in the termination 
		 /// of the process.  This allows jobs to continue running even 
         /// after IQmol has terminated.
         ~Job();

		 /// Converts the process into a form suitable 
		 /// for writing to the Preferences.
         virtual QVariant toQVariant() const;
         bool fromQVariant();

		 /// Set the message that is displayed in the tooltip in the status
		 /// column of the ProcessMonitor.
         void setMessage(QString const& message) { m_message = message; }
         QString const& message() const { return m_message; }

         void setStatus(Status const status);
         Status status() const { return m_status; }
         virtual QString statusString const;

         /// This is an external handle for the process, either a
         /// PID or PBS/SGE job number.
         void setJobId(QString const& id) { m_jobId = id; }
         QString const& jobId() const { return m_jobId; }

		 /// The Job timer runs independently of the actual process and so
		 /// may get out of sync if, for example, the process is suspended. 
         /// This function can be used to set the timer to the actual run 
		 /// time when it is known.
         void resetTimer(unsigned const seconds);
         unsigned runTime() const;

      Q_SIGNALS:
         void updated();

      protected:
         QString  m_jobName;
         QString  m_serverName;
         QString  m_submitTime;
         Status   m_status;
         QString  m_message;
         QString  m_jobId;
         unsigned m_runTime; 
  
         Util::Timer m_timer;
   };

   typedef QList<Job*> JobList;

} } // end namespaces IQmol::Process

#endif
