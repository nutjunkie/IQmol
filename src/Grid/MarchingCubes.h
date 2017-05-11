#ifndef IQMOL_GRID_MARCHING_CUBES_H
#define IQMOL_GRID_MARCHING_CUBES_H
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

#include "Mesh.h"
#include "QGLViewer/vec.h"
#include <QMap>


namespace IQmol {

namespace Data {
   class GridData;
}

   /// Marching cubes algorithm for computing isosurfaces.  Based on the
   /// implementation by Paul Bourke wiki contribution:
   /// Based on the following:
   ///    Qt-Adaption Created on: 15.07.2009  Author: manitoo
   /// Adapted for use with precomputed grids February 2011
   /// Rewritten for Mesh support December 2013
   class MarchingCubes : public QObject {

      Q_OBJECT

      private:
         // Utility class to allow for the creation of a vertex map
         class Index {
            public:
               Index(unsigned const i = 0, unsigned const j = 0, unsigned const k = 0, 
                  unsigned const direction = 0) : m_i(i), m_j(j), m_k(k), m_w(direction) { }

               Index(Index const& that) { copy(that); }

               Index& operator=(Index const& that) 
               {
                   if (this != &that) copy(that);
                   return *this;
               }

               bool operator<(Index const& that) const
               {
                  if (m_i < that.m_i) return true;
                  if (m_i > that.m_i) return false;

                  if (m_j < that.m_j) return true;
                  if (m_j > that.m_j) return false;

                  if (m_k < that.m_k) return true;
                  if (m_k > that.m_k) return false;

                  return (m_w < that.m_w);
               }

               QString toString() const 
               {
                  QString s("(");
                  s += QString::number(m_i);
                  s += ",";
                  s += QString::number(m_j);
                  s += ",";
                  s += QString::number(m_k);
                  if (m_w == 0) s += " / x)";
                  if (m_w == 1) s += " / y)";
                  if (m_w == 2) s += " / z)";
                  return s;
               }

            private:
               unsigned m_i, m_j, m_k, m_w;

               void copy(Index const& that) 
               {
                  m_i = that.m_i;  m_j = that.m_j;  m_k = that.m_k;  m_w = that.m_w;
               }
         };


      public:
         MarchingCubes(Data::GridData const& grid);
         void generateMesh(double const isovalue, Data::Mesh&);


      Q_SIGNALS:
         void progress(double);  // 0.0-1.0


      private:
         /// Performs the Marching Cubes algorithm on a single cube.
         void marchOnCube(int const ix, int const iy, int const iz);

		 /// Finds the approximate point of intersection of the surface between
		 /// two points with the values v1 and v2.
         double getOffset(double const v1, double const v2);

         /// Adds a new vertex to the mesh
         Data::OMMesh::VertexHandle createEdgeVertex(unsigned const edge, double const offset, 
            qglviewer::Vec const& origin);

         // Static Data
         static const double   s_vertexOffset[8][3];
         static const double   s_edgeDirection[12][3];
         static const unsigned s_vertexIndexOffset[8][3];
         static const unsigned s_edgeConnection[12][2];
         static const unsigned s_edgeVertexAssignment[12][2];
         static const int      s_cubeEdgeFlags[256];
         static const int      s_triangleConnectionTable[256][16];

         Data::Mesh*           m_mesh;
         Data::GridData const& m_grid;
         qglviewer::Vec const& m_origin;
         qglviewer::Vec const& m_delta;
         unsigned m_nx, m_ny, m_nz;
         double   m_isovalue;

         QMap<Index, Data::OMMesh::VertexHandle> m_vertexMap;
   };

} // end namespace IQmol

#endif
