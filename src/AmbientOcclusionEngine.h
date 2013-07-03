#ifndef IQMOL_AMBIENTOCCLUSIONENGINE_H
#define IQMOL_AMBIENTOCCLUSIONENGINE_H
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

#include "Task.h"
#include "QGLViewer/vec.h"
#include "boost/multi_array.hpp"


namespace IQmol {

   class AmbientOcclusionEngine : public Task {

      Q_OBJECT

      public:
         AmbientOcclusionEngine(QList<GLfloat> const& vertexData, unsigned lebedevRule);
         // returns a list of occlusion values for each vertex
         QList<GLfloat> occlusionData() const { return m_data; }

      protected:
         void run();

      private:
         QList<GLfloat> m_vertexData;
         unsigned m_nVertices;
         unsigned m_nTriangles;
         unsigned m_lebedevRule;
         QList<GLfloat> m_data;

         qglviewer::Vec normal(unsigned n) {
           return qglviewer::Vec(m_vertexData[6*n+0], m_vertexData[6*n+1], m_vertexData[6*n+2]);
         }
         qglviewer::Vec vertex(unsigned n) {
           return qglviewer::Vec(m_vertexData[6*n+3], m_vertexData[6*n+4], m_vertexData[6*n+5]);
         }

         // Lebedev weights and roots, returns the number of points
         unsigned loadLebedevGrid();
         QList<double> m_gridWeights;
         QList<qglviewer::Vec> m_gridPositions;

		 // Computes the bounding box for the surface
         void computeBoundingBox();
         qglviewer::Vec m_boundingBoxOrigin;
         unsigned m_nx, m_ny, m_nz;
         double m_cellLength;

		 // This is the data structure that stores a pointer to a list of
		 // triangle indices for each 3D box.  If not triangles are in a given
		 // box, the pointer is just zero.
         typedef boost::multi_array<QList<unsigned>*, 3> Boxes;
         typedef Boxes::index Index;
         void allocateBoxes();
         void deallocateBoxes();
         Boxes* m_boxes;

         void assignTrianglesToBoxes();
         bool rayIntersectsTriangle(qglviewer::Vec const& origin, qglviewer::Vec const& ray, 
            unsigned const triangle);
         QList<unsigned> triangleShortList(qglviewer::Vec const& origin, 
            qglviewer::Vec const& ray);
             
   };

} // end namespace IQmol

#endif
