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

#include "QChemOutputParser.h"
#include "TextStream.h"
#include "LocalConnectionThread.h"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>


namespace IQmol {
namespace LocalConnection {

void Exec::run()
{
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
      m_errorMessage = "Cannot execute null command";
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

      if (process.waitForFinished(m_timeout)) {
         m_outputMessage = process.readAllStandardOutput();
      }else {
         process.kill();
         m_errorMessage = "Process timed out " + m_command;
      }

   }else {
      m_errorMessage = "Command not found " + command;
   }
}


void Exists::run()
{
   QFileInfo file(m_filePath);
   bool exists(true);
   if (m_flags & HostDelegate::Directory) exists = exists && file.isDir();
   if (m_flags & HostDelegate::Readable) exists = exists && file.isReadable();
   if (m_flags & HostDelegate::Writable) exists = exists && file.isWritable();
   if (m_flags & HostDelegate::Executable) exists = exists && file.isExecutable();
   if (exists) m_outputMessage = "exists";
}
   

void Move::run()
{
   QFile source(m_sourceFilePath);
   if (!source.exists()) {
      m_errorMessage = "File not found " + m_sourceFilePath;
      return; 
    }

   QFile destination(m_destinationFilePath);
   if (destination.exists()) {
      if (!destination.remove()) {
         m_errorMessage = "Could not remove file " + m_destinationFilePath;
         return; 
       }
   }

   if (!QFile::rename(m_sourceFilePath, m_destinationFilePath)) {
      m_errorMessage = "Could not rename file " + m_sourceFilePath 
        + " to " + m_destinationFilePath;
   }
}


void Remove::run()
{
   QFile file(m_filePath);
   if (file.exists()) {
      if (!file.remove()) {
         m_errorMessage = "Could not remove file " + m_filePath;
      }
   }
}


void Grep::run()
{
   QFile file(m_filePath); 
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      m_errorMessage = "Failed to open file: " + m_filePath;
      return;
   }
   
   char buffer[1024];
   qint64 lineLength;

   while (1) {
      lineLength = file.readLine(buffer, sizeof(buffer));
      if (lineLength == -1) break;
      QString line(buffer);
      if (line.contains(m_string, Qt::CaseInsensitive)) m_outputMessage += line;
   }
   
   file.close();
}


void CheckOutputForErrors::run()
{
   QFile file(m_outputFilePath);

   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      Parser::TextStream output(&file);
      Parser::QChemOutput parser;
      m_outputMessage = parser.parseForErrors(output).join("\n");
      file.close();
   }else {
      m_errorMessage = "Failed to open output file for error checking " + m_outputFilePath;
   }
}


void MakeDirectory::run()
{
   QDir dir(QDir::root());
   if (!dir.mkpath(m_path)) {
      m_errorMessage = "Failed to make local directory: " + m_path;
   }
}
 
} } // end namespce IQmol::LocalConnection
