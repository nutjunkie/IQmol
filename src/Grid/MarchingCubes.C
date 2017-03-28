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

#include "GridData.h"
#include "QsLog.h"
#include "MarchingCubes.h"
#include "MarchingCubesData.h"
#include "boost/bind.hpp"
#include <cmath>

#include <QDebug>

namespace IQmol {


MarchingCubes::MarchingCubes(Data::GridData const& grid) : m_grid(grid), 
   m_origin(grid.origin()), m_delta(grid.delta())
{ 
   grid.getNumberOfPoints(m_nx, m_ny, m_nz);
}


void MarchingCubes::generateMesh(double const isovalue, Data::Mesh& mesh) 
{
   QLOG_INFO() << "Generating surface isovalue" << isovalue;
   double progressStep(1.0/m_nx);
   m_vertexMap.clear();
   m_isovalue       = isovalue;

   m_mesh = &mesh;
   if (!m_mesh) {
      QLOG_ERROR() << "Failed to find mesh reference in MarchingCubes";
      return;
   }

   // Trim the index ranges, 1 for the cube and 2 for the normal
   for (unsigned i = 2; i < m_nx-3; ++i) {
       progress(i*progressStep);
       for (unsigned j = 2; j < m_ny-3; ++j) {
           for (unsigned k = 2; k < m_nz-3; ++k) {
               marchOnCube(i, j, k);
           }
       }
   }
}


void MarchingCubes::marchOnCube(int const ix, int const iy, int const iz)
{
   // Make a local copy of the values at the cube's corners
   double cubeValues[8];
   for (unsigned vertex = 0; vertex < 8; ++vertex) {
       cubeValues[vertex] = m_grid( ix + s_vertexIndexOffset[vertex][0],
                                    iy + s_vertexIndexOffset[vertex][1],  
                                    iz + s_vertexIndexOffset[vertex][2]  );
   }

   // Find which vertices are inside of the surface and which are outside
   int flagIndex(0);
   for (int vertexTest = 0; vertexTest < 8; ++vertexTest) {
       if (cubeValues[vertexTest] <= m_isovalue)  flagIndex |= 1 << vertexTest;
   }

   // Find which edges are intersected by the surface
   int edgeFlags(s_cubeEdgeFlags[flagIndex]);

   // If the cube is entirely inside or outside of the surface, 
   // then there will be no intersections
   if (edgeFlags == 0) return;

   qglviewer::Vec cubeOrigin(ix*m_delta.x, iy*m_delta.y, iz*m_delta.z);
   cubeOrigin += m_origin;

   // Find the point of intersection of the surface with each edge, if any.
   // Each edge vertex gets indexed based on the lowest numbered corner 
   // vertex, and the edge direction from this corner.
   Data::OMMesh::VertexHandle edgeVertexHandle[12];

   for (int edge = 0; edge < 12; ++edge) {

       if (edgeFlags & (1 << edge)) {
          // The surface intersects this edge
          unsigned corner(s_edgeVertexAssignment[edge][0]);
          unsigned axis(s_edgeVertexAssignment[edge][1]);
          unsigned jx(s_vertexIndexOffset[corner][0]);
          unsigned jy(s_vertexIndexOffset[corner][1]);
          unsigned jz(s_vertexIndexOffset[corner][2]);

          Index index(ix+jx, iy+jy, iz+jz, axis);
          if (!m_vertexMap.contains(index)) {
             unsigned v0(s_edgeConnection[edge][0]);
             unsigned v1(s_edgeConnection[edge][1]);
             double   offset(getOffset( cubeValues[v0], cubeValues[v1]));

             Data::OMMesh::VertexHandle vertex(createEdgeVertex(edge, offset, cubeOrigin));
             m_vertexMap.insert(index, vertex);
          }

          edgeVertexHandle[edge] = m_vertexMap.value(index);
       }
   }

   // Draw the triangles that were found (there can be up to five per cube).
   if (m_isovalue > 0.0 ) {

      for (unsigned triangle = 0; triangle < 5; ++triangle) {
          if (s_triangleConnectionTable[flagIndex][3*triangle] < 0) break;
          int v0(s_triangleConnectionTable[flagIndex][3*triangle+0]);
          int v1(s_triangleConnectionTable[flagIndex][3*triangle+1]);
          int v2(s_triangleConnectionTable[flagIndex][3*triangle+2]);
          m_mesh->addFace(edgeVertexHandle[v0], edgeVertexHandle[v1], edgeVertexHandle[v2]);
      }

   }else {   // need to reverse the vertex ordering for the face normal

      for (unsigned triangle = 0; triangle < 5; ++triangle) {
          if (s_triangleConnectionTable[flagIndex][3*triangle] < 0) break;
          int v0(s_triangleConnectionTable[flagIndex][3*triangle+0]);
          int v1(s_triangleConnectionTable[flagIndex][3*triangle+1]);
          int v2(s_triangleConnectionTable[flagIndex][3*triangle+2]);
          m_mesh->addFace(edgeVertexHandle[v2], edgeVertexHandle[v1], edgeVertexHandle[v0]);
      }

   }
}


double MarchingCubes::getOffset(double const v1, double const v2)
{
   double dv(v2-v1);
   return (dv == 0.0) ? 0.5 : (m_isovalue-v1)/dv;
}


Data::OMMesh::VertexHandle MarchingCubes::createEdgeVertex(unsigned const edge, 
   double const offset, qglviewer::Vec const& origin) 
{
   double x = origin.x + (s_vertexOffset[ s_edgeConnection[edge][0] ][0] +  
                                 offset * s_edgeDirection[edge][0]) * m_delta.x;
   double y = origin.y + (s_vertexOffset[ s_edgeConnection[edge][0] ][1] +  
                                 offset * s_edgeDirection[edge][1]) * m_delta.y;
   double z = origin.z + (s_vertexOffset[ s_edgeConnection[edge][0] ][2] +  
                                 offset * s_edgeDirection[edge][2]) * m_delta.z;

/*
   // this is debug code to check that the indexing is working correctly.  If
   // it is, then we should never find an existing vertex closer than epsilon
   // (= 0.00001).

   Data::OMMesh::ConstVertexIter vertex;
   Data::OMMesh::Point point;

   double distance(999999999.0), d;
   for (vertex = m_mesh->vbegin(); vertex != m_mesh->vend(); ++vertex) {
       point = m_mesh->vertex(vertex.handle());
       d = (x-point[0])*(x-point[0]) + (y-point[1])*(y-point[1]) + (z-point[2])*(z-point[2]);
       d = std::sqrt(d);
       distance = std::min(distance, d);
       if (distance < 0.00001) {
          existing = true;
          return vertex.handle();
       }
   }
   //qDebug() << "    Minimum distance to existing points =" << distance;

*/
   Data::OMMesh::VertexHandle handle(m_mesh->addVertex(x, y, z));
   qglviewer::Vec n(m_grid.normal(x,y,z));
   if (m_isovalue < 0.0) n = -n;
   m_mesh->setNormal(handle, n.x, n.y, n.z); 

   return handle;
}

} // end namespace IQmol
