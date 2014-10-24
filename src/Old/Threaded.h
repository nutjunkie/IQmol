#ifndef IQMOL_THREADED_H
#define IQMOL_THREADED_H
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
#include <QMutexLocker>
#include <exception>

#include <QDebug>


namespace IQmol {

   /// Base class for threaded objects that takes care of locking and provides
   /// a hook for terminating threads safely.  Derived classes must implement
   /// thr run() member function.  When finished, the errror message should be
   /// check to see if the thread executed succesfully.
   class Threaded : public QObject {

      Q_OBJECT

      public:
         Threaded(int timeout = 1000) : m_terminate(false), m_timeout(timeout), m_mutex(0)
         {
            moveToThread(&m_thread);
            connect(&m_thread, SIGNAL(started()), this, SLOT(process()));
         }

         virtual ~Threaded() {
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

         bool isRunning() const {
            return m_thread.isRunning();
         }

         QString errorMessage() const {
            return m_errorMessage;
         }

         QString outputMessage() const {
            return m_outputMessage;
         }


      Q_SIGNALS:
         void finished();
         void killed();


      public Q_SLOTS:
         virtual void start() {
            m_thread.start();
         }

		 /// This does not actually kill the processing.  It is up to the
		 /// dervived clasess to check the value of m_terminate appropriately
         /// and clean up after themselves.
         virtual void stopWhatYouAreDoing() {
            m_terminate = true;
         }


      protected:
         virtual void run() = 0;
         void setMutex(QMutex* mutex) { m_mutex = mutex; }
         bool m_terminate;
         int  m_timeout;
         QString m_errorMessage;
         QString m_outputMessage;
         QThread m_thread;
         QMutex* m_mutex;


      protected Q_SLOTS:
         /// We need to catch exceptions here as we are threaded
         virtual void process() {
            try {

               if (m_mutex) {
                  QMutexLocker locker(m_mutex);
                  run();
               }else {
                  run();
               }

               m_thread.quit();

               if (m_terminate) {
                  killed();
               }else {
                  finished();
               }

            } catch (std::exception& err) {
               m_thread.quit();
               m_errorMessage = QString(err.what());
               finished();
            }
         }


      private:
         Threaded(Threaded const&);
         Threaded& operator=(Threaded const&);
   };


} // end namespace IQmol

#endif
