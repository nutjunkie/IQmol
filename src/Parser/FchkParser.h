#ifndef IQMOL_PARSER_FCHK_H
#define IQMOL_PARSER_FCHK_H
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

#include "Parser.h"
#include "Geometry.h"


namespace IQmol {
namespace Parser2 {

   class QChemOutput : public Base {

      public:
         Data::Bank& parse(TextStream&);
      private:
         Data::Geometry* readStandardCoordinates(TextStream&);
         void readStandardCoordinates(TextStream&, Data::Geometry*);
         void readCharges(TextStream&, Data::Geometry*, QString const& label);
         void readNmrShifts(TextStream&, Data::Geometry*);
         void readDipoleMoment(TextStream&, Data::Geometry*);
         void readVibrationalModes(TextStream&);
         void readEffectiveRegion(TextStream&);
         void setTotalEnergy(QString const&, Data::Geometry*);
   };

} } // end namespace IQmol::Parser

#endif
