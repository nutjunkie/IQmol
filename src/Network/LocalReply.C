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

#include "LocalReply.h"
#include "LocalConnection.h"
#include "QsLog.h"
#include <QFileInfo>
#include <QDir>


namespace IQmol {
namespace Network {

LocalExecute::LocalExecute(LocalConnection* connection, QString const& command, 
   QString const& workingDirectory) : LocalReply(connection), m_command(command)
{
   m_timer.setSingleShot(true);
   m_timer.setInterval(connection->timeout());
   if (workingDirectory.isEmpty()) {
      m_process.setWorkingDirectory(QDir::currentPath());
   }else {
      m_process.setWorkingDirectory(workingDirectory);
   }
}


LocalExecute::~LocalExecute()
{
   switch (m_process.state()) {
      case QProcess::NotRunning:
         break;
      case QProcess::Starting:
      case QProcess::Running:
         m_process.kill();
         break;
   }
}


void LocalExecute::run()
{
   m_status = Error;
   QStringList arguments;
   QStringList list(m_command.split("\"", QString::SkipEmptyParts));

   for (int i = 0; i < list.size(); ++i) {
       if (i % 2 == 0) {
          arguments << list[i].split(QRegExp("\\s+"), QString::SkipEmptyParts);
       }else {
          arguments << list[i];
       }   
   }   

   if (arguments.isEmpty()) {
      m_message = "Cannot execute null command";
      finished();
      return;
   }   

   // We assume the first token is the command
   QString command(arguments.takeFirst());
   QFileInfo cmd(command);

   if (!cmd.exists()) {
      m_message = "Command not found: " + command;
      finished();
      return;
   }

   QLOG_DEBUG() << "Executing command" << command << "with args:" << arguments;

   if (!cmd.isExecutable()) {
      QFile file(command);
#if QT_VERSION < 0x050000
#define QFileDevice  QFile
#endif

      QFileDevice::Permissions permissions(file.permissions());
      permissions = permissions | QFileDevice::ExeOwner | QFileDevice::ReadOwner 
                                | QFileDevice::ExeUser  | QFileDevice::ReadUser
                                | QFileDevice::ExeGroup | QFileDevice::ReadGroup
                                | QFileDevice::ExeOther | QFileDevice::ReadOther;

      if (!file.setPermissions(permissions)) {
         m_message = "Failed to set execute permission on file " + command;
         finished();
         return;
      }
   }


   connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)), 
      this, SLOT(runFinished(int, QProcess::ExitStatus)));

   connect(&m_process, SIGNAL(error(QProcess::ProcessError)), 
      this, SLOT(runError(QProcess::ProcessError)));

   connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));

   m_status = Running;

   m_process.start(cmd.filePath(), arguments);
qDebug() << "QProcess prog:" << m_process.program();
qDebug() << "QProcess args:" << m_process.arguments();
qDebug() << "QProcess pwd: " << m_process.workingDirectory();
qDebug() << "QProcess pid: " << m_process.processId();
   m_timer.start();
}


void LocalExecute::timeout()
{
   m_process.kill();
   m_message = "Timeout on local command: " + m_command;
   m_status = TimedOut;
   finished();
}


void LocalExecute::runFinished(int /* exitCode */, QProcess::ExitStatus status)
{
   m_timer.stop();
   switch (status) {
      case QProcess::NormalExit:
         m_message = m_process.readAllStandardOutput();
         m_status  = Finished;
         break;
      case QProcess::CrashExit:
         m_message = "Program " + m_command + " crashed";
         m_status  = Error;
         break;
   }

   qDebug() << "QProcess stderr:" << m_process.readAllStandardError();

   finished();
}


void LocalExecute::runError(QProcess::ProcessError error)
{
   m_timer.stop();
   QStringList list(m_command.split(QRegExp("\\s+"), QString::SkipEmptyParts));
   m_message = list.isEmpty() ? "Program" : list.first();

   switch(error) {
      case QProcess::FailedToStart: 
         m_message += " failed to start";
         break;
      case QProcess::Crashed: 
         m_message += " crashed";
         break;
      case QProcess::Timedout: 
         m_message += " timed out";
         break;
      case QProcess::WriteError: 
         m_message += " encountered a write error";
         break;
      case QProcess::ReadError: 
         m_message += " encountered a read error";
         break;
      case QProcess::UnknownError: 
         m_message += " encountered an unknown error";
         break;
   }

   m_status = Error;
   finished();
}


void LocalCopy::run()
{
   QFile file(m_destinationPath);

   if (file.exists() && !file.remove()) {
      m_message = "Could not overwrite file " + m_destinationPath;
      m_status  = Error;
   }else if (QFile::copy(m_sourcePath, m_destinationPath)) {
      m_status = Finished;
   }else {
      m_message  = "Failed to rename file:\n";
      m_message += m_sourcePath + " -> " + m_destinationPath;
      m_status  = Error;
   }
   finished();
}

} } // end namespace IQmol::Network
