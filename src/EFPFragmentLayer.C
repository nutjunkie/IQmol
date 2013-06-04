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

#include "EFPFragmentLayer.h"
#include "EFPFragmentParser.h"
#include "QMsgBox.h"
#include "Preferences.h"
#include "IQmol.h"
#include <QFileInfo>
#include <QFile>
#include <QSet>
#include <cmath>

#include <QDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {

QMap<QString, QString> EFPFragment::s_parameterFiles;

QString EFPFragment::efpParamsSection(QSet<QString> const& fragmentNames)
{
   QString parameters;

   QSet<QString>::const_iterator iter;
   for (iter = fragmentNames.begin(); iter != fragmentNames.end(); ++iter) {

       // Don't include parameters for the internal library fragments
       if (!(*iter).endsWith("_L", Qt::CaseInsensitive)) {
          bool okay(false);
          if (s_parameterFiles.contains(*iter)) {
             QFile file(s_parameterFiles.value(*iter));
             if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                parameters += QString(file.readAll()); 
                file.close();
                okay = true;
             }
          }

          if (!okay) {
             QString msg("Parameters for fragment ");
             msg +=  *iter + " not found";
             QMsgBox::warning(0, "IQmol", msg);
          }
      }
   }

   return parameters;
}


EFPFragment::EFPFragment(QString const& filePath) : Group("EFPFragment")
{
   loadFromFile(filePath);
   QFileInfo info(filePath);
   QString name(info.completeBaseName());
   s_parameterFiles.insert(name, filePath);
}


QString EFPFragment::format(Mode const mode)
{
   QString format;
   if (mode == Name || mode == NameAndFrame) format = text();

   if (mode == Frame || mode == NameAndFrame) {
      Vec position(getPosition());
      Vec euler(ToEulerAngles(getOrientation()));
      format += " " + QString::number(position.x, 'f', 6);
      format += " " + QString::number(position.y, 'f', 6);
      format += " " + QString::number(position.z, 'f', 6);
      format += " " + QString::number(euler.x, 'f', 6);
      format += " " + QString::number(euler.y, 'f', 6);
      format += " " + QString::number(euler.z, 'f', 6);
   }
   return format;
}


QString EFPFragment::getFilePath(QString const& fragmentName)
{
   QString filePath;
   QDir dir(Preferences::FragmentDirectory()); 
   if (dir.exists() && dir.cd("EFP")) filePath = scanDirectory(dir, fragmentName);
   if (!filePath.isEmpty()) s_parameterFiles.insert(fragmentName, filePath);
   return filePath;
}


QString EFPFragment::scanDirectory(QDir const& dir, QString const& fragmentName)
{
   QString filePath;
   QFileInfoList list(dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
   QFileInfoList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       if ((*iter).isDir()) {
          filePath = scanDirectory((*iter).filePath(), fragmentName);
          if (!filePath.isEmpty()) break;
       }else if ( (*iter).fileName().contains(fragmentName)){
          filePath = (*iter).filePath();
          break;
       }
   }

   return filePath;
}

} } // end namespace IQmol::Layer
