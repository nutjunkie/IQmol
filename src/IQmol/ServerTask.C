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

#include "ServerTask.h"
#include "PBSServer.h"
#include "SGEServer.h"
#include "HttpServer.h"
#include "JobInfo.h"
#include "QsLog.h"
#include "System.h"
#include "SecureConnectionException.h"
#include <QTemporaryFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#ifndef Q_WS_WIN
#include <unistd.h>
#endif
 
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

bool TestConfiguration::testFiles()
{
   QString file;
   HostDelegate::FileFlags flags;

   for (int i = 0; i < m_filesForTesting.size(); ++i) {
       file  = m_filesForTesting[i];
       flags = m_fileFlags[i];
       QLOG_DEBUG() << "TestConfiguration::testFiles() checking " << file 
                    << "with flags" << flags;
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
   return m_errorMessage.isEmpty();
}


void TestPBSConfiguration::run()
{
   if (!testFiles()) return;

   PBSServer* delegate = qobject_cast<PBSServer*>(m_serverDelegate);
   if (!delegate) return;

   QLOG_DEBUG() << "Obtaining queues from PBS server";

   QString cmd(m_server->queueInfo());
   QString output(exec(cmd));

   if (output.contains("command not found", Qt::CaseInsensitive)) {
      QStringList tokens(cmd.split(QRegExp("\\n"), QString::SkipEmptyParts));
      if (tokens.isEmpty()) {
         m_errorMessage  = "${QUEUE_INFO} command not set for server ";
      }else {
         m_errorMessage  = "Could not find " + tokens.first() +  " command on server ";
      }
      m_errorMessage += m_server->name();
   }else  {
      int n(delegate->setQueuesFromQueueInfo(output));
      QLOG_DEBUG() << n << " queues found on SGE server" << m_server->name();
   }
}


void TestSGEConfiguration::run()
{
   if (!testFiles()) return;

   SGEServer* delegate = qobject_cast<SGEServer*>(m_serverDelegate);
   if (!delegate) return;

   QLOG_DEBUG() << "Obtaining queues from SGE server";

   QString cmd(m_server->queueInfo());
   QString output(exec(cmd));

   if (output.contains("command not found", Qt::CaseInsensitive)) {
      QStringList tokens(cmd.split(QRegExp("\\n"), QString::SkipEmptyParts));
      if (tokens.isEmpty()) {
         m_errorMessage  = "${QUEUE_INFO} command not set for server ";
      }else {
         m_errorMessage  = "Could not find " + tokens.first() +  " command on server ";
      }
      m_errorMessage += m_server->name();
   }else  {
      int n(delegate->setQueuesFromQueueInfo(output));
      QLOG_DEBUG() << n << " queues found on SGE server" << m_server->name();
   }
}


void TestHttpConfiguration::run()
{
   HttpServer* server = qobject_cast<HttpServer*>(m_serverDelegate);
   if (!server) {
      m_errorMessage = "Invalid ServerDelegate in TestHttpServer task";
      return;
   }

   QString cmd(m_server->queueInfo());
   cmd = m_server->replaceMacros(cmd);

   QString limits(exec(cmd));
   qDebug() << "WebServer returned limits:";
   qDebug() << limits;
   server->parseLimits(limits);
}



// --------------------------------

void Setup::run()
{
   // Check working directory
   JobInfo* jobInfo(m_process->jobInfo());
   //qDebug() << "8888888888888888888888888888888888888888888";
   //jobInfo->dump();
   //qDebug() << "8888888888888888888888888888888888888888888";
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
qDebug() << "ServerTask: removing output     file" << output.filePath();
         remove(output.filePath());
qDebug() << "ServerTask: removing checkpoint file" << fchk.filePath();
         remove(fchk.filePath());
      }
   }else {
      if (!mkdir(dir)) {
         m_errorMessage = "Could not create working directory " + dir + " on server";
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


void SetupHttp::run()
{
   JobInfo* jobInfo(m_process->jobInfo());
   QString contents(jobInfo->get(JobInfo::InputString));
   // WebHosts deal with push commands a bit differently, the file contents are
   // passed rather than the file name, which is determined by the server and
   // returned in the output.

   Threaded* thread = m_hostDelegate->push(contents, QString());    
   runThread(thread);

   m_errorMessage  = thread->errorMessage();
   m_outputMessage = thread->outputMessage();

   thread->deleteLater();

   if (m_outputMessage.contains("jobID=")) {
      m_outputMessage = m_outputMessage.remove("jobID=");
      bool ok;
      m_outputMessage.toInt(&ok);
      if (ok) {
         qDebug() << "Job setup returned with jobID =" << m_outputMessage;
         m_process->setId(m_outputMessage);
      }else {
         m_errorMessage = "Invalid jobID";
      }
   }

   if (m_outputMessage.contains("ERROR:")) {
      m_errorMessage = m_outputMessage;
      m_outputMessage.clear();
   }

   if (!m_errorMessage.isEmpty()) {
      QLOG_DEBUG() << "ServerTask error:" << m_errorMessage;
      m_errorMessage = "Failed to copy input to server:";
   }
}


// --------------------------------

bool Submit::createSubmissionScript(Process* process)
{
   // We don't create a submission file for Windows
#ifdef Q_WS_WIN
   if (m_server->host() == Server::Local) return true;
#endif

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

   // make sure the submission script is executable (this covers both local and
   // remote.
   exec("/bin/chmod +x " + fileName.filePath());

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
      case Server::Web:
         // shouldn't get here
         qDebug() << "ERROR: Server::Web encountered in BasicSubmit";
         break;
   }
}


void BasicSubmit::runRemote()
{
   QString dir(m_process->jobInfo()->get(JobInfo::RemoteWorkingDirectory));
   QString script(m_process->jobInfo()->get(JobInfo::RunFileName));
   QString exeName(m_server->executableName());
   QString query(m_server->queryCommand());
   query = m_server->replaceMacros(query, m_process);

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
       if ((*iter).contains(exeName)) {
          QStringList tokens((*iter).split(QRegExp("\\s+"), QString::SkipEmptyParts));
          if (tokens.size() == 3) {
             int t(Timer::toSeconds(tokens[2]));
             if (t >= 0 && t < tmin) {
                tmin = t;
                found = true;
                m_process->setId(tokens[1]); // assume second token is the pid
             }
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

   QLOG_DEBUG() << msg;
}


void BasicSubmit::runLocal()
{
   QString submitCommand( m_server->replaceMacros(m_server->submitCommand(), m_process) );  
   QStringList args(submitCommand.split(QRegExp("\\s+"), QString::SkipEmptyParts));
   if (args.isEmpty()) {
      m_errorMessage = "Command not found";
   }else {
      submitCommand = args.takeFirst();
   }

   JobInfo* jobInfo(m_process->jobInfo());
   QDir dir(jobInfo->get(JobInfo::LocalWorkingDirectory));
   QProcess* qprocess = new QProcess();
   qprocess->setWorkingDirectory(dir.path());

   QFileInfo output(dir, jobInfo->get(JobInfo::ErrorFileName));
   qprocess->setStandardOutputFile(output.filePath());
   qprocess->setStandardErrorFile(output.filePath(), QIODevice::Append);

   QLOG_DEBUG() << "QProcess executing command" << submitCommand << "\n"
                << "  in directory " << dir.path() << "\n"
                << "  with arguments " << args;

   jobInfo->localFilesExist(true);
   m_process->setStatus(Process::Running);
   qprocess->start(submitCommand, args);

   qDebug() << "Parent QProcess has PID =" << qprocess->pid() << System::ProcessID(*qprocess);

   // Attempt to get the actual PID of the qchem.exe process, but we need to
   // give the exe a chance to fire up.
   unsigned int pid(0);
   for (int i = 0; i < 7; ++i) {
#ifdef Q_WS_WIN
       Sleep(1000); 
#else
       sleep(1); 
#endif
       pid = System::ExecutablePid(m_server->executableName(), *qprocess);
       qDebug() << "PID search returned" << pid;
       if (pid > 0) break;
   }

   // Note that if the job has already finsished (crashed) then the pid will be
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

   // A successful submission returns a single token containing the job ID
   QStringList tokens(output.split(QRegExp("\\s+"), QString::SkipEmptyParts));
   if (tokens.size() == 1) {
      m_process->setId(output);
      m_process->setStatus(Process::Queued);
      m_server->addToWatchList(m_process);
      QLOG_DEBUG() << "PBS job submitted with id" << m_process->id();
   }else {
      m_errorMessage = "Failed to submit job to PBS server:\n" +  output;
   }
}

// --------------------------------

void SGESubmit::run()
{
   if (!createSubmissionScript(m_process)) return;

   QString dir(m_process->jobInfo()->get(JobInfo::RemoteWorkingDirectory));
   QString script(m_process->jobInfo()->get(JobInfo::RunFileName));

   QString submitCommand("cd " + dir + " && ");
   submitCommand += m_server->replaceMacros(m_server->submitCommand(), m_process);  

   QString output(exec(submitCommand));

   // A successful submission returns a string like:
   //   Your job 2834 ("test.sh") has been submitted
   int id(0);
   if (output.contains("has been submitted")) {
      QStringList tokens(output.split(QRegExp("\\s+"), QString::SkipEmptyParts));
      bool ok;
      id = tokens[2].toInt(&ok);
      if (ok) {
         m_process->setId(QString::number(id));
         m_process->setStatus(Process::Queued);
         m_server->addToWatchList(m_process);
         QLOG_DEBUG() << "SGE job submitted with id" << m_process->id();
      }else {
         id = 0;
      }
   }

   if (id == 0) {
      m_errorMessage = "Failed to submit job to SGE server:\n" + output;
   }
}


void HttpSubmit::run()
{
}

// --------------------------------

void Query::run()
{
   QString query(m_server->queryCommand());
   query = m_server->replaceMacros(query, m_process);
   m_outputMessage = exec(query);
   m_status = m_serverDelegate->parseQueryString(m_outputMessage, m_process);
}


// --------------------------------

void CleanUp::run()
{
   JobInfo* jobInfo(m_process->jobInfo());
   QDir dir(workingDirectory(jobInfo));
   QFileInfo errFile(dir, jobInfo->get(JobInfo::ErrorFileName));
   QFileInfo outFile(dir, jobInfo->get(JobInfo::OutputFileName));

   // Rename the auxiliary output files
   QFileInfo oldFile(dir, "Test.FChk");
   QFileInfo newFile(dir, jobInfo->get(JobInfo::AuxFileName));
   move(oldFile.filePath(), newFile.filePath());

   {//  New fchk file name
      QString fileName(jobInfo->get(JobInfo::InputFileName));
      fileName += ".fchk";
      oldFile.setFile(dir, fileName);
      move(oldFile.filePath(), newFile.filePath());
   }

   oldFile.setFile(dir, "plot.esp");
   newFile.setFile(dir, jobInfo->get(JobInfo::EspFileName));
   move(oldFile.filePath(), newFile.filePath());

   oldFile.setFile(dir, "plot.mo");
   newFile.setFile(dir, jobInfo->get(JobInfo::MoFileName));
   move(oldFile.filePath(), newFile.filePath());

   oldFile.setFile(dir, "plot.hf");
   newFile.setFile(dir, jobInfo->get(JobInfo::DensityFileName));
   move(oldFile.filePath(), newFile.filePath());

   // Get an updated time, first we check for PBS output
   QString pbs(grep("elapsed time", errFile.filePath()));
   if (!pbs.isEmpty()) {
      QStringList tokens(pbs.split(QRegExp("\\s+"), QString::SkipEmptyParts));
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
          QStringList tokens((*iter).split(QRegExp("\\s+"), QString::SkipEmptyParts));
          tokens = tokens.filter("(wall)");
          if (!tokens.isEmpty()) {
             bool ok;
             double s(tokens.first().remove("s(wall),").toDouble(&ok));
             if (ok) m_time += (int)s;
             qDebug() << "  time += " << s;
          }
      }
   }

   // Check for any errors reported in the output
   Threaded* thread = m_hostDelegate->checkOutputForErrors(outFile.filePath());
   runThread(thread);
   m_runTimeError = thread->outputMessage();
   thread->deleteLater();

   // If none, check for PBS errors in the error file
   if (m_runTimeError.isEmpty()) {
      m_runTimeError = grep("terminate", errFile.filePath());
      if (m_runTimeError.isEmpty()) m_runTimeError = grep("what()", errFile.filePath());
   }
      
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
   QLOG_DEBUG() << "Kill command" << cmd;
   exec(cmd);
}


// --------------------------------


// This assumes the Server is a) Remote and b) *nix based.
void CopyResults::run()
{
   m_process->setCopyActive(true);
   JobInfo* jobInfo(m_process->jobInfo());

   QFileInfo source;
   QFileInfo destination;
   QDir  localDir(jobInfo->get(JobInfo::LocalWorkingDirectory));
   QDir remoteDir(jobInfo->get(JobInfo::RemoteWorkingDirectory));

   int  kbToCopy(0);
   bool ok(true);
   QString cmd;

   QStringList copyList;
   QStringList fileList;
   fileList.append(jobInfo->get(JobInfo::InputFileName));
   fileList.append(jobInfo->get(JobInfo::OutputFileName));
   fileList.append(jobInfo->get(JobInfo::AuxFileName));
   fileList.append(jobInfo->get(JobInfo::ErrorFileName));
   fileList.append(jobInfo->get(JobInfo::EspFileName));
   fileList.append(jobInfo->get(JobInfo::MoFileName));
   fileList.append(jobInfo->get(JobInfo::DensityFileName));

   // First check the files exist and, if so, their sizes
   QStringList::iterator iter;
   for (iter = fileList.begin(); iter != fileList.end(); ++iter) {
       source.setFile(remoteDir, *iter);

       cmd = "test -f " + source.filePath() + " && du -k " + source.filePath();
       cmd = exec(cmd);
qDebug() << "cmd returned" << cmd;
       if (!cmd.isEmpty()) {
          cmd = cmd.split(QRegExp("\\s+"), QString::SkipEmptyParts).first();
qDebug() << "  first token" << cmd;
          int n = cmd.toInt(&ok);
          if (ok) {
             kbToCopy += n;
             copyList.append(*iter);
          }
       }
   }

   if (copyList.isEmpty() || kbToCopy == 0) {
      m_errorMessage += "No files found to copy";
      m_process->setCopyActive(false);
      return;
   }

   m_process->setCopyTarget(kbToCopy);

   // Now copy each of the files.
   bool allOk(true);
   for (iter = copyList.begin(); iter != copyList.end(); ++iter) {
       source.setFile(remoteDir, *iter);
       destination.setFile(localDir, *iter);
       Threaded* thread = m_hostDelegate->pull(source.filePath(), destination.filePath());
       connect(thread, SIGNAL(copyProgress()), m_process, SLOT(copyProgress()));
       runThread(thread);
       ok = thread->errorMessage().isEmpty();
       allOk = allOk && ok;
       if (!ok) m_errorMessage += "Failed to copy file " + source.filePath() + "\n";
       thread->deleteLater();
   }

   jobInfo->localFilesExist(allOk);
   m_process->setCopyActive(false);
}


} } // end namespace IQmol::ServerTask
