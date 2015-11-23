/*******************************************************************************
       
  Copyright (C) 2011-2015 Andrew Gilbert
           
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

#include "CubeParser.h"
#include "CartesianCoordinatesParser.h"
#include "Constants.h"
#include "TextStream.h"
#include "Geometry.h"
#include "CubeData.h"
#include "QsLog.h"
#include <QtCore/QFile>
#include <QFileInfo>
#include <cmath>


namespace IQmol {
namespace Parser {

bool Cube::parse(TextStream& textStream)
{
   // Header information
   textStream.skipLine(2);

   int nAtoms(parseGridAxes(textStream));
   if (nAtoms == 0) {
      QString msg("Incorrect format on line ");
      msg += QString::number(textStream.lineNumber());
      msg += "\nExpected: <int>  <double>  <double>  <double>";
      m_errors.append(msg);
      return false;
   }

   if (!parseCoordinates(textStream, nAtoms)) return false;
   parseGridData(textStream);

   return m_errors.isEmpty();
}


bool Cube::parseCoordinates(TextStream& textStream, unsigned nAtoms) 
{
   Data::Geometry* geometry(new Data::Geometry);
   bool ok;
   unsigned atomicNumber;
   qglviewer::Vec position;

   for (unsigned i = 0; i < nAtoms; ++i) {
       QStringList tokens(textStream.nextLineAsTokens());
       if (tokens.size() != 5) return false; 
       atomicNumber = tokens[0].toUInt(&ok);  if (!ok) goto error;
       position.x = tokens[2].toDouble(&ok);  if (!ok) goto error;
       position.y = tokens[3].toDouble(&ok);  if (!ok) goto error;
       position.z = tokens[4].toDouble(&ok);  if (!ok) goto error;
       geometry->append(atomicNumber, position);
   } 

   geometry->scaleCoordinates(m_scale);
   geometry->computeGasteigerCharges();
   m_dataBank.append(geometry);
   return true;

   error:
      QString msg("Incorrect format on line ");
      msg += QString::number(textStream.lineNumber());
      delete geometry;
      return false;
}


int Cube::parseGridAxes(TextStream& textStream) 
{
   int nAtoms(0);
   bool ok, xOk, yOk, zOk;
   double x, y, z, dx, dy, dz;
   double thresh(1e-6);

   // origin line
   QStringList tokens(textStream.nextLineAsTokens());
   if (tokens.size() != 4) return 0;
   nAtoms = tokens[0].toInt(&ok);
   x = tokens[1].toDouble(&xOk);
   y = tokens[2].toDouble(&yOk);
   z = tokens[3].toDouble(&zOk);
   if (!(ok && xOk && yOk && zOk)) return 0;
   m_origin.setValue(x, y, z);

   // x line
   tokens = textStream.nextLineAsTokens();
   if (tokens.size() != 4) return 0;
   m_nx = tokens[0].toInt(&ok);
   dx   = tokens[1].toDouble(&xOk);
   y    = tokens[2].toDouble(&yOk);
   z    = tokens[3].toDouble(&zOk);
   if (!(ok && xOk && yOk && zOk)) return 0;
   if (std::abs(y) > thresh || std::abs(z) > thresh) goto axial_error;

   // y line
   tokens = textStream.nextLineAsTokens();
   if (tokens.size() != 4) return 0;
   m_ny = tokens[0].toInt(&ok); 
   x    = tokens[1].toDouble(&xOk);
   dy   = tokens[2].toDouble(&yOk);
   z    = tokens[3].toDouble(&zOk);
   if (!(ok && xOk && yOk && zOk)) return 0;
   if (std::abs(x) > thresh || std::abs(z) > thresh) goto axial_error;

   // z line
   tokens = textStream.nextLineAsTokens();
   if (tokens.size() != 4) return 0;
   m_nz = tokens[0].toInt(&ok); 
   x    = tokens[1].toDouble(&xOk);
   y    = tokens[2].toDouble(&yOk);
   dz   = tokens[3].toDouble(&zOk);
   if (!(ok && xOk && yOk && zOk)) return 0;
   if (std::abs(x) > thresh || std::abs(y) > thresh) goto axial_error;

   m_delta.setValue(dx, dy, dz); 

   // Check what units we are using.
   if (m_nx < 0) {
      m_scale = 1.0;
      QLOG_TRACE() << "Leaving grid data units unscaled";
   }else {
      m_scale = Constants::BohrToAngstrom;
      QLOG_TRACE() << "Scaling grid data units: Bohr -> Angstrom";
   }

   m_nx = std::abs(m_nx);
   m_ny = std::abs(m_ny);
   m_nz = std::abs(m_nz);
   m_origin *= m_scale;
   m_delta  *= m_scale;

   return nAtoms;

   axial_error:
      m_errors.append("Non-axial grid data not supported");
      return 0;
}


void Cube::parseGridData(TextStream& textStream) 
{
   QList<double> data;
   while (!textStream.atEnd()) {
      data += textStream.nextLineAsDoubles();
   }   

   QList<Data::Geometry*> geometryList(m_dataBank.findData<Data::Geometry>());
   if (geometryList.isEmpty()) {
      m_errors.append("Geometry data not found in cube file");
      return;
   }

   try {
      Data::SurfaceType type(Data::SurfaceType::CubeData);
      Data::GridSize    size(m_origin, m_delta, m_nx, m_ny, m_nz);

      Data::CubeData* cube(new Data::CubeData(*(geometryList.last()),size, type, data));
      m_dataBank.append(cube);

      QFileInfo info(m_filePath);
      cube->setLabel(info.completeBaseName());
   } catch (std::out_of_range const& err) {
      m_errors.append("Invalid grid data in cube file");
   }
}


bool Cube::save(QString const& filePath, Data::Bank& bank)
{
   // Make sure we have the required data
   QList<Data::Geometry*> geometries(bank.findData<Data::Geometry>());
   if (geometries.isEmpty()) {
      m_errors.append("No geometry information found");
      return false;
   }else if (geometries.size() > 1) {
      QLOG_WARN() << "Ambiguous geometry information found in Cube parser";
   }
   Data::Geometry* geometry(geometries.first());

   QList<Data::GridData*> grids(bank.findData<Data::GridData>()); 
   if (grids.isEmpty()) {
      m_errors.append("No grid data found");
      return false;
   }else if (grids.size() > 1) {
      QLOG_WARN() << "More than one grid specified in Cube parser";
   }
   Data::GridData* grid(grids.first());
  
   QFile file(filePath);
   if (file.exists() || !file.open(QIODevice::WriteOnly)) {
      m_errors.append("Failed to open file for write");
      return false;
   }

   QStringList header;
   header << "Cube file for " + grid->surfaceType().toString();
   header << "Generated using IQmol";
   
   QStringList coords(formatCoordinates(*geometry));

   unsigned nx, ny, nz;
   grid->getNumberOfPoints(nx, ny, nz);
   qglviewer::Vec delta(grid->delta());
   qglviewer::Vec origin(grid->origin());
   origin *= Constants::AngstromToBohr;

   header << QString("%1 %2 %3 %4").arg(coords.size(), 5)
                                   .arg(origin.x, 13, 'f', 6)
                                   .arg(origin.y, 13, 'f', 6)
                                   .arg(origin.z, 13, 'f', 6);

   header << QString("%1 %2 %3 %4").arg(nx, 5).arg(delta.x, 13, 'f', 6)
                                              .arg(0.0,     13, 'f', 6)
                                              .arg(0.0,     13, 'f', 6);
   header << QString("%1 %2 %3 %4").arg(ny, 5).arg(0.0,     13, 'f', 6)
                                              .arg(delta.y, 13, 'f', 6)
                                              .arg(0.0,     13, 'f', 6);
   header << QString("%1 %2 %3 %4").arg(nz, 5).arg(0.0,     13, 'f', 6)
                                              .arg(0.0,     13, 'f', 6)
                                              .arg(delta.z, 13, 'f', 6);
   header << coords;

   QByteArray buffer;
   buffer.append(header.join("\n"));
   buffer.append("\n");
   file.write(buffer);
   buffer.clear();

   double w;
   unsigned col(0);

   for (unsigned i = 0; i < nx; ++i) {
       for (unsigned j = 0; j < ny; ++j) {
           for (unsigned k = 0; k < nz; ++k, ++col) {
               w = (*grid)(i, j, k);
               if (w >= 0.0) buffer += " ";
               buffer += QString::number(w, 'E', 5);
               if (col == 5) {
                  col = -1;
                  buffer += "\n";
               }else {
                  buffer += " ";
               }
           }
           file.write(buffer); 
           buffer.clear();
       }
   }

   buffer += "\n";
   file.write(buffer); 
   file.flush();
   file.close();

   return true;
}


QStringList Cube::formatCoordinates(Data::Geometry const& geometry)
{
   qglviewer::Vec position;
   unsigned atomicNo;
   QString line;
   QStringList coords;

   for (unsigned i = 0; i < geometry.nAtoms(); ++i) {
       position = Constants::AngstromToBohr * geometry.position(i);
       atomicNo = geometry.atomicNumber(i);
       line  = QString("%1").arg(atomicNo, 5);
       line += QString("%1").arg((double)atomicNo,  14, 'f', 6);
       line += QString("%1").arg(position.x, 14, 'f', 6);
       line += QString("%1").arg(position.y, 14, 'f', 6);
       line += QString("%1").arg(position.z, 14, 'f', 6);
       coords.append(line);
   }
   return coords; 
}

} } // end namespace IQmol::Parser
