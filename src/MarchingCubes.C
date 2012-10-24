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

#include "Grid.h"
#include "MarchingCubes.h"
#include "MarchingCubesData.h"
#include "boost/bind.hpp"
#include <cmath>


namespace IQmol {

MarchingCubes::MarchingCubes(Grid* grid) :  m_grid(grid), m_stepSize(grid->stepSize()) { }


//! Returns a list of interleaved surface vertices and normals.
Layer::Surface::Data const& MarchingCubes::generateSurface(GLfloat const& isoValue) 
{
   m_surfaceData.clear();
   double progressStep(1.0/(m_grid->xMax()-m_grid->xMin()-3));
   int p(0);

   // Trim the index ranges, 1 for the cube and 2 for the normal
   for (GLint i = m_grid->xMin()+2; i <= m_grid->xMax()-3; ++i) {
      surfaceProgress( p * progressStep );
       for (GLint j = m_grid->yMin()+2; j <= m_grid->yMax()-3; ++j) {
           for (GLint k = m_grid->zMin()+2; k <= m_grid->zMax()-3; ++k) {
               vMarchCube1(i, j, k, isoValue);
           }
       }
       ++p;
   }
   return m_surfaceData;
}


// getOffset finds the approximate point of intersection of the surface
// between two points with the values fValue1 and fValue2
GLfloat MarchingCubes::getOffset(const GLfloat &fValue1, const GLfloat &fValue2, 
   const GLfloat &fValueDesired)
{
/*
   if (fValue1*fValue2 > 0) {
      double v1(std::log(std::abs(fValue1)));
      double v2(std::log(std::abs(fValue2)));
      double c(std::log(std::abs(fValueDesired)));
      return (c - v1)/(v2-v1);
   }
*/

   GLdouble fDelta = fValue2 - fValue1;
   if (fDelta == 0.0) { return 0.5; }
   return (fValueDesired - fValue1)/fDelta;
}



// vMarchCube1 performs the Marching Cubes algorithm on a single cube
GLvoid MarchingCubes::vMarchCube1(const GLint &iX, const GLint &iY, const GLint &iZ, 
   const GLfloat& isoValue)
{

   GLfloat fX(iX*m_stepSize);
   GLfloat fY(iY*m_stepSize);
   GLfloat fZ(iZ*m_stepSize);

   GLint iCorner, iVertex, iVertexTest, iEdge, iTriangle, iFlagIndex, iEdgeFlags;
   GLfloat fOffset;
   GLfloat afCubeValue[8];
   GLvector asEdgeVertex[12];
   GLvector asEdgeNorm[12];

   // Make a local copy of the values at the cube's corners
   for (iVertex = 0; iVertex < 8; iVertex++) {
       afCubeValue[iVertex] = (*m_grid)(iX + s_vertexIndexOffset[iVertex][0],
                                        iY + s_vertexIndexOffset[iVertex][1],
                                        iZ + s_vertexIndexOffset[iVertex][2]);
   }

   // Find which vertices are inside of the surface and which are outside
   iFlagIndex = 0;
   for (iVertexTest = 0; iVertexTest < 8; iVertexTest++) {
       if (afCubeValue[iVertexTest] <= isoValue)  iFlagIndex |= 1<<iVertexTest;
   }

   // Find which edges are intersected by the surface
   iEdgeFlags = s_cubeEdgeFlags[iFlagIndex];

   // If the cube is entirely inside or outside of the surface, then there will
   // be no intersections
   if(iEdgeFlags == 0) return;

   // Find the point of intersection of the surface with each edge
   // Then find the normal to the surface at those points
   for (iEdge = 0; iEdge < 12; iEdge++) {
       // if there is an intersection on this edge
       if (iEdgeFlags & (1<<iEdge)) {
           fOffset = getOffset(afCubeValue[ s_edgeConnection[iEdge][0] ],
                     afCubeValue[ s_edgeConnection[iEdge][1] ], isoValue);
           asEdgeVertex[iEdge].fX = fX + (s_vertexOffset[ s_edgeConnection[iEdge][0] ][0] +  
                                         fOffset * s_edgeDirection[iEdge][0]) * m_stepSize;
           asEdgeVertex[iEdge].fY = fY + (s_vertexOffset[ s_edgeConnection[iEdge][0] ][1] +  
                                         fOffset * s_edgeDirection[iEdge][1]) * m_stepSize;
           asEdgeVertex[iEdge].fZ = fZ + (s_vertexOffset[ s_edgeConnection[iEdge][0] ][2] +  
                                         fOffset * s_edgeDirection[iEdge][2]) * m_stepSize;

          surfaceNormal(asEdgeNorm[iEdge], asEdgeVertex[iEdge].fX, 
             asEdgeVertex[iEdge].fY, asEdgeVertex[iEdge].fZ);

          if (isoValue < 0) {
             asEdgeNorm[iEdge].fX = - asEdgeNorm[iEdge].fX;
             asEdgeNorm[iEdge].fY = - asEdgeNorm[iEdge].fY;
             asEdgeNorm[iEdge].fZ = - asEdgeNorm[iEdge].fZ;
          }
       }
   }


   // Draw the triangles that were found.  There can be up to five per cube
   for (iTriangle = 0; iTriangle < 5; iTriangle++) {
       if (s_triangleConnectionTable[iFlagIndex][3*iTriangle] < 0) break;

       for (iCorner = 0; iCorner < 3; iCorner++) {
           iVertex = s_triangleConnectionTable[iFlagIndex][3*iTriangle+iCorner];
           m_surfaceData.push_back(asEdgeNorm[iVertex].fX);
           m_surfaceData.push_back(asEdgeNorm[iVertex].fY);
           m_surfaceData.push_back(asEdgeNorm[iVertex].fZ);
           m_surfaceData.push_back(asEdgeVertex[iVertex].fX);
           m_surfaceData.push_back(asEdgeVertex[iVertex].fY);
           m_surfaceData.push_back(asEdgeVertex[iVertex].fZ);
       }
   }
}



// This does a tri-linear interpolation of the derivatives at each of the 8
// grid points around the given point.  This avoids additional calls to the
// function and speeds things up by a factor of 2 or so.
GLvoid MarchingCubes::surfaceNormal(GLvector &norm, GLfloat const& x, GLfloat const& y, 
   GLfloat const& z)
{
   // needs offloading to the Grid class
   GLvector V000, V001, V010, V011, V100, V101, V110, V111;

   GLfloat gx(x/m_stepSize);
   GLfloat gy(y/m_stepSize);
   GLfloat gz(z/m_stepSize);

   GLint x0( std::floor(gx) );
   GLint y0( std::floor(gy) );
   GLint z0( std::floor(gz) );

   GLint x1(x0+1);
   GLint y1(y0+1);
   GLint z1(z0+1);

   int i = x0; int j = y0; int k = z0;
   V000.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V000.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V000.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   i = x0; j = y0; k = z1;
   V001.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V001.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V001.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   i = x0; j = y1; k = z0;
   V010.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V010.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V010.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   i = x0; j = y1; k = z1;
   V011.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V011.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V011.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   i = x1; j = y0; k = z0;
   V100.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V100.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V100.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   i = x1; j = y0; k = z1;
   V101.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V101.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V101.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   i = x1; j = y1; k = z0;
   V110.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V110.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V110.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   i = x1; j = y1; k = z1;
   V111.fX = (*m_grid)(i+1, j,   k  ) - (*m_grid)(i-1, j,   k  );
   V111.fY = (*m_grid)(i,   j+1, k  ) - (*m_grid)(i,   j-1, k  );
   V111.fZ = (*m_grid)(i,   j,   k+1) - (*m_grid)(i,   j,   k-1);

   GLvector p0;
   p0.fX = gx - x0;
   p0.fY = gy - y0;
   p0.fZ = gz - z0;

   GLvector p1;
   p1.fX = x1 - gx;
   p1.fY = y1 - gy;
   p1.fZ = z1 - gz;

   GLfloat dX, dY, dZ;
   dX = V000.fX * p1.fX * p1.fY * p1.fZ
      + V001.fX * p1.fX * p1.fY * p0.fZ
      + V010.fX * p1.fX * p0.fY * p1.fZ
      + V011.fX * p1.fX * p0.fY * p0.fZ
      + V100.fX * p0.fX * p1.fY * p1.fZ
      + V101.fX * p0.fX * p1.fY * p0.fZ
      + V110.fX * p0.fX * p0.fY * p1.fZ
      + V111.fX * p0.fX * p0.fY * p0.fZ;

   dY = V000.fY * p1.fX * p1.fY * p1.fZ
      + V001.fY * p1.fX * p1.fY * p0.fZ
      + V010.fY * p1.fX * p0.fY * p1.fZ
      + V011.fY * p1.fX * p0.fY * p0.fZ
      + V100.fY * p0.fX * p1.fY * p1.fZ
      + V101.fY * p0.fX * p1.fY * p0.fZ
      + V110.fY * p0.fX * p0.fY * p1.fZ
      + V111.fY * p0.fX * p0.fY * p0.fZ;

   dZ = V000.fZ * p1.fX * p1.fY * p1.fZ
      + V001.fZ * p1.fX * p1.fY * p0.fZ
      + V010.fZ * p1.fX * p0.fY * p1.fZ
      + V011.fZ * p1.fX * p0.fY * p0.fZ
      + V100.fZ * p0.fX * p1.fY * p1.fZ
      + V101.fZ * p0.fX * p1.fY * p0.fZ
      + V110.fZ * p0.fX * p0.fY * p1.fZ
      + V111.fZ * p0.fX * p0.fY * p0.fZ;

   norm.fX = -dX;
   norm.fY = -dY;
   norm.fZ = -dZ;

   normalizeVector(norm, norm);
}



GLvoid MarchingCubes::normalizeVector(GLvector &result, GLvector &source)
{ 
   GLfloat scale = sqrt( (source.fX * source.fX) +
                         (source.fY * source.fY) +
                         (source.fZ * source.fZ) );

   if (scale == 0.0) {
      result.fX = source.fX;
      result.fY = source.fY;
      result.fZ = source.fZ;
   } else {
      scale = 1.0/scale;
      result.fX = source.fX * scale;
      result.fY = source.fY * scale;
      result.fZ = source.fZ * scale;
   }
}


} // end namespace IQMol
