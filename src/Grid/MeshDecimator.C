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

#include "MeshDecimator.h"
#include "QsLog.h"

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModAspectRatioT.hh>
#include <OpenMesh/Tools/Decimater/ModEdgeLengthT.hh>
#include <OpenMesh/Tools/Decimater/ModHausdorffT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModIndependentSetsT.hh>
#include <OpenMesh/Tools/Decimater/ModRoundnessT.hh>


typedef OpenMesh::Decimater::DecimaterT<IQmol::Data::OMMesh> DecimatorT;


namespace IQmol {

MeshDecimator::MeshDecimator(Data::Mesh& mesh)
{
   m_meshes << &(mesh.data());
}


MeshDecimator::MeshDecimator(Data::OMMesh& mesh)
{
   m_meshes << &mesh;
}


MeshDecimator::MeshDecimator(Data::OMMesh& mesh1, Data::OMMesh& mesh2)
{
   m_meshes << &mesh1 << &mesh2;
}


QString MeshDecimator::toString(Algorithm const algorithm)
{
   QString s;
   switch (algorithm) {
      case AspectRatio:      s = "Aspect Ratio";      break;
      case EdgeLength:       s = "Edge Length";       break;
      case Hausdorff:        s = "Hausdorff";         break;
      case NormalDeviation:  s = "Normal Deviation";  break;
      case NormalFlipping:   s = "Normal Flipping";   break;
      case Quadric:          s = "Quadric";           break;
      case ProgMesh:         s = "Prog Mesh";         break;
      case IndependentSets:  s = "Independent Sets";  break;
      case Roundness:        s = "Roundness";         break;
   }
   return s;
}


bool MeshDecimator::decimate(double const edgeThreshold) 
{
   QList<Data::OMMesh*>::iterator iter;

   for (iter = m_meshes.begin(); iter != m_meshes.end(); ++iter) {
       Data::OMMesh& mesh(*(*iter));

       unsigned nVertices(mesh.n_vertices());

       //decimateQuadric(mesh);
       decimateQuadricAndEdge(mesh, edgeThreshold);
       //decimateNormalDeviation(mesh);
       //decimateEdgeLength(mesh);
       //decimateAspectRatio(mesh);

       double decimated(100.0-mesh.n_vertices()*100.0/nVertices);
       QString pc;
       pc.setNum(decimated, 'f', 1);
       pc += "% removed";
       QLOG_INFO() << "Mesh decimation: " << pc;
   }

   return true;
}



void MeshDecimator::decimateNormalDeviation(Data::OMMesh& mesh) 
{
   // Normal Deviation
   DecimatorT decimator(mesh);
   OpenMesh::Decimater::ModNormalDeviationT<Data::OMMesh>::Handle handle;
   decimator.add(handle);
   decimator.module(handle).set_binary(false);
   decimator.module(handle).set_normal_deviation(4);

   if (!decimator.initialize()) {
      m_error = "Initialization for mesh decimation failed: Normal Deviation module";
      return;
   }

   decimator.decimate();
   mesh.garbage_collection();
}


void MeshDecimator::decimateEdgeLength(Data::OMMesh& mesh) 
{
   DecimatorT decimator(mesh);
   OpenMesh::Decimater::ModEdgeLengthT<Data::OMMesh>::Handle handle;
   decimator.add(handle);
   decimator.module(handle).set_binary(false);
   decimator.module(handle).set_edge_length(0.1);

   if (!decimator.initialize()) {
      m_error = "Initialization for mesh decimation failed: Edge Length module";
      return;
   }

   decimator.decimate();
   mesh.garbage_collection();
}


void MeshDecimator::decimateAspectRatio(Data::OMMesh& mesh) 
{
   DecimatorT decimator(mesh);
   OpenMesh::Decimater::ModAspectRatioT<Data::OMMesh>::Handle handle;
   decimator.add(handle);
   decimator.module(handle).set_binary(false);
   decimator.module(handle).set_aspect_ratio(10.0);

   if (!decimator.initialize()) {
      m_error = "Initialization for mesh decimation failed: Aspect Ratio module";
      return;
   }

   decimator.decimate();
   mesh.garbage_collection();
}


void MeshDecimator::decimateQuadric(Data::OMMesh& mesh) 
{
   DecimatorT decimator(mesh);
   OpenMesh::Decimater::ModQuadricT<Data::OMMesh>::Handle handle;
   decimator.add(handle);
   decimator.module(handle).set_binary(false);
   decimator.module(handle).set_max_err(0.0001);

   if (!decimator.initialize()) {
      m_error = "Initialization for mesh decimation failed: Quadric module";
      return;
   }

   decimator.decimate();
   mesh.garbage_collection();
}


void MeshDecimator::decimateQuadricAndEdge(Data::OMMesh& mesh, double const edgeThreshold) 
{
   DecimatorT decimator(mesh);

   OpenMesh::Decimater::ModQuadricT<Data::OMMesh>::Handle handle;
   decimator.add(handle);
   decimator.module(handle).set_binary(false);
   decimator.module(handle).set_max_err(0.0001);

   OpenMesh::Decimater::ModEdgeLengthT<Data::OMMesh>::Handle ehandle;
   decimator.add(ehandle);
   decimator.module(ehandle).set_binary(true);
   decimator.module(ehandle).set_edge_length(edgeThreshold);

   if (!decimator.initialize()) {
      m_error = "Initialization for mesh decimation failed: Quadric module";
      return;
   }

   decimator.decimate();
   mesh.garbage_collection();
}

} // end namespace IQmol
