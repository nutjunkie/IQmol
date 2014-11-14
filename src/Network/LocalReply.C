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

#include "LocalReply.h"
#include "LocalConnection.h"
#include <QFileInfo>
#include <QProcess>
#include <QDir>


namespace IQmol {
namespace Network {

void LocalExecute::run()
{
   m_status = Running;
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
      m_status  = Error;
      m_message = "Cannot execute null command";
      return;
   }   

   // We assume the first token is the command
   QString command(arguments.takeFirst());
   QFileInfo cmd(command);

   qDebug() << "Executing command" << command << "with args:" << arguments;

   if (cmd.exists()) {
      QProcess process;
      process.setWorkingDirectory(cmd.absolutePath());
      process.start(cmd.fileName(), arguments);

      if (process.waitForFinished(m_connection->timeout())) {
         m_message = process.readAllStandardOutput();
         m_status  = Finished;
      }else {
         process.kill();
         m_status  = TimedOut;
         m_message = "Timeout on command return: " + command;
      }   

   }else {
      m_status  = Error;
      m_message = "Command not found: " + command;
   }   
}


void LocalCopy::run()
{
   qDebug() << "LocalCopy::run() called to copy" << m_sourcePath << "->" << m_destinationPath;
}

} } // end namespace IQmol::Network
