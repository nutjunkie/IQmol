#ifndef IQMOL_PARSER_EXTERNALCHARGE_H
#define IQMOL_PARSER_EXTERNALCHARGE_H
/*******************************************************************************
       
  Copyright (C) 2011 Andrew 2015ert
           
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


namespace IQmol {
namespace Parser {

   /// Parses a list of external charges with the format:
   ///    x(float)  y(float)  z(float)   charge(float)  
   /// Optionally the first line can contain a single integer with the total
   /// number of charges.  Empty lines are ignored as are any tokens beyond the
   /// 4th.  The parser stops when it reaches an the end or an invalid line, but
   /// a warning is only issued if the expected number of charges is not found.  
   /// The last (invalid) line has to be read from the TextStream, but can be
   /// accessed using the previousLine() function.
   class ExternalCharges : public Base {
      public:
         bool parse(TextStream&);
   };

} } // end namespace IQmol::Parser

#endif
