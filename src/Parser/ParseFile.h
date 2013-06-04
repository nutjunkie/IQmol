#ifndef IQMOL_PARSER_PARSEFILE_H
#define IQMOL_PARSER_PARSEFILE_H
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

#include "Task.h"
#include <QStringList>


namespace IQmol {

namespace Data {
   class Bank;
}

namespace Parser2 {

   class Base;

   /// This is the main file parsing class that parses files in a separate
   /// thread allowing the sub-parsers to perform some computational work 
   /// if required.
   class ParseFile : public Task {

      Q_OBJECT 

      public:
         ParseFile(QStringList filePaths);
         ParseFile(QString filePath);

		 /// Returns the composite data found in the file(s).  Note that the
		 /// caller takes ownership of the Data::Bank and this will return a
		 /// null pointer if called more than once.
         Data::Bank* takeData();

         /// Returns a list of errors encountered when processing the file(s).
         QStringList const& errors() const { return m_errorList; }

      protected:
         void run();

      private:
         static bool obSupported(QString const& extension);
         static QStringList s_obFormats;

         /// Parses the file, returning false only if the file doesn't exist.
         bool parse(QString const& filePath);
         void runParser(Base* parser, QString const& filePath);
         Data::Bank* m_dataBank;
         QStringList m_filePaths;
         QStringList m_errorList;
   };

} } // end namespace IQmol::Parser

#endif
