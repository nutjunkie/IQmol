#ifndef IQMOL_PARSER_QCHEMINPUT_H
#define IQMOL_PARSER_QCHEMINPUT_H
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
#include "GeometryList.h"


namespace IQmol {
namespace Parser {

   class QChemInput : public Base {

      public:
         bool parse(TextStream&);

      private:
         void readRemSection(TextStream&);
         void readEfpFragmentSection(TextStream&);
         void readExternalChargesSection(TextStream&);
         void readEfpParamsSection(TextStream&);
         void readMoleculeSection(TextStream&);
         void parseGeometry(QStringList const& lines, int offset, bool zmat);

         Data::GeometryList* m_geometryList;
   };

} } // end namespace IQmol::Parser

#endif
