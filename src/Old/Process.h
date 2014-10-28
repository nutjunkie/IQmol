#ifndef IQMOL_PROCESS_H
#define IQMOL_PROCESS_H
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

   /// The Process class is used to interface beteen the ProcessMonitor and the
   /// various Servers.  
   class Process : public QObject {

      Q_OBJECT

      public:
         enum Status { NotRunning = 0,  
                       Queued,  
                       Running,  
                       Suspended, 
                       Killed,  
                       Error,   
                       Finished,  
                       Copying,  
                       Unknown };

         static QString toString(Status const& status);

         Process(JobInfo* jobInfo) : m_jobInfo(jobInfo), m_status(NotRunning),
           m_copyProgress(0), m_copyTarget(1) { }

		 /// Note that deleting a Process will not result in the termination 
		 /// of the process.  This allows jobs to continue running even after
         /// IQmol has terminated.
         ~Process();

		 /// Converts the process in to a form suitable for writing to the
		 /// Preferences.
         QVariant serialize();
         static Process* deserialize(QVariant const&);

		 /// Generates a list of strings for items in the Process::Monitor
		 /// processTable.  Note if there is an error we tack it on to the
		 /// end of this list so that it can be displayed as a tooltip for 
		 /// the item in the Status column.
         QStringList monitorItems() const;

		 /// Set the comment that is displayed in the tooltip in the status
		 /// column of the ProcessMonitor.
         void setComment(QString const& comment) { m_comment = comment; }
         QString comment() const { return m_comment; }

		 /// The Process timer runs independently of the actual process and so
		 /// may get out of sync if, for example, the process is suspended.  This
		 /// function can be used to set the timer to the actual run time when it
		 /// is known.
         void resetTimer(int const seconds);

         QString id() const { return m_id; }
         void setId(QString const& id) { m_id = id; }

         void setStatus(Status const status);
         Status status() const { return m_status; }

         void setCopyActive(bool const);
         void setCopyTarget(int kbBlocks);

         JobInfo* jobInfo() const { return m_jobInfo; }
         QString name() const { return m_jobInfo->get(JobInfo::BaseName); }
         QString serverName() const { return m_jobInfo->get(JobInfo::ServerName); }

         bool getLocalSaveDirectory();

      Q_SIGNALS:
         void updated();
         void finished();

      public Q_SLOTS:
         void viewOutput();
         void copyProgress();

      protected:
         void setPID(unsigned int const);
         unsigned int getPID() const;
         QString m_comment;
         QString m_id;       // PID or PBS job number

      private:

         JobInfo* m_jobInfo;
         Status   m_status;
         QString  m_submitTime;
         unsigned m_runTime;

         Util::Timer m_timer;
         Status   m_preCopyStatus;
         int      m_copyProgress; // block count
         int      m_copyTarget;   // target block count (kb blocks)
   };

   typedef QList<Process*> ProcessList;

} // end namespaces IQmol

#endif
