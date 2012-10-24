#ifndef IQMOL_BASEPARSER_H
#define IQMOL_BASEPARSER_H
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

#include <exception>
#include "DataLayer.h"


class QTextStream;

namespace IQmol {
namespace Parser {

   /// The primary interface function for the Parser module - use this function
   /// to parse the contents of a file.  The function uses the file name to
   /// determine what parsers should have a chance to extract data from the file.  
   DataList ParseFile(QString const& fileName);

   /// This is also a primary interface function which parses several files
   /// which are assumed to relate to the same molecule (e.g. output and
   /// checkpoint files).
   DataList ParseFiles(QStringList const& fileNames);

   /// Abstract base class for file parsers.  Each derived class should handle
   /// a single file type and return the extracted data as a list of Layer::Data
   /// objects.  To handle a new file type, derive a new parser from this Base 
   /// class and add it to the ParseFile function in BaseParser.C
   class Base {

      friend DataList ParseFile(QString const&);

      public:
         Base() { }
         virtual ~Base() { }

		 /// This is the primary interface function used to parse the contents
		 /// of a file.  The default implementation performs basic checks on 
		 /// the file before handing the work off to the parse() function. 
		 virtual DataList parseFile(QString const& fileName);

         /// This function does the actual work of parsing the file contents 
         /// and must be reimplemented for a specific file type.
         virtual DataList parse(QTextStream&) = 0;

	     /// Controls whether or not any errors should be displayed to the user.
	     /// Call with false if the exceptions are to be handled internally.
         void displayErrors(bool tf) { s_displayErrors = tf; }


      protected:
         static bool s_displayErrors;
         DataList m_dataList;
   };


   /// Base class for errors that arise from reading and writing files.
   class IOError : public std::exception { };

   class ReadError : public IOError {
      public:
         virtual const char* what() const throw () {
            return "Could not open file for reading";
         }
   };

   class WriteError : public IOError {
      public:
         virtual const char* what() const throw () {
            return "Could not write to file";
         }
   };

   class ExtensionError : public IOError {
      public:
         virtual const char* what() const throw() {
            return "Could not determine file type from extension";
         }
   };

   class FormatError : public IOError {
      public:
         virtual const char* what() const throw() {
            return "File format inconsistent with extension";
         }
   };

} } // end namespace IQmol::Parser

#endif
