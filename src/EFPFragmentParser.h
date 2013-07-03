#ifndef IQMOL_EFPFRAGMENTPARSER_H
#define IQMOL_EFPFRAGMETNPARSER_H
/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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

#include "BaseParser.h"


class QTextStream;

namespace IQmol {
namespace Parser {

   /// Parser for EFP files.  At the moment all we do is grab the coordinates
   /// and ignore the rest of the guff, the format of which is beyond me.
   class EFPFragment : public Base {

      public:
         EFPFragment() : m_name("Unknown"), m_done(false) { }

         DataList parse(QTextStream&);
         QString name() const { return m_name; }

      private:
         void processLine(QTextStream&);
         void readCoordinates(QTextStream&);
         QString m_name;
         bool m_done;
   };

} } // end namespace IQmol::Parser

#endif
