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

#include "LocalHost.h"
#include "BasicServer.h"
#include "LocalConnectionThread.h"
#include "QMsgBox.h"
#include "Preferences.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>


namespace IQmol {


bool LocalHost::getWorkingDirectoryFromUser(JobInfo* jobInfo)
{
   bool success(getSaveDirectory(jobInfo));
   if (success) {
      QDir dir(jobInfo->get(JobInfo::LocalWorkingDirectory));
      jobInfo->set(JobInfo::BaseName, dir.dirName());
      jobInfo->promptOnOverwrite(false);
   }
   return success;
}


bool LocalHost::getSaveDirectory(JobInfo* jobInfo)
{
   QDir dir(jobInfo->get(JobInfo::LocalWorkingDirectory));
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

      if (dir.dirName().contains(" ")) {
         QMsgBox::warning(0, "IQmol", "Directory name cannot contain spaces");
      }else {
         if (dir.count() == 0) break;

         QString msg("Directory ");
         msg += dir.dirName() + " exists, overwrite?";

         if (QMsgBox::question(0, "IQmol", msg,
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) break;
      }
   }

   QString dirPath(dir.path());
   if (dirPath.endsWith("/")) dirPath.remove(dirPath.length()-1,1);
   jobInfo->set(JobInfo::LocalWorkingDirectory, dirPath);

   QFileInfo info(dir, jobInfo->get(JobInfo::InputFileName));
   Preferences::LastFileAccessed(info.filePath());

   return true;
}


Threaded* LocalHost::exec(QString const& command)
{
   return new LocalConnection::Exec(command);
}


Threaded* LocalHost::exists(QString const& file, HostDelegate::FileFlags const flags)
{
   return new LocalConnection::Exists(file, flags);
}



Threaded* LocalHost::mkdir(QString const& path)
{
   return new LocalConnection::MakeDirectory(path);
}


Threaded* LocalHost::push(QString const& sourcePath, QString const& destinationPath)
{
   return new LocalConnection::Move(sourcePath, destinationPath);
}


Threaded* LocalHost::pull(QString const& sourcePath, QString const& destinationPath)
{
   return new LocalConnection::Move(sourcePath, destinationPath);
}


Threaded* LocalHost::move(QString const& sourcePath, QString const& destinationPath)
{
   return new LocalConnection::Move(sourcePath, destinationPath);
}


Threaded* LocalHost::remove(QString const& filePath)
{
   return new LocalConnection::Remove(filePath);
}


Threaded* LocalHost::grep(QString const& string, QString const& filePath)
{
   return new LocalConnection::Grep(string, filePath);
}


Threaded* LocalHost::checkOutputForErrors(QString const& filePath)
{
   return new LocalConnection::CheckOutputForErrors(filePath);
}

} // end namespace IQmol

