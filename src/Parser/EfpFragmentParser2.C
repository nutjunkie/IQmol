/*******************************************************************************
       
  Copyright (C) 2011-13 Andrew Gilbert
           
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

#include "EfpFragmentParser2.h"
#include "CartesianCoordinatesParser.h"
#include "Geometry.h"
#include "EfpFragment.h"
#include "EfpFragmentLibrary.h"
#include "TextStream.h"
#include <QFile>

#include <QtDebug>


namespace IQmol {
namespace Parser2 {

Data::Bank& EfpFragment::parse(TextStream& textStream)
{
   QString name;
   while (!textStream.atEnd()) {
      name = loadNextFragment(textStream);
      if (!name.isEmpty()) {
         m_dataBank.append(new Data::EfpFragment(name));
      }
      if (textStream.previousLine().contains("$end", Qt::CaseInsensitive)) break;
   }
   return m_dataBank;
}


QString EfpFragment::readFragment(QString const& filePath)
{
   QFile file(filePath);
   QString name;
   
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QString msg("Failed to find fragment file for read: ");
      msg += filePath;
      m_errors.append(msg);
   }else {
      TextStream textStream(&file);
      name = loadNextFragment(textStream);
      file.close();
   }

   return name;
}


QString EfpFragment::loadNextFragment(TextStream& textStream)
{
   QString line(textStream.previousLine()); 
   if (!line.contains("fragment", Qt::CaseInsensitive)) {
      line = textStream.seek("fragment", Qt::CaseInsensitive);
   }

   QStringList tokens(TextStream::tokenize(line));
   QString name;

   if (tokens.size() >= 2 && tokens[0].toLower() == "fragment") {
      name = tokens[1];
   }else {
      return QString();
   }

   CartesianCoordinates parser;
   Data::Geometry* geom(parser.parse(textStream)); 
   if ( !geom || !parser.error().isEmpty()) {
      m_errors.append("Invalid EFP file, no coordinates found");
      delete geom;
      return QString();
   }

   if (name.endsWith("_L", Qt::CaseInsensitive)) {
      Data::EfpFragmentLibrary::instance().add(name, geom);
   }else {
	  // We assume the fragment is not in the library and so we need to collect
	  // the additional guff so we can add it to the input file if required.
      QString line;
      QStringList guff;
      while ( !textStream.atEnd() && 
         !line.contains("fragment", Qt::CaseInsensitive) &&
         !line.contains("$end", Qt::CaseInsensitive) ) {
         guff << line;
      }
      Data::EfpFragmentLibrary::instance().add(name, geom, guff.join("\n"));
   }

   return name;
}

} } // end namespace IQmol::Parser
