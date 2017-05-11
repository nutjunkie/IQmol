#ifndef IQMOL_UTIL_TASK_H
#define IQMOL_UTIL_TASK_H
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

#include <QThread>
#include <QTime>


namespace IQmol {

   /// Base class for tasks that need to run in a separate thread.  If no
   /// thread is passed in the ctor, one is created and managed by the class.
   class Task : public QObject {

      Q_OBJECT

      public:
         enum Status { Pending, Running, Completed, Terminated, Error, SigTrap };
 
         Task(QThread* thread = 0, int timeout = 1000);

         virtual ~Task() {
            m_terminate = true;
            if (!wait(m_timeout)) {
               m_thread->quit();
               if (!wait(m_timeout)) {
                  m_thread->terminate();
               }
            } 
            if (m_deleteThread) delete m_thread;
         }

         // time in milliseconds
         bool wait(unsigned long time = ULONG_MAX) {
            return m_thread->wait(time);
         }

         void msleep(unsigned long time) {
            m_thread->msleep(time);
         }

         bool isRunning() const { return (m_status == Pending || m_status == Running); }

         QString info() const { return m_info; } 
         Status  status() const { return m_status; }
         double  timeTaken() const { return m_time; }
         int     totalProgress() const { return m_totalProgress; }


      Q_SIGNALS:
		 /// Signals the task is no longer running, check the status to see if
		 /// this is because it has completed, was terminated by the user or 
         /// if an error occured.
         void finished(); 

         // done <= m_totalProgress
         void progress(int done);


      public Q_SLOTS:
         virtual void start() {
            m_thread->start();
         }

		 /// This simply sets the m_terminate flag and does not actually kill
		 /// the thread.  It is up to the dervived clasess to check the value
		 /// of the flag when appropriate and terminate cleanly.
         virtual void stopWhatYouAreDoing() {
            m_terminate = true;
            setStatus(Terminated);
         }


      protected:
         void setStatus(Status const status);
		 /// This function needs to be re-implemented in the derived classes
		 /// and is where all the work is done.
         virtual void run() = 0;

         bool     m_terminate;
         QString  m_info;
         QThread* m_thread;
         int      m_totalProgress;


      private Q_SLOTS:
         /// We need to catch exceptions here as we are threaded
         void process();


      private:
         Status   m_status;
         bool     m_deleteThread;
         double   m_time;
         int      m_timeout;  // in msec

         // No copying allowed
         Task(Task const&);
         Task& operator=(Task const&);
   };


} // end namespace IQmol

#endif
