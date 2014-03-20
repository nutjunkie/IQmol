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

#include "OpenBabelParser.h"
#include "FileLayer.h"
#include "QMsgBox.h"
#include "openbabel/plugin.h"
#include "QsLog.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <QDebug>

namespace IQmol {
namespace Deprecate {


Layer::List ParseFiles(QStringList const& fileNames)
{
  Layer::List dataList;
  QStringList::const_iterator file;
  for (file = fileNames.begin(); file != fileNames.end(); ++file) {
      dataList += ParseFile(*file);
  }

  return dataList;
}


Layer::List Base::parseFile(QString const& fileName)
{
   QFile file(fileName);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) throw ReadError();
   QTextStream textStream(&file);
   parse(textStream);
   file.close();
   return m_dataList;
}


Layer::List ParseFile(QString const& fileName) 
{
   Layer::List dataList;

   QFileInfo info(fileName);
   if (!info.exists()) return dataList;

   try {
      std::vector<std::string> formats;
      if (::OpenBabel::OBPlugin::ListAsVector("formats", 0, formats)) {
         QString ext(info.suffix());
         std::vector<std::string>::iterator iter;
         for (iter = formats.begin(); iter != formats.end(); ++iter) {
             QString fmt(QString::fromStdString(*iter));
             fmt = fmt.split(QRegExp("\\s+"), QString::SkipEmptyParts).first();
             if (ext.contains(fmt, Qt::CaseInsensitive)) {
                qDebug() << "Running OBparser on file " << fileName;
                QLOG_DEBUG() << "Running OBparser on file " << fileName;
                OpenBabel obParser;
                dataList += obParser.parseFile(fileName);
                break;
             }
         }
      }

   } catch (IOError& ioerr) {
       QMsgBox::warning(0, "IQmol", ioerr.what());
   } catch (std::exception& e) {
       QMsgBox::warning(0, "IQmol", e.what());
   }

   return dataList;
}

} } // end namespace IQmol::Deprecate
