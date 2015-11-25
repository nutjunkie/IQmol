#ifndef IQMOL_PARSER_QCHEMOUTPUT_H
#define IQMOL_PARSER_QCHEMOUTPUT_H
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

namespace Data {
   class Nmr;
   class Geometry;
}

namespace Parser {

   class QChemOutput : public Base {

      public:
         bool parse(TextStream&);

         static QStringList parseForErrors(QString const& filePath);
         // move to private when LocalConnectionThread is deprecated
         static QStringList parseForErrors(TextStream&);

      private:
         Data::Geometry* readStandardCoordinates(TextStream&);
         void readStandardCoordinates(TextStream&, Data::Geometry&);
         void readCharges(TextStream&, Data::Geometry&, QString const& label);
         void readNmrShifts(TextStream&, Data::Geometry&, Data::Nmr&);
         void readNmrReference(TextStream&, Data::Nmr&);
         void readNmrCouplings(TextStream&, Data::Geometry&, Data::Nmr&);
         void readDipoleMoment(TextStream&, Data::Geometry&);
         void readHessian(TextStream&, Data::Geometry&);
         void readVibrationalModes(TextStream&);
         void readEffectiveRegion(TextStream&);
         void readDMA(TextStream&, Data::Geometry&);
         void readCisStates(TextStream&);
         void readOrbitalSymmetries(TextStream&, bool const readSymmetries);
         void setTotalEnergy(QString const&, Data::Geometry*, QString const& label);

         unsigned m_nAlpha;
         unsigned m_nBeta;
   };

} } // end namespace IQmol::Parser

#endif
