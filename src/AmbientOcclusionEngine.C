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

#include "AmbientOcclusionEngine.h"
#include "Lebedev.h"
#include "QGLViewer/quaternion.h"
#include <cmath>

#include <QDebug>


// Maximum number of cells in a single dimension
const unsigned MaxCellCount = 30;
const double Epsilon = 10e-6;
using namespace qglviewer;

namespace IQmol {

AmbientOcclusionEngine::AmbientOcclusionEngine(QList<GLfloat> const& vertexData, 
   unsigned lebedevRule) : m_vertexData(vertexData), m_lebedevRule(lebedevRule)
{
}


void AmbientOcclusionEngine::run()
{
   m_nVertices  = m_vertexData.size()/6;
   m_nTriangles = m_nVertices/3;
   if (m_nTriangles < 1) return;

   computeBoundingBox();
qDebug() << "Computing bounding box min:" << m_boundingBoxOrigin.x  << m_boundingBoxOrigin.y 
                                          << m_boundingBoxOrigin.z  << m_nx << m_ny << m_nz;
qDebug() << "Computing bounding box max:" << m_boundingBoxOrigin.x + m_nx*m_cellLength
                                          << m_boundingBoxOrigin.y + m_ny*m_cellLength
                                          << m_boundingBoxOrigin.z + m_nz*m_cellLength;

   allocateBoxes();
   assignTrianglesToBoxes();

   unsigned nGrid(loadLebedevGrid());
   QList<unsigned> triangles;
   double occlusion;
   Vec v, n, r, z(0.0, 0.0, 1.0);

   // loop over each vertex
   for (unsigned i = 0; i < m_nVertices; ++i) {
       n = normal(i); 
       v = vertex(i); 
       // determine the alignment for the Lebedev grid and z-axis
       Quaternion q(n,z);
       occlusion = 0.0;
      
       // loop over the rays in the grid
       for (unsigned j = 0; j < nGrid; ++j) {
           r = q*m_gridPositions[j];
           triangles = triangleShortList(v, r);
//qDebug() << "Triangle considered " << triangles.size();
           for (int k = 0; k < triangles.size(); ++k) {
               if (rayIntersectsTriangle(v, r, triangles[k])) {
                  //occlusion += m_gridWeights[j];
                  occlusion += 1.0;
                  break;
               }
           }
       }

       m_data.append(1.0-occlusion/nGrid);
   }

   // clean up
   deallocateBoxes();
   qDebug() << "AO finished computing" << m_data.size() << "values for" 
            << m_nVertices << "vertices";
}


void AmbientOcclusionEngine::computeBoundingBox()
{
   Vec bbMin(vertex(0));
   Vec bbMax(bbMin);
   Vec v;
   for (unsigned i = 1; i < m_nVertices; ++i) {
       v = vertex(i);
       bbMin.x = std::min(bbMin.x, v.x);  bbMax.x = std::max(bbMax.x, v.x);
       bbMin.y = std::min(bbMin.y, v.y);  bbMax.y = std::max(bbMax.y, v.y);
       bbMin.z = std::min(bbMin.z, v.z);  bbMax.z = std::max(bbMax.z, v.z);
   }
   m_boundingBoxOrigin = bbMin;
   // give ourselves a bit of wiggle room in the box
   bbMax += Vec(0.0001,0.0001,0.0001);

   // Determine the cell length
   v = bbMax-bbMin;

   m_cellLength = std::max(v.x, v.y);
   m_cellLength = std::max(m_cellLength, v.z)/MaxCellCount;

   //double lambda(0.01); 
   //double volume(v.x*v.y*v.z);
   //m_cellLength = std::min(std::pow(lambda*m_nTriangles/volume, 0.333333), m_cellLength);

   // and the number of cells in each direction
   v /= m_cellLength;
   m_nx = v.x;  if (bbMin.x + m_nx*m_cellLength < bbMax.x) ++m_nx;
   m_ny = v.y;  if (bbMin.y + m_ny*m_cellLength < bbMax.y) ++m_ny;
   m_nz = v.z;  if (bbMin.z + m_nz*m_cellLength < bbMax.z) ++m_nz;
}


void AmbientOcclusionEngine::allocateBoxes()
{
   m_boxes = new Boxes(boost::extents[m_nx][m_ny][m_nz]);
   for (unsigned i = 0; i < m_nx; ++i) {
       for (unsigned j = 0; j < m_ny; ++j) {
           for (unsigned k = 0; k < m_nz; ++k) {
               (*m_boxes)[i][j][k] = 0;
           }
       }
   }
}


void AmbientOcclusionEngine::deallocateBoxes()
{
   for (unsigned i = 0; i < m_nx; ++i) {
       for (unsigned j = 0; j < m_ny; ++j) {
           for (unsigned k = 0; k < m_nz; ++k) {
               delete (*m_boxes)[i][j][k];
           }
       }
   }

   delete m_boxes;
}


void AmbientOcclusionEngine::assignTrianglesToBoxes()
{
   // loop over triangles and determine which boxes they might intesect
   Vec a, b, c, bb(m_boundingBoxOrigin);
   unsigned i0, i1, j0, j1, k0, k1;
   double   x0, x1, y0, y1, z0, z1;
   for (unsigned t = 0; t < m_nTriangles; ++t) {
       a = vertex(3*t+0);
       b = vertex(3*t+1);
       c = vertex(3*t+2);

       x0 = std::min(a.x, b.x);   x0 = std::min(x0, c.x);   
       x1 = std::max(a.x, b.x);   x1 = std::max(x1, c.x);
       y0 = std::min(a.y, b.y);   y0 = std::min(y0, c.y);   
       y1 = std::max(a.y, b.y);   y1 = std::max(y1, c.y);
       z0 = std::min(a.z, b.z);   z0 = std::min(z0, c.z);   
       z1 = std::max(a.z, b.z);   z1 = std::max(z1, c.z);

       i0 = (x0-bb.x)/m_cellLength;  i1 = (x1-bb.x)/m_cellLength;
       j0 = (y0-bb.y)/m_cellLength;  j1 = (y1-bb.y)/m_cellLength;
       k0 = (z0-bb.z)/m_cellLength;  k1 = (z1-bb.z)/m_cellLength;
       
	   // for simplicity, all cubes in the bounding box are considered to
	   // contain the triangle
       for (unsigned i = i0; i <= i1; ++i) {
           for (unsigned j = j0; j <= j1; ++j) {
               for (unsigned k = k0; k <= k1; ++k) {
                   if ((*m_boxes)[i][j][k] == 0) (*m_boxes)[i][j][k] = new QList<unsigned>();
                   (*m_boxes)[i][j][k]->append(t);
               }
           }
       }
   }

   // Collect some statistics
   int maxTriangles;
   int count[11] = {0};
   int ntriangles;
   
   for (unsigned i = 0; i < m_nx; ++i) {
       for (unsigned j = 0; j < m_ny; ++j) {
           for (unsigned k = 0; k < m_nz; ++k) {
               ntriangles = ((*m_boxes)[i][j][k] == 0) ? 0 : (*m_boxes)[i][j][k]->size();
               maxTriangles = std::max(maxTriangles, ntriangles);
               ntriangles = std::min(ntriangles,50);
               ntriangles = std::ceil((double)ntriangles/5.0);
               ++count[ntriangles];
           }
       }
   }

   qDebug() << "Total number of triangles" << m_nTriangles;
   qDebug() << "Total number of boxes    " << m_nx*m_ny*m_nz;
   qDebug() << "Empty boxes              " << count[0] 
            << (100.0*count[0])/(m_nx*m_ny*m_nz) << "%";
   for (unsigned i = 0; i < 10; ++i) {
       qDebug() << 5*i+1 <<"-" << 5*i+5 << "  " << count[i+1];
   }
}
           


unsigned AmbientOcclusionEngine::loadLebedevGrid()
{
   Lebedev grid(m_lebedevRule);
   unsigned n(grid.numberOfPoints());

   // only take the positive z hemisphere
   Vec v;
double tot(0.0);
   for (unsigned i = 0; i < n; ++i) {
       v = grid.point(i);
       if (v.z > 0) {
          m_gridPositions.append(v.unit());
          // double the weight as we only integrate the z hemisphere.
          m_gridWeights.append(2.0*grid.weight(i));
          //qDebug() << "Lebedev" << v.x << v.y << v.z << m_gridWeights.last();
          tot += m_gridWeights.last();
       }
   }
          qDebug() << "Lebedev total weight" << tot;
   return m_gridPositions.size();
}


bool AmbientOcclusionEngine::rayIntersectsTriangle(Vec const& origin, Vec const& ray, 
   unsigned const triangle)
{
   Vec a(vertex(3*triangle+0));
   Vec b(vertex(3*triangle+1));
   Vec c(vertex(3*triangle+2));

   Vec ab(b-a);
   Vec ac(c-a);
   Vec pvec(ray^ac);

   double det(ab*pvec);
   if (std::abs(det) < Epsilon) return false;
   double invDet(1.0/det);

   Vec tvec(origin-a);
   double u(tvec*pvec);
   u *= invDet;
   if (u < 0.0 || u > 1.0) return false;

   Vec qvec(tvec^ab);
   double v(invDet*(ray*qvec));
   if (v < 0.0 || u + v > 1.0) return false;

   double t(ac*qvec);

   return t > Epsilon;
}


QList<unsigned> AmbientOcclusionEngine::triangleShortList(Vec const& origin, Vec const& ray)
{
   Vec dir(ray);
   if (abs(dir.x) < Epsilon) dir.x = Epsilon;
   if (abs(dir.y) < Epsilon) dir.y = Epsilon;
   if (abs(dir.z) < Epsilon) dir.z = Epsilon;

   Vec delta;
   delta.x = m_cellLength/abs(dir.x);
   delta.y = m_cellLength/abs(dir.y);
   delta.z = m_cellLength/abs(dir.z);

   Vec t;
   Vec offset(origin-m_boundingBoxOrigin);
   Vec e(dir.x<0 ? 0.0 : 1.0, dir.y<0 ? 0.0 : 1.0, dir.z<0 ? 0.0 : 1.0);
   t.x = ((floor(offset.x/m_cellLength)+e.x)*m_cellLength - offset.x) / dir.x;
   t.y = ((floor(offset.y/m_cellLength)+e.y)*m_cellLength - offset.y) / dir.y;
   t.z = ((floor(offset.z/m_cellLength)+e.z)*m_cellLength - offset.z) / dir.z;

   int i(floor(offset.x/m_cellLength));   
   int j(floor(offset.y/m_cellLength));   
   int k(floor(offset.z/m_cellLength));

   QList<unsigned> triangles;

   while (i >= 0 && i < m_nx && j >= 0 && j < m_ny && k >=0  && k < m_nz) {

      if ((*m_boxes)[i][j][k]) {
          triangles += *((*m_boxes)[i][j][k]);
      }

      if (t.x < t.y && t.x < t.z) {
         t.x += delta.x; 
         i += (dir.x < 0.0) ? -1 : 1;
      }else if (t.y < t.x && t.y < t.z) {
         t.y += delta.y; 
         j += (dir.y < 0.0) ? -1 : 1;
      }else {
         t.z += delta.z; 
         k += (dir.z < 0.0) ? -1 : 1;
      }
   }

   return  triangles;//.toSet().toList();

}

} // end namespace IQmol
