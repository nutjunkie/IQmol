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

#include "EfpFragmentLibrary.h"
#include "EfpFragmentParser.h"
#include "ScanDirectory.h"
#include "Preferences.h"
#include "Geometry.h"
#include <QDebug>


namespace IQmol {
namespace Data {

QMap<QString, Geometry*> EfpFragmentLibrary::s_geometries = QMap<QString, Geometry*>();
QMap<QString, QString>   EfpFragmentLibrary::s_parameters = QMap<QString, QString>();
EfpFragmentLibrary* EfpFragmentLibrary::s_instance = 0;


EfpFragmentLibrary& EfpFragmentLibrary::instance()
{
   if (s_instance == 0) {
      s_instance = new EfpFragmentLibrary();
      s_geometries.insert("empty", new Geometry);
      atexit(EfpFragmentLibrary::checkOut);
   }
   return *s_instance;
}


void EfpFragmentLibrary::checkOut()
{
   QMap<QString, Geometry*>::iterator iter;
   for (iter = s_geometries.begin(); iter != s_geometries.end(); ++iter) {
       delete iter.value();
   }
}


void EfpFragmentLibrary::add(QString const& fragmentName, Geometry* geometry, 
   QString const& params)
{
   if (isLoaded(fragmentName)) {
      qDebug() << "Attempt to overwrite EFP fragment data in library:" << fragmentName;
   }else {
      QString name(fragmentName.toLower());
      s_geometries.insert(name, geometry);
      s_parameters.insert(name, params);
   }
}


bool EfpFragmentLibrary::add(QString const& fragmentName)
{
   if (isLoaded(fragmentName)) return true;

   QString filePath(getFilePath(fragmentName));
   Parser::EfpFragment parser;
   // This relies on the side effect of the parser, which inserts any parsed
   // EFP fragments into the library.
   parser.readFragment(filePath);
   return isLoaded(fragmentName);
}


bool EfpFragmentLibrary::defined(QString const& fragmentName)
{
   if (isLoaded(fragmentName)) return true;
   return add(fragmentName);
}


bool EfpFragmentLibrary::isLoaded(QString const& fragmentName) const
{
   return s_geometries.contains(fragmentName.toLower());
}


QString EfpFragmentLibrary::getFilePath(QString const& fragmentName)
{
   QString filePath;
   QDir dir(Preferences::FragmentDirectory());
   if (dir.exists() && dir.cd("EFP")) filePath = Util::ScanDirectory(dir, fragmentName);
   return filePath;
}


Geometry const& EfpFragmentLibrary::geometry(QString const& name)
{
   QString tmp(name.toLower());
   if (!defined(name)) tmp = "empty";
   return *s_geometries.value(tmp);
   
}


void EfpFragmentLibrary::dump() const
{
   QMap<QString, QString>::iterator iter;
   qDebug() << "EFP Fragment Library contents:";
   for (iter = s_parameters.begin(); iter != s_parameters.end(); ++iter) {
       if (iter.value().isEmpty()) {
          qDebug() << iter.key() << "parameters are from library file";
       }else {
          qDebug() << iter.key() << "parameters are given explicitly";
       }
   }
}

} } // end namespace IQmol::Data
