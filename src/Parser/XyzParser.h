#ifndef IQMOL_PARSER_XYZ_H
#define IQMOL_PARSER_XYZ_H
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
#include "Geometry.h"


namespace IQmol {
namespace Parser {

   /// Parser for XYZ format files.  Note that the file may contain more than
   /// one structure which may be useful in the case of animations.
   class Xyz : public Base {

      public:
         bool  parse(TextStream&);

      private:
         Data::Geometry* readNextGeometry(TextStream&);
   };

} } // end namespace IQmol::Parser

#endif
