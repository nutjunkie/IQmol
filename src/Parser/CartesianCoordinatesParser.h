#ifndef IQMOL_PARSER_CARTESIANCOORDINATES_H
#define IQMOL_PARSER_CARTESIANCOORDINATES_H
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

#include <climits>
#include <QString>


namespace IQmol {

namespace Data {
   class Geometry;
}

namespace Parser {

   class TextStream;

   /// Parses a single list of cartesian coordinates of the format:
   ///
   ///    atom(int|string)  x(float)  y(float)  z(float)
   /// or 
   ///    index(int)  atom(int|string)  x(float)  y(float)  z(float)
   ///
   /// Atoms can be either atomic numbers or symbols.  Empty lines are ignored,
   /// as are any tokens beyond the 5th.  The parser stops when it reaches an
   /// invalid line, but does not issue a warning message.  The last (invalid)
   /// line has to be read from the TextStream, but can be accessed using the
   /// previousLine() function.  We do not derive from Parser::Base as this does
   /// not correspond to a file type.
   class CartesianCoordinates {
      public:
		 /// Optional argument max limits the number of coordinates that are
		 /// read in.  This may be useful if the TextStream containts a 
         /// concatenation of several geometries of the same molecule.
         CartesianCoordinates(int max = INT_MAX) : m_max(max) { }

         Data::Geometry* parse(TextStream&);
         QString const& error() const { return m_error; }

      private:
         int m_max;
         QString m_error;
   };

} } // end namespace IQmol::Parser

#endif
