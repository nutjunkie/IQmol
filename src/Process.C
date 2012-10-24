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

#include "Process.h"
#include "QMsgBox.h"
#include "QsLog.h"
#include "FileLayer.h"
#include "Preferences.h"
#include "Server.h"
#include "ServerRegistry.h"
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QProcess>


namespace IQmol {

QString Process::toString(Status const& state) 
{
   QString s;

   switch (state) {
      case NotRunning:  s = "Not Running";  break;
      case Queued:      s = "Queued";       break;
      case Running:     s = "Running";      break;
      case Suspended:   s = "Suspended";    break;
      case Killed:      s = "Killed";       break;
      case Error:       s = "Error";        break;
      case Finished:    s = "Finished";     break;
      case Copying:     s = "Copying";      break;
      default:          s = "Unknown";      break;
   }
   return s;
}


Process::~Process()
{
  // Let the server know we don't exist any more.
   if (m_qprocess) delete m_qprocess;
   Server* server = ServerRegistry::instance().get(serverName());
   if (server) server->removeFromWatchList(this);
}


QVariant Process::serialize()
{
   Status status = (m_status == Copying) ? m_preCopyStatus : m_status;
   QList<QVariant> list;
   list << m_jobInfo->serialize();  // 0
   list << (int)status;             // 1
   list << m_comment;               // 2
   list << m_id;                    // 3
   list << m_submitTime;            // 4
   list << m_runTime;               // 5

   return QVariant(list);
}
 


Process* Process::deserialize(QVariant const& qvariant)
{
   QList<QVariant> list(qvariant.toList());
   bool ok = (list.size() == 6)            &&
           //list[0] is the JobInfo which we test separately
             list[1].canConvert<int>()     &&
             list[2].canConvert<QString>() &&
             list[3].canConvert<QString>() &&
             list[4].canConvert<QString>() &&
             list[5].canConvert<QString>();

   if (!ok) {
      QLOG_ERROR() << "Process deserialization error: Invalid format";
      return 0;
   }

   JobInfo* jobInfo(JobInfo::deserialize(list[0]));
   if (!jobInfo) {
      QLOG_ERROR() << "Process deserialization error: Invalid JobInfo";
      return 0;
   }

   int s(list[1].toInt(&ok));
   Status status;
   if (0 <= s && s <= Unknown) {
      status = (Status)s; 
   }else {
      QLOG_ERROR() << "Process deserialization error: Invalid status";
      return 0;
   }

   // Now that the JobInfo has loaded, check we have a valid server
   QString serverName(jobInfo->get(JobInfo::ServerName));
   Server* server = ServerRegistry::instance().get(serverName);
   if (!server) {
      QLOG_ERROR() << "Process deserialization error: Server " << serverName << " not found ";
      return 0;
   }

   Process* process = new Process(jobInfo);
   process->m_comment    = list[2].toString();
   process->m_id         = list[3].toString();
   process->m_submitTime = list[4].toString();
   process->m_runTime    = list[5].toString();
   process->m_status     = status;

   process->m_timer.reset(Timer::toSeconds(process->m_runTime));

   return process;
}


void Process::resetTimer(int const seconds)
{ 
   m_timer.reset(seconds); 
   m_runTime = m_timer.toString();
}


QStringList Process::monitorItems() const 
{
   QString status;
   QStringList items;
   items << name() << serverName() << m_submitTime;
   items << (m_timer.isRunning() ? m_timer.toString() : m_runTime);
   status = toString(m_status);
   if (m_status == Copying) {
      status += ": " + QString::number(int(100.0*m_copyProgress/m_copyTarget)) +"%";
   }
   items << status;
   if (!m_comment.isEmpty()) items << m_comment;
   return items;
}


void Process::setPID(unsigned int const pid)
{
   m_id = QString::number(pid);
   updated();
}


unsigned int Process::getPID() const
{
   bool ok;
   unsigned int pid(m_id.toUInt(&ok));
   if (!ok) pid = 0;
   return pid;
}


void Process::setStatus(Status const status)
{
   if (m_status == status) return;
   m_status = status;

   switch (m_status) {
      case NotRunning:
         break;
      case Queued:
         m_submitTime = QTime::currentTime().toString("h:mm:ss");
         break;
      case Running:
         m_timer.start();
         break;
      case Suspended:
         m_timer.stop();
         m_runTime = m_timer.toString();
         break;
      case Killed:
         m_timer.stop();
         m_runTime.clear();
         break;
      case Error:
         m_timer.stop();
         m_runTime = m_timer.toString();
         finished();
         break;
      case Finished:
         m_timer.stop();
         m_runTime = m_timer.toString();
         finished();
         break;
      case Copying:
         QLOG_ERROR() << "Process::setStatus called with Copying";
         break;
      case Unknown:
         m_timer.stop();
         m_runTime.clear();
         break;
   }

   updated();
}


bool Process::getLocalSaveDirectory()
{
   QDir dir(m_jobInfo->get(JobInfo::LocalWorkingDirectory));
   dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

   while (1) {
      QString dirName(dir.dirName());
      dir.cdUp();
      QString dirPath(dir.path());
      qDebug() << "LocalServer::getSaveDirectory() dirPath: " << dirPath;
      qDebug() << "LocalServer::getSaveDirectory() dirName: " << dirName;

      QFileDialog dialog(0, "Save As", dirPath);
      dialog.selectFile(dirName);
      dialog.setOption(QFileDialog::ShowDirsOnly, true);
      dialog.setFileMode(QFileDialog::Directory);

      if (dialog.exec() == QDialog::Rejected) return false;

      QStringList list(dialog.selectedFiles());
      if (list.isEmpty()) return false;

      dir.setPath(list.first());
      if (dir.path().isEmpty()) return false;

      if (dir.path().contains(" ")) {
         QMsgBox::warning(0, "IQmol", "Directory name cannot contain spaces");
      }else {
         if (dir.count() == 0) break;

         QString msg("Directory ");
         msg += dir.dirName() + " exists, overwrite?";

         if (QMsgBox::question(0, "IQmol", msg,
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) break;
      }
   }

   QString dirPath(dir.path());
   if (dirPath.endsWith("/")) dirPath.remove(dirPath.length()-1,1);
   m_jobInfo->set(JobInfo::LocalWorkingDirectory, dirPath);

   QFileInfo info(dir, m_jobInfo->get(JobInfo::InputFileName));
   Preferences::LastFileAccessed(info.filePath());

   return true;
}


void Process::viewOutput()
{
   if (m_status == Process::NotRunning || 
       m_status == Process::Queued     ||
      !m_jobInfo->localFilesExist()) return;

   QDir dir(m_jobInfo->get(JobInfo::LocalWorkingDirectory));
   QFileInfo output(dir, m_jobInfo->get(JobInfo::OutputFileName));

   if (!output.exists()) {
      QMsgBox::warning(0,"IQmol", "Output file no longer exists");
      m_jobInfo->localFilesExist(false);
      return;
   }

   Layer::File* file = new Layer::File(output.filePath());

   if (m_status == Process::Running || m_status == Process::Suspended) {
      file->tail();
   }else {
      file->configure();
   }
}


void Process::setCopyActive(bool const tf)
{
   if (tf) {
      m_copyProgress = 0;
      m_preCopyStatus = m_status;
      m_status = Copying;
   }else {
      m_status = m_preCopyStatus;
   }
   updated();
}


void Process::setCopyTarget(int kbBlocks)
{
   m_copyTarget = kbBlocks;
   qDebug() << "Setting copy target to"<< m_copyTarget << "blocks";
}


void Process::copyProgress()
{
   ++m_copyProgress;
   if (m_copyProgress > m_copyTarget) m_copyProgress = m_copyTarget;
}


// ------------ These are only applicable to local processes ------------

void Process::setQProcess(QProcess*)
{
qDebug() << "setting qprocess for process";
// these signals do not seem to get picked up.
/*
   m_qprocess = qprocess;
   connect(qprocess, SIGNAL(finished(int, QProcess::ExitStatus)),
      this, SLOT(qprocessFinished(int, QProcess::ExitStatus)));
   connect(qprocess, SIGNAL(error(QProcess::ProcessError)),
      this, SLOT(qprocessError(QProcess::ProcessError)));
*/
}

/*
void Process::qprocessFinished(int, QProcess::ExitStatus exitStatus)
{
qDebug() << "**************** Local Process finished **************** " << name();
   if (exitStatus == QProcess::NormalExit) {
      setStatus(Process::Finished);
      //cleanUp(this, exitStatus == QProcess::CrashExit);
   }else {
      setStatus(Process::Error);
       // need to determine error
      //cleanUp(this, exitStatus == QProcess::CrashExit);
   }
}


void Process::qprocessError(QProcess::ProcessError error)
{
qDebug() << "**************** Local Process error **************** " << name();
   switch (error) {
      case QProcess::FailedToStart: {
         setComment("Process failed to start");
      } break;
      case QProcess::Crashed: {
         setComment("Process crashed");
      } break;
      case QProcess::Timedout: {
         setComment("Process timed out");
      } break;
      case QProcess::WriteError: {
         setComment("Could not write to process");
      } break;
      case QProcess::ReadError: {
         setComment("Could not read from process");
      } break;
      default: {
         setComment("Unknown error occured");
      } break;
   }

   setStatus(Process::Error);
}
*/

/*
void LocalServer::cleanUp(Process* process, bool qError)
{
   QString err(checkForErrors(process->m_jobInfo));
   renameFchkFile(process->m_jobInfo);

   if (!qError && err.isEmpty()) {
      process->setStatus(Process::Finished);
   }else {
      if (err.isEmpty()) {
         setComment("Process Error");
      }else {
         setComment(err);
      }
      process->setStatus(Process::Error);
   }

   m_activeProcesses.remove(process);
   runQueue();
}





*/



} // end namespaces IQmol
