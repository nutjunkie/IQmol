/*******************************************************************************

  Copyright (C) 2011-2015ndrew Gilbert
 
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

#include "Parser.h"
#include "TextStream.h"
#include <QFile>


namespace IQmol {
namespace Parser {

bool Base::parseFile(QString const& filePath)
{
   m_filePath = filePath;
   QFile file(m_filePath);
   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      TextStream textStream(&file);
      parse(textStream);
      file.close();
   }else {
      QString msg("Failed to open file for reading: ");
      msg += m_filePath;
      m_errors.append(msg);
   }

   return m_errors.isEmpty();
}

} } // end namespace IQmol::Parser
