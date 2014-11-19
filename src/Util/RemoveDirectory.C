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

#include "RemoveDirectory.h"
#include "QMsgBox.h"
#include <QDir>
#include <QFileInfo>


namespace IQmol {
namespace Util {

bool RemoveDirectory(QString const& dirName)
{
   bool result = true;
   QDir dir(dirName);

   if (dir.exists(dirName)) {
      Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot |
         QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
         if (info.isDir()) {
            result = RemoveDirectory(info.absoluteFilePath());
         }else {
            result = QFile::remove(info.absoluteFilePath());
         }
         if (!result) return result;
     }
     result = dir.rmdir(dirName);
   }
   return result;
}

} }  // end namespace IQmol::Util
