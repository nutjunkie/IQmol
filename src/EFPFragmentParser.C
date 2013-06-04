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

#include "OpenBabelParser.h"
#include "EFPFragmentParser.h"
#include <QTextStream>
#include <QtDebug>


using namespace OpenBabel;

namespace IQmol {
namespace Parser {

DataList EFPFragment::parse(QTextStream& textStream)
{
   while (!textStream.atEnd() && !m_done) {
      processLine(textStream);
   }

   return m_dataList;
}


void EFPFragment::processLine(QTextStream& textStream)
{
   QString line = textStream.readLine().trimmed();

   if (line.contains("fragment", Qt::CaseInsensitive)) {
      QStringList tokens(line.split(QRegExp("\\s+"), QString::SkipEmptyParts));
      if (tokens.size() >= 2) m_name = tokens[1];
      readCoordinates(textStream);
   }

   m_done = true;
}


void EFPFragment::readCoordinates(QTextStream& textStream)
{
   QString line;
   QStringList lines;

   while (!textStream.atEnd()) {
      line = textStream.readLine();
      if (line.contains("mult", Qt::CaseInsensitive)) break;
      lines.append(line);
   }

   int n(lines.size());
   if (n == 0) return;

   line = lines.join("\n");
   line = QString::number(n) + "\n\n" + line;

   OpenBabel obParser;
   QTextStream s(&line);
   m_dataList = obParser.parse(s);
}


} } // end namespace IQmol::Parser
