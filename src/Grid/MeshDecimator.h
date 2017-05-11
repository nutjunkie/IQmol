#ifndef IQMOL_GRID_MESHDECIMATOR_H
#define IQMOL_GRID_MESHDECIMATOR_H
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

#include "Task.h"
#include "Mesh.h"


namespace IQmol {

   class MeshDecimator {

      public:
         enum Algorithm { AspectRatio, EdgeLength, Hausdorff, NormalDeviation,
            NormalFlipping, Quadric, ProgMesh, IndependentSets, Roundness };

         MeshDecimator(Data::Mesh& mesh);
         MeshDecimator(Data::OMMesh& mesh);
         MeshDecimator(Data::OMMesh& mesh1, Data::OMMesh& mesh2);

         bool decimate(double const edgeThreshold = 0.25);

         QString const& error() const { return m_error; }

         static QString toString(Algorithm const);

      private:
          void decimateEdgeLength(Data::OMMesh&);
          void decimateAspectRatio(Data::OMMesh&);
          void decimateNormalDeviation(Data::OMMesh&);
          void decimateQuadric(Data::OMMesh& mesh);
          void decimateQuadricAndEdge(Data::OMMesh& mesh, double const edgeThreshold);

          QList<Data::OMMesh*> m_meshes;
          QString m_error;
   };


   class MeshDecimatorTask :  public Task {

      Q_OBJECT

      public:
         MeshDecimatorTask(Data::OMMesh& mesh) : m_meshDecimator(mesh) { }

         MeshDecimatorTask(Data::OMMesh& mesh1, Data::OMMesh& mesh2) 
          : m_meshDecimator(mesh1, mesh2) { }

      protected:
         void run() 
         {
            if (!m_meshDecimator.decimate()) {
               setStatus(Error);
               m_info = m_meshDecimator.error();
            }
         }

      private:
         MeshDecimator m_meshDecimator; 
   };


} // end namespace IQmol

#endif
