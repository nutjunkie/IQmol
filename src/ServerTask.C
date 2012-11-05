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

#include "ServerTask.h"
#include "PBSServer.h"
#include "JobInfo.h"
#include "QsLog.h"
#include "System.h"
#include <QTemporaryFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>

#include <QDebug>


namespace IQmol {
namespace ServerTask {

Base::Base(Server* server, int timeout) : Threaded(timeout), m_server(server)
{ 
   m_serverDelegate = server->m_serverDelegate;
   m_hostDelegate = m_serverDelegate->m_hostDelegate;
}


void Base::stopWhatYouAreDoing() 
{
   m_terminate = true;
   terminateRequested();
}


QString Base::exec(QString const& command) 
{
   Threaded* thread = m_hostDelegate->exec(command);    
   runThread(thread);
   QString output(thread->outputMessage());
   thread->deleteLater();
   return output;
}


bool Base::exists(QString const& filePath, HostDelegate::FileFlags const flags)
{
   Threaded* thread = m_hostDelegate->exists(filePath, flags);    
   runThread(thread);
   bool output(thread->outputMessage().contains("exists"));
   thread->deleteLater();
   return output;
}


bool Base::mkdir(QString const& dir)
{
   Threaded* thread = m_hostDelegate->mkdir(dir);
   runThread(thread);
   bool output(thread->errorMessage().isEmpty());
   thread->deleteLater();
   return output;
}


bool Base::push(QString const& sourcePath, QString const& destinationPath)
{
   Threaded* thread = m_hostDelegate->push(sourcePath, destinationPath);    
   runThread(thread);
   bool output(thread->errorMessage().isEmpty());
   if (!output) {
      qDebug() << errorMessage();
   }
   thread->deleteLater();
   return output;
}


bool Base::pull(QString const& sourcePath, QString const& destinationPath)
{
   Threaded* thread = m_hostDelegate->pull(sourcePath, destinationPath);    
   runThread(thread);
   bool output(thread->errorMessage().isEmpty());
   thread->deleteLater();
   return output;
}


bool Base::move(QString const& sourcePath, QString const& destinationPath)
{
   Threaded* thread = m_hostDelegate->move(sourcePath, destinationPath);    
   runThread(thread);
   bool output(thread->errorMessage().isEmpty());
   thread->deleteLater();
   return output;
}


bool Base::remove(QString const& filePath)
{
   Threaded* thread = m_hostDelegate->remove(filePath);    
   runThread(thread);
   bool output(thread->errorMessage().isEmpty());
   thread->deleteLater();
   return output;
}


QString Base::grep(QString const& string, QString const& filePath)
{
   Threaded* thread = m_hostDelegate->grep(string, filePath);    
   runThread(thread);
   QString output(thread->outputMessage());
   thread->deleteLater();
   return output;
}


void Base::runThread(Threaded* thread)
{
   QMutexLocker locker(&m_workerLock);
   connect(this, SIGNAL(terminateRequested()), thread, SLOT(stopWhatYouAreDoing()));
   thread->start();
   thread->wait();
}


/// Writes contents to a temporary file and returns the temporary file name.
/// This is done in a cack-arsed way as it seems that on Windows the buffer 
/// is not properly flushed to disk when the file is closed, so we end copying
/// an emtpy file.  We use QTemoporaryFile simply to get a unique file name.
QString Base::writeToTemporaryFile(QString const& contents)
{
   QDir tmpDir(QDir::temp());
   if (!tmpDir.exists()) tmpDir = QDir::home();
   QFileInfo tmpFileInfo(tmpDir, "iqmol_temp.XXXXXX");

   QString tmpFilePath;
   { 
      QTemporaryFile file(tmpFileInfo.filePath());
      if (file.open()) {
         tmpFilePath = file.fileName();
         file.close();
      }
   }

   if (tmpFilePath.isEmpty()) return QString();
   QFile file(tmpFilePath);
   if (file.exists()) file.remove();
   if (!file.open(QIODevice::WriteOnly)) return QString();

   QByteArray buffer;
   buffer.append(contents);
   file.write(buffer);
   file.flush();
   file.close();

   return tmpFilePath;
}
 

// --------------------------------


void TestConfiguration::run()
{
   QString file;
   HostDelegate::FileFlags flags;

   for (int i = 0; i < m_filesForTesting.size(); ++i) {
       file  = m_filesForTesting[i];
       flags = m_fileFlags[i];
       QLOG_DEBUG() << "TestConfiguration::run() checking file" << file << "with flags" << flags;

       if (m_terminate) {
          m_errorMessage += "Terminated";
          break;
       }
       if (!exists(file, flags)) {
          if (flags & (HostDelegate::Directory | HostDelegate::Create)) {
             if (!mkdir(file)) m_errorMessage += "Could not create directory: " + file + "\n";
          }else if (flags & HostDelegate::Executable) {
             m_errorMessage += "Could not find command: " + file + "\n";
          }else {
             m_errorMessage += "Could not access file: " + file + "\n";
          }
       }
   }

   PBSServer* server = qobject_cast<PBSServer*>(m_serverDelegate);
   if (server) {
      QLOG_DEBUG() << "Obtaining queues from PBS server";
      QString cmd("qstat -fQ");
      cmd = exec(cmd);

      if (cmd.contains("Command not found")) {
         m_errorMessage  = "Could not find qstat command on PBS server ";
         m_errorMessage += m_server->name();
      }else  {
         QueueList queues(PBSQueue::fromQstat(cmd));
         server->setQueues(queues);
         QLOG_DEBUG() << "PBSQueue::fromQstat found" << queues.size() << "queues";
      }
   }
}


// --------------------------------

void Setup::run()
{
   // Check working directory
   JobInfo* jobInfo(m_process->jobInfo());
   QString dir(workingDirectory(jobInfo));
   HostDelegate::FileFlags flags;

   if (exists(dir, HostDelegate::Directory)) {
      if (jobInfo->promptOnOverwrite()) {
         m_errorMessage = "Working directory exists";
         return;
      }else {
		 // We need to clean out the directory as if the job fails there may be
		 // some inconsistent/outdated files lying around from last time.
         QDir qDir(dir);
         QFileInfo output(qDir, jobInfo->get(JobInfo::OutputFileName));
         QFileInfo fchk(qDir, jobInfo->get(JobInfo::AuxFileName));
         remove(output.filePath());
         remove(fchk.filePath());
      }
   }else {
      if (!mkdir(dir)) {
         m_errorMessage = "Could not create working directory on server";
         return;
      }
   }

   QString contents(jobInfo->get(JobInfo::InputString));
   QString tmpFilePath(writeToTemporaryFile(contents));

   if (tmpFilePath.isEmpty()) {
      m_errorMessage = "Failed to create temporary input file";
      return;
   }

   QFileInfo fileName(dir, jobInfo->get(JobInfo::InputFileName));

   if (!push(tmpFilePath, fileName.filePath())) {
       m_errorMessage = "Failed to copy input file to server";
   }
      
   QFile tmp(tmpFilePath);
   tmp.remove();
}


// --------------------------------

bool Submit::createSubmissionScript(Process* process)
{
   QString contents(m_server->runFileTemplate());
   contents += "\n";
   contents = m_server->replaceMacros(contents, process);
   QString tmpFilePath(writeToTemporaryFile(contents));

   if (tmpFilePath.isEmpty()) {
      m_errorMessage = "Failed to create temporary run file";
      return false;
   }

   JobInfo* jobInfo(process->jobInfo());
   QString dir(workingDirectory(jobInfo));
   QFileInfo fileName(dir, jobInfo->get(JobInfo::RunFileName));

   bool ok(true);
   if (!push(tmpFilePath, fileName.filePath())) {
      m_errorMessage = "Failed to copy run file to server";
      ok = false;
   }

   QFile tmp(tmpFilePath);
   tmp.remove();
   return ok;
}


void BasicSubmit::run()
{
   if (!createSubmissionScript(m_process)) return;

   switch (m_server->host()) {
      case Server::Local:
         runLocal();
         break;
      case Server::Remote:
         runRemote();
         break;
   }

}


void BasicSubmit::runRemote()
{
   QString dir(m_process->jobInfo()->get(JobInfo::RemoteWorkingDirectory));
   QString script(m_process->jobInfo()->get(JobInfo::RunFileName));
   QString query(m_server->queryCommand());
   query = m_server->replaceMacros(query, m_process);

   // make sure the submission script is executable
   exec("cd " + dir + " && chmod +x " + script);

   QString submitCommand("cd " + dir + " && ");
   submitCommand += m_server->replaceMacros(m_server->submitCommand(), m_process);  
   submitCommand += " && sleep 1 && " + query;

   QString output(exec(submitCommand));
   bool found(false);
   m_process->setId("0");

   QStringList progs(output.split(QRegExp("\\n"), QString::SkipEmptyParts));
   QStringList::iterator iter;
   // We need to find the most recently submitted process that matches the
   // executable name and was submitted sufficiently recently.
   int tmin(5);
   for (iter = progs.begin(); iter != progs.end(); ++iter) {
       QStringList tokens((*iter).split(" ", QString::SkipEmptyParts));
       if (tokens.size() == 3) {
          int t(Timer::toSeconds(tokens[2]));
          if (t >= 0 && t < tmin) {
             tmin = t;
             found = true;
             m_process->setId(tokens[1]); // assume second token is the pid
          }
       }
   }

   // We assume the process started running and leave it until a later query to
   // determine if it has already finished, crashed or is still running.
   m_process->setStatus(Process::Running);
   m_server->addToWatchList(m_process);

   QString msg("Basic job submitted with PID ");
   if (found) {
      msg += m_process->id();
   }else {
      msg += "not found";
      m_errorMessage = "Process not found";
   }

qDebug() << msg;
      QLOG_DEBUG() << msg;
}


void BasicSubmit::runLocal()
{
   JobInfo* jobInfo(m_process->jobInfo());
   QDir dir(jobInfo->get(JobInfo::LocalWorkingDirectory));
   QFileInfo  input(dir, jobInfo->get(JobInfo::InputFileName));
   QFileInfo output(dir, jobInfo->get(JobInfo::OutputFileName));

   QProcess* qprocess = new QProcess();
   qprocess->setWorkingDirectory(dir.path());
   qprocess->setStandardOutputFile(output.filePath());
   qprocess->setStandardErrorFile(output.filePath(), QIODevice::Append);

   QStringList args;
   args << input.fileName();
#ifndef Q_WS_WIN
   args << output.fileName();
#endif

   QString cmd(m_server->replaceMacros("${QC}/bin/qchem", m_process));

   jobInfo->localFilesExist(true);

   m_process->setQProcess(qprocess);
   m_process->setStatus(Process::Running);
   qprocess->start(cmd, args);

   // attempt to get the actual PID of the qchem.exe process, but we need to
   // give the exe a chance to fire up.
   unsigned int pid(0);
   for (int i = 0; i < 5; ++i) {
       wait(1000);  // wait 1000 msec for the job to start
       pid = System::ExecutablePid(m_server->executableName(), *qprocess);
       if (pid > 0) break;
   }
   // Note that if the job has alread finsished (crashed) then the pid will be
   // set to 0.  The server will pick this up the next time it updates the
   // processes, won't find the pid=0 process and so will attempt a clean up.
   m_process->setId(QString::number(pid));
   m_server->addToWatchList(m_process);
}


// --------------------------------


void PBSSubmit::run()
{
   if (!createSubmissionScript(m_process)) return;

   QString dir(m_process->jobInfo()->get(JobInfo::RemoteWorkingDirectory));
   QString script(m_process->jobInfo()->get(JobInfo::RunFileName));

   QString submitCommand("cd " + dir + " && ");
   submitCommand += m_server->replaceMacros(m_server->submitCommand(), m_process);  

   QString output(exec(submitCommand));

   if (output.isEmpty()) {
      m_errorMessage = "Failed to submit job to PBS server";
   }else {
      m_process->setId(output);
      m_process->setStatus(Process::Queued);
      m_server->addToWatchList(m_process);
      QLOG_DEBUG() << "PBS job submitted with id" << m_process->id();
   }
}

// --------------------------------

void Query::run()
{
   QString query(m_server->queryCommand());
   query = m_server->replaceMacros(query, m_process);
qDebug() << "Query command" << query;
   m_outputMessage = exec(query);
//qDebug() << "Query return" << m_outputMessage;
   m_status = m_serverDelegate->parseQueryString(m_outputMessage, m_process);
qDebug() << "Status set to" << m_status;
}


// --------------------------------

void CleanUp::run()
{
   JobInfo* jobInfo(m_process->jobInfo());
   QDir dir(workingDirectory(jobInfo));
   QFileInfo errFile(dir, jobInfo->get(JobInfo::ErrorFileName));
   QFileInfo outFile(dir, jobInfo->get(JobInfo::OutputFileName));

   // Rename the checkpoint file, if required
   QFileInfo oldFile(dir, "Test.FChk");
   QFileInfo newFile(dir, jobInfo->get(JobInfo::AuxFileName));
   move(oldFile.filePath(), newFile.filePath());

   // Get an updated time, first we check for PBS output
   QString pbs(grep("elapsed time", errFile.filePath()));
   if (!pbs.isEmpty()) {
      QStringList tokens(pbs.split(" ", QString::SkipEmptyParts));
      m_time = Timer::toSeconds(tokens.last());
qDebug() << "Time updated from PBS file" << m_time;
   }

   // If nothing found, we tally the times in the output file
   if (m_time <= 0) {
      QString times(grep("Total job time:", outFile.filePath()));
      QStringList lines(times.split(QRegExp("\\n")));
      QStringList::iterator iter;
qDebug() << "Time updated from output file";
      for (iter = lines.begin(); iter != lines.end(); ++iter) {
          QStringList tokens((*iter).split(" ", QString::SkipEmptyParts));
          tokens = tokens.filter("(wall)");
          if (!tokens.isEmpty()) {
             bool ok;
             double s(tokens.first().remove("s(wall),").toDouble(&ok));
             if (ok) m_time += (int)s;
qDebug() << "  time += " << s;
          }
      }
   }

   // Check for PBS errors in the error file
   m_runTimeError = grep("terminated", errFile.filePath());

   // If none, check for any errors reported in the output
   Threaded* thread = m_hostDelegate->checkOutputForErrors(outFile.filePath());
   runThread(thread);
   m_runTimeError = thread->outputMessage();
   thread->deleteLater();
      
   // Finally check to see that at least one job has completed
   if (m_runTimeError.isEmpty()) {
      QString happyDays(grep("Have a nice day.", outFile.filePath()));
      if (happyDays.isEmpty()) m_runTimeError = "Job failed to finish";
   }

}


// --------------------------------


void KillProcess::run()
{
   QString cmd(m_server->killCommand());
   cmd = m_server->replaceMacros(cmd, m_process);
qDebug() << "Kill command" << cmd;
   exec(cmd);
}


// --------------------------------


// This assumes the Server is a) Remote and b) *nix based.
void CopyResults::run()
{
   m_process->setCopyActive(true);
   JobInfo* jobInfo(m_process->jobInfo());

   QDir  localDir(jobInfo->get(JobInfo::LocalWorkingDirectory));
   QDir remoteDir(jobInfo->get(JobInfo::RemoteWorkingDirectory));

   QFileInfo source;
   QFileInfo destination;
   QString file;
   QString cmd;
   int  kb(0);
   bool ok(true);
   bool allOk(true);

   file = jobInfo->get(JobInfo::InputFileName);
   source.setFile(remoteDir, file);
   cmd = "du -k " + source.filePath() + " | awk \'{print $1}\'";
   cmd = exec(cmd);
   kb = cmd.toInt(&ok);
   allOk = allOk && ok;
   file = jobInfo->get(JobInfo::OutputFileName);
   source.setFile(remoteDir, file);
   cmd = "du -k " + source.filePath() + " | awk \'{print $1}\'";
   cmd = exec(cmd);
   kb += cmd.toInt(&ok);
   allOk = allOk && ok;
   file = jobInfo->get(JobInfo::AuxFileName);
   source.setFile(remoteDir, file);
qDebug() << "need to copy all fchk files";
   cmd = "du -k " + source.filePath() + " | awk \'{print $1}\'";
   cmd = exec(cmd);
   kb += cmd.toInt(&ok);
   allOk = allOk && ok;

   if (allOk) m_process->setCopyTarget(kb);

   allOk = true;

   // Input
   file = jobInfo->get(JobInfo::InputFileName);
   source.setFile(remoteDir, file);
   destination.setFile(localDir, file);
   Threaded* thread = m_hostDelegate->pull(source.filePath(), destination.filePath());
   connect(thread, SIGNAL(copyProgress()), m_process, SLOT(copyProgress()));
   runThread(thread);
   ok = thread->errorMessage().isEmpty();
   if (ok) {
      qDebug() << "File copy successful" << source.filePath();
   }else {
      qDebug() << "File copy failed" << source.filePath();
   }
   thread->deleteLater();
   if (!ok) m_errorMessage += "Failed to copy file from server\n" + file + "\n";
   allOk = allOk && ok;

   // Output
   file = jobInfo->get(JobInfo::OutputFileName);
   source.setFile(remoteDir, file);
   destination.setFile(localDir, file);
   thread = m_hostDelegate->pull(source.filePath(), destination.filePath());
   connect(thread, SIGNAL(copyProgress()), m_process, SLOT(copyProgress()));
   runThread(thread);
   ok = thread->errorMessage().isEmpty();
   if (ok) {
      qDebug() << "File copy successful" << source.filePath();
   }else {
      qDebug() << "File copy failed" << source.filePath();
   }
 
   thread->deleteLater();
   if (!ok) m_errorMessage += "Failed to copy file from server\n" + file + "\n";
   allOk = allOk && ok;

   // Fchk
   file = jobInfo->get(JobInfo::AuxFileName);
   source.setFile(remoteDir, file);
   destination.setFile(localDir, file);
   thread = m_hostDelegate->pull(source.filePath(), destination.filePath());
   connect(thread, SIGNAL(copyProgress()), m_process, SLOT(copyProgress()));
   runThread(thread);
   ok = thread->errorMessage().isEmpty();
   if (ok) {
      qDebug() << "File copy successful" << source.filePath();
   }else {
      qDebug() << "File copy failed" << source.filePath();
   }
 
   thread->deleteLater();
   if (!ok) m_errorMessage += "Failed to copy file from server\n" + file + "\n";
   allOk = allOk && ok;

   jobInfo->localFilesExist(allOk);
qDebug() << "ServerTask::CopyResults finished with ok=" << allOk;
   m_process->setCopyActive(false);
}


} } // end namespace IQmol::ServerTask
