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

#include "Task.h"


namespace IQmol {

Task::Task(QThread* thread, int timeout) : m_terminate(false), m_thread(thread), 
   m_totalProgress(100), m_deleteThread(false), m_time(0.0), m_timeout(timeout)
{
   if (!m_thread) {  
      m_thread = new QThread();
      m_deleteThread = true;
   }

   setStatus(Pending);
   moveToThread(m_thread);
   connect(m_thread, SIGNAL(started()), this, SLOT(process()));
}


void Task::setStatus(Status const status) 
{
   m_status = status;
   switch (m_status) {
      case Pending:
      case Running:
        break;
      case Completed:
      case Terminated:
      case Error:
        finished();
        break;
   }
}


/// We need to catch exceptions here as we are threaded
void Task::process() 
{
   setStatus(Running);
   QTime time;
   time.start();
   try {
      run();
      m_thread->quit();
      setStatus(Completed);
   } catch (std::exception& err) {
      m_thread->quit();
      m_info = QString(err.what());
      setStatus(Error);
   }
   m_time = time.elapsed() / 1000.0;
}

} // end namespace IQmol
