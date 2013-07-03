#ifndef IQMOL_MARCHINGCUBES_H
#define IQMOL_MARCHINGCUBES_H
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

#include "SurfaceLayer.h"
#include <QList>
#include  "QGLViewer/vec.h"


namespace IQmol {

   class Grid;

   /// Marching cubes algorithm for computing isosurfaces.  Based on the
   /// implementiont by Paul Bourke wiki contribution:
   /// Based on the following:
   ///    Qt-Adaption Created on: 15.07.2009
   ///        Author: manitoo
   /// Adapted for use with precomputed grids February 2011
   class MarchingCubes : public QObject {

      Q_OBJECT

      public:
         MarchingCubes(Grid* grid);
         Layer::Surface::Data const& generateSurface(GLfloat const& isoValue);

      Q_SIGNALS:
         void surfaceProgress(double);

      private:
         struct GLvector {
            GLfloat fX;
            GLfloat fY;
            GLfloat fZ;
         };

         //Marching Cubes Algorithm
         GLvoid vMarchCube1(const GLint &iX, const GLint &iY, const GLint &iZ, 
            const GLfloat& isoValue);

         //Helpers
         GLfloat getOffset(const GLfloat &fValue1, const GLfloat &fValue2, 
           const GLfloat &fValueDesired);
         GLvoid normalizeVector(GLvector &rfVectorResult, GLvector &rfVectorSource);
         GLvoid surfaceNormal(GLvector &rfNormal, const GLfloat &fX, const GLfloat &fY, 
            const GLfloat &fZ);

         Grid*   m_grid;
         GLfloat m_stepSize;
         Layer::Surface::Data m_surfaceData;

         // Static Data
         static const GLfloat s_vertexOffset[8][3];
         static const GLint   s_vertexIndexOffset[8][3];
         static const GLfloat s_edgeDirection[12][3];
         static const GLint   s_edgeConnection[12][2];
         static const GLint   s_cubeEdgeFlags[256];
         static const GLint   s_triangleConnectionTable[256][16];
   };

} // end namespace IQmol


#endif
