#ifndef IQMOL_SERVERTASK_H
#define IQMOL_SERVERTASK_H
/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include "Threaded.h"
#include "HostDelegate.h"
#include "Process.h"


namespace IQmol {

class HostDelegate;
class Server;

namespace ServerTask {

   class Base : public Threaded {

      Q_OBJECT

      public:
         Base(Server* server, int timeout = 5000);
         void setOutputMessage(QString const& msg) { m_outputMessage = msg; }

      Q_SIGNALS:
         void terminateRequested();

      public Q_SLOTS:
         void stopWhatYouAreDoing();

      protected:
         virtual void run() { }
         /// Convenience function that does a blocking call taking care to
         /// lock the appropriate mutex.
         void runThread(Threaded* thread);
         bool exists(QString const& filePath, HostDelegate::FileFlags const flags);
         bool mkdir(QString const& dir);
         bool push(QString const& sourcePath, QString const& destinationPath);
         bool pull(QString const& sourcePath, QString const& destinationPath);
         bool move(QString const& sourcePath, QString const& destinationPath);
         bool remove(QString const& filePath);
         QString exec(QString const& command);
         QString grep(QString const& string, QString const& filePath);

         QString workingDirectory(JobInfo* jobInfo) const { 
            return m_hostDelegate->workingDirectory(jobInfo); 
         }

      protected:
         Server* m_server;
         ServerDelegate* m_serverDelegate;
         HostDelegate* m_hostDelegate;

      private:
         QMutex m_workerLock;
         int m_timeout;
   };

   // --------------------------------

   class TestConfiguration : public Base {
      Q_OBJECT
      public:
         TestConfiguration(Server* server) : Base(server) { }
         void addFileTest(QString const& file, HostDelegate::FileFlags const flags) {
            m_filesForTesting.append(file);
            m_fileFlags.append(flags);
         }
      protected:
         void run();
      private:
         QStringList m_filesForTesting;
         QList<HostDelegate::FileFlags> m_fileFlags;
   };

   // --------------------------------

   class ProcessTask : public Base {
      Q_OBJECT
      public:
         ProcessTask(Server* server, Process* process) : Base(server), m_process(process) { }
         Process* process() const { return m_process; }
      protected:
         Process* m_process;
   };

   // --------------------------------

   class Setup : public ProcessTask {
      Q_OBJECT
      public:
         Setup(Server* server, Process* process) : ProcessTask(server, process) { }
      protected:
         void run();
   };

   // --------------------------------

   class Submit :  public ProcessTask {
      Q_OBJECT
      public:
         Submit(Server* server, Process* process) : ProcessTask(server, process) { }
      protected:
         bool createSubmissionScript(Process*);
   };


   // --------------------------------

   class BasicSubmit : public Submit {
      
      public:
         BasicSubmit(Server* server, Process* process) : Submit(server, process) { }
      protected:
         void run();
      private:
         void runRemote();
         void runLocal();
   };

   // --------------------------------

   class PBSSubmit : public Submit {
      Q_OBJECT
      public:
         PBSSubmit(Server* server, Process* process) : Submit(server, process) { }
      protected:
         void run();
   };

   // --------------------------------

   class Query : public ProcessTask {
      Q_OBJECT
      public:
         Query(Server* server, Process* process) : ProcessTask(server, process) { }
         Process::Status newStatus() const { return m_status; }
      protected:
         void run();
      private:
         Process::Status m_status;
   };

   // --------------------------------

   class CleanUp : public ProcessTask {
      Q_OBJECT
      public:
         CleanUp(Server* server, Process* process) : ProcessTask(server, process), m_time(0) { }
         int time() const { return m_time; }
         QString runTimeError() const { return m_runTimeError; }
      protected:
         void run();
      private:
         QString m_runTimeError;
         int m_time;
   };

   // --------------------------------

   class KillProcess : public ProcessTask {
      Q_OBJECT
      public:
         KillProcess(Server* server, Process* process) : ProcessTask(server, process) { }
      protected:
         void run();
   };

   // --------------------------------

   class CopyResults : public ProcessTask {
      Q_OBJECT
      public:
         CopyResults(Server* server, Process* process) : ProcessTask(server, process) { }
      protected:
         void run();
   };


} } // end namespace IQmol::ServerTask

#endif
