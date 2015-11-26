#ifndef IQMOL_PARSER_H
#define IQMOL_PARSER_H
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

#include "Bank.h"

namespace IQmol {
namespace Parser {

   class TextStream;

   class Base {

      public:
         Base() { }
         virtual ~Base() { }

		 /// The default implementation performs basic checks on the file
		 /// bfore handing the work off to the parse() function. Returns
         /// true only if no errors were encountered.
		 virtual bool parseFile(QString const& filePath);

         /// This function does the actual work of parsing the file contents 
         /// and must be reimplemented for a specific file type.  Returns
         /// true only if no errors were encountered.
         virtual bool parse(TextStream&) = 0;

         QStringList const& errors() const { return m_errors; } 
         Data::Bank& data() { return m_dataBank; }

      protected:
         QString     m_filePath;
         QStringList m_errors;
         Data::Bank  m_dataBank;
   };

} } // end namespace IQmol::Parser

#endif
