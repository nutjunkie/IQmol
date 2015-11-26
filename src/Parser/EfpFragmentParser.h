#ifndef IQMOL_PARSER_EFPFRAGMENT_H
#define IQMOL_PARSER_EFPFRAGMENT_H
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


namespace IQmol {
namespace Parser {

   /// Parser for EFP parameter files.  Note this parser has the side effect of
   /// loading the EFP data into the EfpFragmentLibrary.
   class EfpFragment : public Base {

      public:
         /// Generic parser can read multiple fragments in a section or file.
         bool parse(TextStream&);

         /// Parser for reading a single fragment from a library file.
         QString readFragment(QString const& filePath);

         /// This function actually loads any EFP fragment in the TextStream into
         /// the library, returning the name of the fragment, or an empty string
         /// if the load failed.
         QString loadNextFragment(TextStream&);
   };

} } // end namespace IQmol::Parser

#endif
