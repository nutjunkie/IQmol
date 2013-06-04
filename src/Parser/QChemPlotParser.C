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

#include "QChemPlotParser.h"
#include "TextStream.h"
#include "GridData.h"

#include <QtDebug>


namespace IQmol {
namespace Parser2 {

Data::Bank& QChemPlot::parse(TextStream& textStream)
{
   QString line;
   QString description;
   QStringList tokens;

   while (!textStream.atEnd() && !line.contains("Grid point positions")) {
      line = textStream.nextLine();
   }

   if (line.contains("esp values")) {
      description = "Electrostatic Potential ";
   }else if (line.contains("electronic density")) {
      description = "Electronic Density ";
   }else if (line.contains("MO values")) {
      description = "Molecular Orbital ";
   }

   QList<double> values;

   textStream.skipLine();
   tokens = textStream.nextLineAsTokens();
   int nProperties(tokens.size()-3);
   if (nProperties < 1) goto error;

   m_data = new Data::GridList;
   for (int i = 0; i < nProperties; ++i) {
       m_data->append(new Data::Grid(description + tokens[i+3]));
   }

   values = textStream.nextLineAsDoubles();
   if (values.size() != 3 + nProperties) goto error;
   for (int i = 0; i < nProperties; ++i) {
       (*m_data)[i]->append(values[i+3]);
   }

   {/**/
      double xMin(values[0]), yMin(values[1]), zMin(values[2]);
      double xMax(xMin), yMax(yMin), zMax(zMin);
      int nx(1), ny(1), nz(1);

      while (!textStream.atEnd()) {
         if (values.size() == 3 + nProperties) {
            if (values[0] > xMax) { xMax = values[0]; ++nx; }
            if (values[1] > yMax) { yMax = values[1]; ++ny; }
            if (values[2] > zMax) { zMax = values[2]; ++nz; }
            for (int i = 0; i < nProperties; ++i) {
                (*m_data)[i]->append(values[i+3]);
            }
         }
         values = textStream.nextLineAsDoubles();
      }

      for (int i = 0; i < nProperties; ++i) {
          (*m_data)[i]->setMin(xMin, yMin, zMin);
          (*m_data)[i]->setMax(xMax, yMax, zMax);
          (*m_data)[i]->setNum(nx, ny, nz);
          (*m_data)[i]->dump();
          if (!(*m_data)[i]->isValid()) goto error;
      }
   }/**/

   m_dataBank.append(m_data);
   return m_dataBank;

   error:
      QString msg("Problem parsing QChem plot data, line ");
      m_errors.append(msg += QString::number(textStream.lineNumber()));
qDebug() << m_errors;
      if (m_data) {
         for (int i = 0; i < m_data->size(); ++i) {
             delete (*m_data)[i];
         }
         delete m_data;
       }

   return m_dataBank;
}

} } // end namespace IQmol::Parser
