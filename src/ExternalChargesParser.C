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

#include "ExternalChargesParser.h"
#include "ChargeLayer.h"
#include <QTextStream>


namespace IQmol {
namespace Parser {

DataList ExternalCharges::parse(QTextStream& textStream) 
{
   bool parseOkay(true), isDouble;
   QString line;
   QStringList tokens;
   double x, y, z, q;

   Layer::Data* charges = new Layer::Data("Charges");

   while (!textStream.atEnd() && parseOkay) {
      line = textStream.readLine();

      if (line.contains("$external_charges", Qt::CaseInsensitive)) {
      }else if (line.contains("$end", Qt::CaseInsensitive)) {
         break;
      }else {
         tokens = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
         if (tokens.count() == 0) {
            // ignore emtpy lines
         }else if (tokens.count() == 1) {
            tokens[0].toUInt(&parseOkay);
         }else if (tokens.count() == 4) {
            x = tokens[0].toDouble(&isDouble);  parseOkay = parseOkay && isDouble;
            y = tokens[1].toDouble(&isDouble);  parseOkay = parseOkay && isDouble;
            z = tokens[2].toDouble(&isDouble);  parseOkay = parseOkay && isDouble;
            q = tokens[3].toDouble(&isDouble);  parseOkay = parseOkay && isDouble;
            if (parseOkay) charges->appendRow(new Layer::Charge(q, qglviewer::Vec(x,y,z)));
         }else {
            parseOkay = false;
         }
      }
   } 

   if (parseOkay && charges->hasChildren()) {
      m_dataList.append(charges);
   }else {
      ChargeList list(charges->findLayers<Layer::Charge>(Layer::Children));
      ChargeList::iterator charge;
      for (charge = list.begin(); charge != list.end(); ++charge) {
          delete (*charge);
      }
// this delete is causing problems, see bug report #00094
      //delete charges;
   }

   return m_dataList;
}

} } // end namespace IQmol::Parser
