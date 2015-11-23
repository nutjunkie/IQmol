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

#include "QChemPlotParser.h"
#include "TextStream.h"
#include "GridData.h"

#include <QtDebug>


namespace IQmol {
namespace Parser {

// Note the grid data is read in as bohr, but converted to angstroms in the
// Grid constructor
bool QChemPlot::parse(TextStream& textStream)
{
   QList<double> values;

   QList<Data::SurfaceType> surfaceTypes(parseForProperties(textStream));
   unsigned nProperties(surfaceTypes.size());
   if (nProperties < 1) goto error;
   
   values = textStream.nextLineAsDoubles();
   if ((unsigned)values.size() != 3 + nProperties) goto error;

   {/**/
      double xMin(values[0]);
      double yMin(values[1]);
      double zMin(values[2]);
      double xMax(xMin);
      double yMax(yMin);
      double zMax(zMin);

      typedef QList<double> List;
      List* data = new List[nProperties];

      for (unsigned i = 0; i < nProperties; ++i) {
          data[i].append(values[i+3]);
      }

      unsigned nx(1), ny(1), nz(1);

      while (!textStream.atEnd()) {
         if ((unsigned)values.size() == 3 + nProperties) {
            if (values[0] > xMax) { xMax = values[0]; ++nx; }
            if (values[1] > yMax) { yMax = values[1]; ++ny; }
            if (values[2] > zMax) { zMax = values[2]; ++nz; }
            for (unsigned i = 0; i < nProperties; ++i) {
                data[i].append(values[i+3]);
            }
         }
         values = textStream.nextLineAsDoubles();
      }

      qglviewer::Vec delta((xMax-xMin)/nx, (yMax-yMin)/ny, (zMax-zMin)/nz);
      qglviewer::Vec origin(xMin, yMin, zMin);
      Data::GridDataList* gridList(new Data::GridDataList);

      try {

         for (unsigned i = 0; i < nProperties; ++i) {
             Data::GridSize size(origin, delta, nx, ny, nz);
             gridList->append(new Data::GridData(size, surfaceTypes[i]));
         }

         m_dataBank.append(gridList);

      } catch (...) {
         m_errors.append("Problem creating QChem plot data");
         delete gridList; 
      }

      delete[] data;
   }/**/

   return m_errors.isEmpty();

   error:
      QString msg("Problem parsing QChem plot data, line ");
      m_errors.append(msg += QString::number(textStream.lineNumber()));
      return false;
}


QList<Data::SurfaceType> QChemPlot::parseForProperties(TextStream& textStream)
{
   QString firstLine;
   while (!textStream.atEnd() && !firstLine.contains("Grid point positions")) {
      firstLine = textStream.nextLine();
   }

   bool esp(firstLine.contains("esp values"));
   bool rho(firstLine.contains("electronic density values"));

   textStream.skipLine();
   QString secondLine(textStream.nextLine());

   QList<Data::SurfaceType> surfaceTypes;

   unsigned count(1);
   while (!secondLine.isEmpty()) {
      QString field(secondLine.left(13));
      secondLine.remove(0, 13);
      field = field.trimmed();

      if (field == "X" || field == "Y" || field == "Z") {
         // ignore
      }else if(field.contains("ALPHA")) {
         surfaceTypes.append(
           Data::SurfaceType(Data::SurfaceType::AlphaOrbital, count++));
      }else if(field.contains("BETA")) {
         surfaceTypes.append(
           Data::SurfaceType(Data::SurfaceType::BetaOrbital, count++));
      }else if (esp) {
         surfaceTypes.append(
           Data::SurfaceType(Data::SurfaceType::ElectrostaticPotential, count++));
      }else if (rho) {
         surfaceTypes.append(
           Data::SurfaceType(Data::SurfaceType::TotalDensity, count++));
      }
   }
   
   return surfaceTypes;
}

} } // end namespace IQmol::Parser
