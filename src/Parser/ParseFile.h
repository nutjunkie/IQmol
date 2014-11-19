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
#include "Bank.h"
#include <QStringList>


namespace IQmol {

class JobInfo;

namespace Parser {

   class Base;

   /// This is the main file parsing class that parses files in a separate
   /// thread, allowing the sub-parsers to perform some computational work 
   /// if required.  If a directory is passed to the constructor, the
   /// directory is searched for all files with the same base name as the
   /// directory.  For example, if the directory is ~/Ethane, then we look
   /// for all files of the form ~/Ethane/Ethane.* 
   class ParseFile : public Task {

      Q_OBJECT 

      public:
         /// Constructs a ParseFile object for the given path.  Note that no
         /// parsing actually takes place untilt the start() member function
         /// (inherited from Task) is called.
         ParseFile(QString const& filePath, QString const& filter = QString());

		 /// Returns the file path passed to the constructor (either an actual
		 /// file or directory).
         QString filePath() { return m_filePath; }

         QString name() { return m_name; } 

		 /// Returns the composite data found in the file(s).
         Data::Bank& data() { return m_dataBank; } 

         /// Returns a list of errors encountered when processing the file(s).
         QStringList const& errors() const { return m_errorList; }

      protected:
         void run();

      private:
         /// searches a directory for all the files matching filter, if
         /// specified, otherwise uses the directory name to determine
         /// what files to parse.
         void parseDirectory(QString const& path, QString const& filter);

         /// Parses a file, returning false only if the file doesn't exist.
         bool parse(QString const& filePath, bool& addToFileList);

         /// Convenience function that runs a sub-parser over the file.
         void runParser(Base* parser, QString const& filePath);

         QString     m_name;
         QString     m_filePath;
         Data::Bank  m_dataBank;
         QStringList m_filePaths;
         QStringList m_errorList;
   };

} } // end namespace IQmol::Parser

#endif
