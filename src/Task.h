#ifndef IQMOL_TASK_H
#define IQMOL_TASK_H
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

#include <QThread>
#include <QTime>
#include <QMutexLocker>
#include <exception>


namespace IQmol {

   /// Base class for objects that should be run in a separate thread.  
   class Task : public QObject {

      Q_OBJECT

      public:
         /// A mutex can be passed to the ctor to guard a resource.
         Task(QMutex* mutex = 0, int timeout = 1000) : m_terminate(false), 
            m_timeout(timeout), m_mutex(mutex), m_time(0.0)
         {
            moveToThread(&m_thread);
            connect(&m_thread, SIGNAL(started()), this, SLOT(process()));
         }

         virtual ~Task() {
            m_terminate = true;
            if (!wait(m_timeout)) {
               m_thread.quit();
               if (!wait(m_timeout)) {
                  m_thread.terminate();
               }
            } 
         }

         bool wait(unsigned long time = ULONG_MAX) {
            return m_thread.wait(time);
         }

         QString errorMessage() const {
            return m_errorMessage;
         }

		 /// Provides a way of passing information on successful completion of
		 /// the Task
         QString outputMessage() const {
            return m_outputMessage;
         }

         double timeTaken() const { return m_time; }


      Q_SIGNALS:
         /// Issued only on successful completion of the Task.
         void finished();
         /// Issued when the Task does not complete successfully.
         void failed();
		 /// Issued to update the caller on the progress of the Task.  Signal a
		 /// total of 0 if the real total cannot be calculated.
		 /// value if the percentage cannot be calculated.
         void progress(int done, int total);


      public Q_SLOTS:
         virtual void start() {
            m_thread.start();
         }

		 /// This simply sets the m_terminate flag and does not actually kill
		 /// the thread.  It is up to the dervived clasess to check the value
		 /// of the flag when appropriate and terminate cleanly.
         virtual void stopWhatYouAreDoing() {
            disconnect(&m_thread, SIGNAL(finished()), this, SLOT(finished()));
            m_terminate = true;
         }


      protected:
		 /// This function needs to be re-omplemented in the derived classes
		 /// and is where all the work is done.
         virtual void run() = 0;

         bool m_terminate;
         int  m_timeout;
         QString m_errorMessage;
         QString m_outputMessage;


      protected Q_SLOTS:
         /// We need to catch exceptions here as we are threaded
         virtual void process() {
            QTime time;
            time.start();
            try {

               if (m_mutex) {
                  QMutexLocker locker(m_mutex);
                  run();
               }else {
                  run();
               }

               m_thread.quit();
               finished();

            } catch (std::exception& err) {
               m_thread.quit();
               m_errorMessage = QString(err.what());
               failed();
            }
            m_time = time.elapsed() / 1000.0;
         }


      private:
         QThread m_thread;
         QMutex* m_mutex;
         Task(Task const&);
         Task& operator=(Task const&);
         double m_time;
   };


} // end namespace IQmol

#endif
