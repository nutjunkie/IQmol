#ifndef IQMOL_EXTERNALCHARGEPARSER_H
#define IQMOL_EXTERNALCHARGEPARSER_H
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

#include "DataLayer.h"
#include "BaseParser.h"


namespace IQmol {
namespace Parser {

   /// Parses a file containing external charges.  For a file to have a valid
   /// format it may start with a line containing either a single integer (the
   /// number of charges) or the string $external_charges (if the latter the
   /// file must end with the token $end).  The remaining lines in the file must 
   /// contain exactly four real numbers representing the coordinates followed 
   /// by the value of the charge.
   /// 
   /// \b Note: Curently the coordinates must be in angstroms and the charge in 
   /// atomic units.
   class ExternalCharges : public Base {

      public:
         ExternalCharges() { }
         DataList parse(QTextStream&);
   };

} } // end namespace IQmol::Parser

#endif
