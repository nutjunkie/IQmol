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

#include "CartesianCoordinatesParser.h"
#include "Geometry.h"
#include "TextStream.h"
#include "Atom.h"

#include <QDebug>

namespace IQmol {
namespace Parser {

Data::Geometry* CartesianCoordinates::parse(TextStream& textStream)
{
   Data::Geometry* geometry(new Data::Geometry);
   // Initialize these in case the parser is used more than once.
   m_error.clear();

   double x, y, z;
   int offset(0), cnt(0);
   unsigned a;
   QStringList tokens;

   while (!textStream.atEnd() && cnt < m_max) {
      tokens = textStream.nextNonEmptyLineAsTokens();
      bool allOk(tokens.size() >= 4), ok;

	  // If there are 5 tokens then we check to see if the first is just the
	  // index.  This will fail if there are 5 tokens but the last is a comment
	  // and the first atom is hydrogen specified with atomic number 1 rather 
      // than H.  What are the chances?
      if (cnt == 0 && tokens.size() == 5) {
         a = tokens[0].toInt(&ok);
         if (ok && a == 1) offset = 1;
      }

      if (allOk) {
         // Make sure we can get a valid atomic number
         a = tokens[0+offset].toUInt(&ok);
         if (!ok) a = Data::Atom::atomicNumber(tokens[0+offset]);
         if (a == 0) allOk = false;   
      }

      if (allOk) {
         // Make sure our coordinates are valid
         x = tokens[1+offset].toDouble(&ok);  allOk = allOk && ok;
         y = tokens[2+offset].toDouble(&ok);  allOk = allOk && ok;
         z = tokens[3+offset].toDouble(&ok);  allOk = allOk && ok;
      }

      if (allOk) {
         geometry->append(a, qglviewer::Vec(x,y,z));
         ++cnt;
      }else {
         break;
      }
   }

   if (cnt == 0) {
      delete geometry;
      geometry = 0;
   }else {
      if (m_max < INT_MAX && cnt != m_max) {
         m_error = ("Invalid format line ");
         m_error += QString::number(textStream.lineNumber());
         m_error += ":\nExpected: [int|symbol]   float  float  float";
      }
      geometry->computeGasteigerCharges();
   }

   return geometry;
}

} } // end namespace IQmol::Parser
