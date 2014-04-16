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

#include "ExternalChargesParser.h"
#include "TextStream.h"
#include "PointCharge.h"
#include <limits>


namespace IQmol {
namespace Parser {

bool ExternalCharges::parse(TextStream& textStream) 
{
   int max(INT_MAX);
   bool allOk(true), isDouble;
   bool maxSet(false), invalidFormat(false);
   double x, y, z, q;
   QStringList tokens;
   Data::PointChargeList* charges(new Data::PointChargeList);

   while (!textStream.atEnd() && charges->size() < max) {
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() >= 4) {
         x = tokens[0].toDouble(&isDouble);  allOk = allOk && isDouble;
         y = tokens[1].toDouble(&isDouble);  allOk = allOk && isDouble;
         z = tokens[2].toDouble(&isDouble);  allOk = allOk && isDouble;
         q = tokens[3].toDouble(&isDouble);  allOk = allOk && isDouble;

         if (allOk) {
            charges->append(new Data::PointCharge(q, qglviewer::Vec(x,y,z)));
         }else {
            invalidFormat = true;
            break;
         }
      }else if (tokens.size() >= 1) {
         if (tokens.first().contains("$end", Qt::CaseInsensitive) || maxSet) {
            break;
         }else {
            max = tokens[0].toInt(&maxSet);
         }
      }
   } 

   if (maxSet && charges->size() != max) {
      if (invalidFormat) {
         QString msg("Invalid format on line ");
         m_errors.append(msg += QString::number(textStream.lineNumber()));
      }else {
         m_errors.append("End of stream encountered");
      }
      delete charges;
   }else if (charges->isEmpty()) {
      m_errors.append("No charges found");
      delete charges;
   }else {
      m_dataBank.append(charges);
   }

   return m_errors.isEmpty();
}

} } // end namespace IQmol::Parser
