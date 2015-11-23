/*******************************************************************************
       
  Copyright (C) 2011 Andrew 2015ert
           
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

#include "WriteToTemporaryFile.h"
#include <QTemporaryFile>
#include <QFileInfo>
#include <QDir>


namespace IQmol {
namespace Util {

QString WriteToTemporaryFile(QString const& contents)
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

} } // end namespace IQmol::Util
