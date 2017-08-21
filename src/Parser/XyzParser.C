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

#include "Energy.h"
#include "XyzParser.h"
#include "GeometryList.h"
#include "CartesianCoordinatesParser.h"
#include "TextStream.h"

#include <QtDebug>


namespace IQmol {
namespace Parser {

bool Xyz::parse(TextStream& textStream)
{
   Data::GeometryList* geometryList(new Data::GeometryList(m_label));
   Data::Geometry* geometry(0);

   while (!textStream.atEnd()) {
      geometry = readNextGeometry(textStream);
      if (geometry) geometryList->append(geometry);
   }

   if (geometryList->isEmpty()) {
      QString msg("No coordinates found");
      m_errors.append(msg);
      delete geometryList;
   }else {
      m_dataBank.append(geometryList);
   }

   return m_errors.isEmpty();
}


Data::Geometry* Xyz::readNextGeometry(TextStream& textStream)
{
   const QRegExp integerOnly("^\\d+$");  // note nextLine() returns a trimmed line
   QRegExp anyReal("([-+]?[0-9]*\\.[0-9]+([eE][-+]?[0-9]+)?)");

   Data::Geometry* geometry(0);
   QString line(textStream.previousLine());

   // We look for a line with a single integer giving the number of atoms, this
   // may be the previousLine in the TextStream if the XYZ blocks are back to
   // back. Note that the number is maximal, i.e. if there are more coordinates
   // they will not be read.   
   if (!line.contains(integerOnly)) {
      line = textStream.seek(integerOnly);
   }

   bool ok;
   int nAtoms(line.toInt(&ok));
   double energy;

   if (ok) {
      // Search for an energy/real quantity on the comment line
      energy = 0.0;
      line = textStream.nextLine();
      if (anyReal.indexIn(line) > -1) energy = anyReal.cap(1).toDouble(&ok);

      CartesianCoordinates parser(nAtoms);
      geometry = parser.parse(textStream);

      if (geometry) {
         Data::TotalEnergy& totalEnergy(geometry->getProperty<Data::TotalEnergy>());
         totalEnergy.setValue(energy, Data::Energy::Hartree);
         QString error(parser.error());
         if (!error.isEmpty()) m_errors.append(error);
      }else {
         m_errors.append("No coordinates found");
      }
   }

   return geometry;
}

} } // end namespace IQmol::Parser
