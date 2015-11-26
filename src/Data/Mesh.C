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

#include "OpenMesh/Core/IO/MeshIO.hh"
#include "Mesh.h"
#include "QsLog.h"
#include <string>
#include <sstream>
#include <climits>
#include <QDebug>
#include <exception>


using qglviewer::Vec;

namespace IQmol {
namespace Data {

std::string const Mesh::s_archiveFormat       = ".obj";
std::string const Mesh::s_scalarFieldString   = "ScalarField";
std::string const Mesh::s_faceCentroidsString = "FaceCentroids";
std::string const Mesh::s_meshIndexString     = "MeshIndex";

double const Mesh::s_thresh = 0.000001;

template<> const Type::ID MeshList::TypeID = Type::MeshList;


Mesh::Mesh()
{
   requestDefaultProperties();
}


Mesh::Mesh(Mesh const& that) : Base() 
{ 
   requestDefaultProperties();
   copy(that); 
}


void Mesh::requestDefaultProperties()
{
   if (!requestProperty(VertexNormals)) {
      throw std::runtime_error("Mesh vertex normals unavailable");
   }

   if (!requestProperty(FaceNormals)) {
      throw std::runtime_error("Mesh face normals unavailable");
   }
  
   if (!requestProperty(FaceCentroids)) {
      throw std::runtime_error("Mesh face centroids available");
   }
}


bool Mesh::requestProperty(Property const property)
{
   switch (property) {
      case VertexNormals:
         m_omMesh.request_vertex_normals();
         return m_omMesh.has_vertex_normals();
         break;

      case FaceNormals:
         m_omMesh.request_face_normals();
         return m_omMesh.has_face_normals();
         break;

      case FaceCentroids:
         m_omMesh.add_property(m_faceCentroidsHandle, s_faceCentroidsString);
         return m_faceCentroidsHandle.is_valid();
         break;

      case ScalarField:
         m_omMesh.add_property(m_scalarFieldHandle, s_scalarFieldString);
         //m_omMesh.property(m_scalarFieldHandle).set_persistent(true);
         return m_scalarFieldHandle.is_valid();
         break;

      case MeshIndex:
         m_omMesh.add_property(m_meshIndexHandle, s_meshIndexString);
         //m_omMesh.property(m_meshIndexHandle).set_persistent(true);
         return m_meshIndexHandle.is_valid();
         break;
   }

   QLOG_WARN() << "Request for Mesh property failed" << property;
   return false;
}

 
Mesh& Mesh::operator=(Mesh const& that)
{
   if (this != &that) copy(that);
   return *this;
}


void Mesh::copy(Mesh const& that)
{
//   qDebug() << "Invoking Mesh::copy()";
   deleteProperty(ScalarField);
   deleteProperty(MeshIndex);

   if (that.hasProperty(ScalarField)) {
      if (requestProperty(MeshIndex)) {
         // copy property
      }
   }

   if (that.hasProperty(MeshIndex)) {
      if (requestProperty(MeshIndex)) {
         // copy property
      }
   }

   m_omMesh = that.m_omMesh;
}


Mesh& Mesh::operator+=(Mesh const& that)
{
//   qDebug() << "Invoking Mesh::operator+=";
   QMap<Vertex, Vertex> vertexMap;
   QMap<Face, Face> faceMap;

   Vertex oldVertex;
   Vertex newVertex;
   Normal normal;
   Point  point;

   OMMesh::ConstVertexIter iter;

   for (iter = that.vbegin(); iter != that.vend(); ++iter) {
       point     = that.vertex(iter);
       normal    = that.normal(iter);
       oldVertex = iter.handle();
       newVertex = m_omMesh.add_vertex(point);
       m_omMesh.set_normal(newVertex, normal);
       vertexMap.insert(oldVertex, newVertex);
   }

   OMMesh::ConstFaceVertexIter fvIter;
   OMMesh::ConstFaceIter face;
   Vertex v0, v1, v2;

   for (face = that.fbegin(); face != that.fend(); ++face) {
       fvIter = that.m_omMesh.cfv_iter(face.handle());

       v0 = vertexMap.value(fvIter.handle());  ++fvIter;
       v1 = vertexMap.value(fvIter.handle());  ++fvIter;
       v2 = vertexMap.value(fvIter.handle());

       if (v0.is_valid() && v1.is_valid() && v2.is_valid()) {
          faceMap.insert(face.handle(), addFace(v0, v1, v2));
       }else {
          QLOG_WARN() << "Found invalid vertex index:" << v0.idx() << v1.idx() << v2.idx()
                      << " on face" << face.handle().idx();
       }
   }

   // Copy properties
   if (that.hasProperty(MeshIndex)) {
      if (!hasProperty(MeshIndex) && !requestProperty(MeshIndex)) {
         QLOG_WARN() << "Mesh request for index property failed";
      }else {
         for (face = that.fbegin(); face != that.fend(); ++face) {
             int index(that.meshIndex(face.handle()));
             m_omMesh.property(m_meshIndexHandle, faceMap.value(face.handle())) = index;
         }
      }
   }

   if (that.hasProperty(ScalarField)) {
      if (!hasProperty(ScalarField) && !requestProperty(ScalarField)) {
         QLOG_WARN() << "Mesh request for scalar field property failed";
      }else {
         OMMesh::ConstVertexIter vertex;
         for (vertex = that.vbegin(); vertex != that.vend(); ++vertex) {
             double value(that.scalarFieldValue(vertex.handle()));
             m_omMesh.property(m_scalarFieldHandle, vertexMap.value(vertex.handle())) = value;
         }
      }
   }

   return *this;
}


bool Mesh::hasProperty(Property const property) const
{
   switch (property) {
      case FaceNormals:
         return m_omMesh.has_face_normals();
         break;
      case VertexNormals:
         return m_omMesh.has_vertex_normals();
         break;
      case FaceCentroids:
         return m_faceCentroidsHandle.is_valid();
         break;
      case ScalarField:
         return m_scalarFieldHandle.is_valid();
         break;
      case MeshIndex:
         return m_meshIndexHandle.is_valid();
         break;
   }
   return false;
}


void Mesh::deleteProperty(Property const property)
{
   switch (property) {
      case ScalarField:
         if (m_scalarFieldHandle.is_valid()) {
            m_omMesh.property(m_scalarFieldHandle).clear();   
            m_scalarFieldHandle.invalidate();
         }
         break;

      case MeshIndex:
         if (m_meshIndexHandle.is_valid()) {
            m_omMesh.property(m_meshIndexHandle).clear();   
            m_meshIndexHandle.invalidate();
         }
         break;

      case FaceNormals:
      case VertexNormals:
      case FaceCentroids:
         break;
   }
}


void Mesh::serialize(OutputArchive& ar, unsigned const) 
{
   // This is not pretty, we use the OpenMesh routine to save to a std::string
   // and then save that string to the archive.
   OpenMesh::IO::Options options;
   options += OpenMesh::IO::Options::VertexNormal;

   std::stringstream osstream(std::ios_base::out);
   if (OpenMesh::IO::write_mesh(m_omMesh, osstream, s_archiveFormat, options)) {
      std::string s(osstream.str());
      ar & s;

	  // Can't seem to get the OpenMesh library to save custom properties, so
	  // we do it manually.
      QList<double> values;
      if (hasProperty(ScalarField)) {
         OMMesh::ConstVertexIter vertex(m_omMesh.vertices_begin());
         for (; vertex != m_omMesh.vertices_end(); ++vertex) {
             values << scalarFieldValue(vertex);
         }
      }
      ar & values;

      QList<int> indices;
      if (hasProperty(MeshIndex)) {
         OMMesh::ConstFaceIter face(m_omMesh.faces_begin());
         for (; face != m_omMesh.faces_end(); ++face) {
             values << meshIndex(face);
         }
      }
      ar & indices;

      QLOG_INFO() << "Mesh write to archive successful";
   }else {
      QLOG_ERROR() << "Mesh write to archive failed";
   }
}


void Mesh::serialize(InputArchive& ar, unsigned const) 
{
   std::string s;
   QList<double> values;
   QList<int> indices;

   ar & s;
   ar & values;
   ar & indices;

   OpenMesh::IO::Options options;
   options += OpenMesh::IO::Options::VertexNormal;
   std::stringstream isstream(s, std::ios_base::in);

   if (OpenMesh::IO::read_mesh(m_omMesh, isstream, s_archiveFormat, options)) {

      computeFaceNormals();

      if ((int)m_omMesh.n_vertices() == values.size()) {
         if (requestProperty(ScalarField)) {
            unsigned i(0);
            OMMesh::ConstVertexIter vertex(m_omMesh.vertices_begin());
            for (; vertex != m_omMesh.vertices_end(); ++vertex, ++i) {
                m_omMesh.property(m_scalarFieldHandle, *vertex) = values.at(i);
            }
         }
      }

      if ((int)m_omMesh.n_faces() == indices.size()) {
         if (requestProperty(MeshIndex)) {
            unsigned i(0);
            OMMesh::ConstFaceIter face(m_omMesh.faces_begin());
            for (; face != m_omMesh.faces_end(); ++face, ++i) {
                m_omMesh.property(m_meshIndexHandle, *face) = indices.at(i);
            }
         }
      }

      QLOG_INFO() << "Mesh read from archive successful";

   }else {
      QLOG_ERROR() << "Mesh read from archive failed";
   }
}


void Mesh::writeToFile() 
{
   OpenMesh::IO::Options options;
   options += OpenMesh::IO::Options::VertexNormal;

   std::stringstream osstream(std::ios_base::out);

   if (OpenMesh::IO::write_mesh(m_omMesh, "mesh.stl", options)) {
      qDebug() << "Mesh written to mesh.stl";
   }else {
      qDebug() << "Mesh write to file failed";
   }
}


bool Mesh::computeScalarField(Function3D const& function)
{
   if (!hasProperty(ScalarField) && !requestProperty(ScalarField))  return false;

   OMMesh::ConstVertexIter vertex;
   for (vertex = m_omMesh.vertices_begin(); vertex != m_omMesh.vertices_end(); ++vertex) {
       OMMesh::Point const& p(m_omMesh.point(vertex));
       m_omMesh.property(m_scalarFieldHandle, *vertex) = function(p[0], p[1], p[2]);
   }

   return true;
}


bool Mesh::computeIndexField()
{
   if (!hasProperty(MeshIndex) && !requestProperty(MeshIndex))  return false;
   if (!hasProperty(ScalarField) && !requestProperty(ScalarField))  return false;

   OMMesh::FaceIter face;
   OMMesh::FaceVertexIter vertex;
   for (face = fbegin(); face != fend(); ++face) {
       int index( m_omMesh.property(m_meshIndexHandle, face));
       // add a little bit to avoid round-off changing the int value
       double value(index + 0.0001);
   
       vertex = m_omMesh.fv_iter(face);
       m_omMesh.property(m_scalarFieldHandle, vertex) = value;
       ++vertex;
       m_omMesh.property(m_scalarFieldHandle, vertex) = value;
       ++vertex;
       m_omMesh.property(m_scalarFieldHandle, vertex) = value;
   }

   return true;
}


bool Mesh::setMeshIndex(int const index)
{
   if (!hasProperty(MeshIndex) && !requestProperty(MeshIndex)) return false;

   OMMesh::FaceIter face;
   for (face = fbegin(); face != fend(); ++face) {
       m_omMesh.property(m_meshIndexHandle, face) = index;
   }

   return true;
}


int Mesh::meshIndex(Face const& face) const
{
  return m_omMesh.property(m_meshIndexHandle, face);
}


void Mesh::getScalarFieldRange(double& min, double& max)
{
   if (hasProperty(ScalarField)) {
      min = DBL_MAX;
      max = DBL_MIN;

      OMMesh::ConstVertexIter vertex;
      for (vertex = m_omMesh.vertices_begin(); vertex != m_omMesh.vertices_end(); ++vertex) {
          double f(scalarFieldValue(*vertex));
          min = std::min(min, f);
          max = std::max(max, f);
      }
   }else {
      min = 0.0;
      max = 0.0;
   }
}


double Mesh::scalarFieldValue(Vertex const& vertex) const
{
   return m_omMesh.property(m_scalarFieldHandle, vertex);
}


// Access and setters

Mesh::Vertex Mesh::addVertex(double const x, double const y, double const z)
{
   return m_omMesh.add_vertex(OMMesh::Point(x, y, z));
} 


Mesh::Vertex Mesh::addVertex(Point const p)
{
   return m_omMesh.add_vertex(p);
}


Mesh::Face Mesh::addFace(Vertex const& v0, Vertex const& v1, Vertex const& v2)
{
   Face face(m_omMesh.add_face(v0, v1, v2));

   Point A(m_omMesh.point(v0));
   Point B(m_omMesh.point(v1));
   Point C(m_omMesh.point(v2));
       
   m_omMesh.property(m_faceCentroidsHandle, face) = (A+B+C)/3.0;
   A = (B-A)%(C-A);
   m_omMesh.set_normal(face, A.normalize());

   return face;
} 


void Mesh::setNormal(Vertex const& handle, double dx, double dy, double dz)
{
   m_omMesh.set_normal(handle, Normal(dx, dy, dz));
}


void Mesh::setNormal(Vertex const& handle, Normal const& normal)
{
   m_omMesh.set_normal(handle, normal);
}

void Mesh::setPoint(Vertex const& handle, Point const& p)
{
   m_omMesh.set_point(handle, p);
} 


Mesh::Point const& Mesh::vertex(Vertex const& vertex) const
{
   return m_omMesh.point(vertex);
}


Mesh::Normal const& Mesh::normal(Vertex const& vertex) const
{
   return m_omMesh.normal(vertex);
}


Mesh::Normal const& Mesh::normal(Face const& face) const
{
   return m_omMesh.normal(face);
}


Mesh::Point const& Mesh::faceCentroid(Face const& face) const
{
   return m_omMesh.property(m_faceCentroidsHandle, face);
}


bool Mesh::computeFaceNormals()
{
   if (!hasProperty(FaceCentroids) || !hasProperty(FaceNormals)) {
      QLOG_ERROR() << "Mesh face centroid request failed";
      return false;
   }

   OMMesh::ConstFaceIter face;
   OMMesh::ConstFaceVertexIter vertex;
   Point A, B, C;

   for (face = m_omMesh.faces_begin(); face != m_omMesh.faces_end(); ++face) {
       vertex = m_omMesh.cfv_iter(face.handle());
       A = m_omMesh.point(vertex);   ++vertex;
       B = m_omMesh.point(vertex);   ++vertex;
       C = m_omMesh.point(vertex);
       
       m_omMesh.property(m_faceCentroidsHandle, face) = (A+B+C)/3.0;
   }
   m_omMesh.update_face_normals();

   return true;
}


void Mesh::removeDisconnectedVertices()
{
   unsigned orphans(0);

   OMMesh::VertexIter vi;
   OMMesh::VertexVertexIter vvi;

   for (vi = vbegin(); vi != vend(); ++vi) {
       unsigned count(0);
       for (vvi = m_omMesh.vv_iter(vi); vvi; ++vvi) { ++count; }
       if (count == 0) {
          ++orphans;
          m_omMesh.delete_vertex(vi, false);
       }
   }

   m_omMesh.garbage_collection();
   //qDebug() << "Mesh cliping removed" << orphans << "vertices";
}


void Mesh::clip(Vec const& normal, Vec const& pointOnPlane)
{
   Point  p0(pointOnPlane[0], pointOnPlane[1], pointOnPlane[2]);
   Normal n(normal[0], normal[1], normal[2]);
   bool clipA, clipB, clipC, planeA, planeB, planeC;
   double d;

   OMMesh::FaceIter face;
   OMMesh::FaceVertexIter vertex;
   Vertex Av, Bv, Cv, v1, v2;
   Point Ap, Bp, Cp;

   NewVertexMap newVertexMap;

   for (face = m_omMesh.faces_begin(); face != m_omMesh.faces_end(); ++face) {
       vertex = m_omMesh.fv_iter(face);

       Av = vertex.handle(); ++vertex;
       Bv = vertex.handle(); ++vertex;
       Cv = vertex.handle();

       Ap = m_omMesh.point(Av);
       Bp = m_omMesh.point(Bv);
       Cp = m_omMesh.point(Cv);

       d      = n|(Ap-p0);
       planeA = std::abs(d) < s_thresh;
       clipA  = !planeA && (d < 0.0);

       d      = n|(Bp-p0);
       planeB = std::abs(d) < s_thresh;
       clipB  = !planeB && (d < 0.0);

       d      = n|(Cp-p0);
       planeC = std::abs(d) < s_thresh;
       clipC  = !planeC && (d < 0.0);

       unsigned clip  = (clipA  ? 1 : 0) + (clipB  ? 1 : 0) + (clipC  ? 1 : 0);
       unsigned plane = (planeA ? 1 : 0) + (planeB ? 1 : 0) + (planeC ? 1 : 0);

       if (clip + plane == 3) {
          m_omMesh.delete_face(face, false);
  
       }else if (clip == 1 && plane == 0) {
          m_omMesh.delete_face(face, false);

          if (clipA) {
             v1 = createLinePlaneIntersection(n, p0, Av, Bv, newVertexMap);
             v2 = createLinePlaneIntersection(n, p0, Av, Cv, newVertexMap);
             addFace(v1, Bv, Cv);
             addFace(v1, Cv, v2);
          }else if (clipB) {
             v1 = createLinePlaneIntersection(n, p0, Bv, Cv, newVertexMap);
             v2 = createLinePlaneIntersection(n, p0, Bv, Av, newVertexMap);
             addFace(v1, Cv, Av);
             addFace(v1, Av, v2);
          }else if (clipC) {
             v1 = createLinePlaneIntersection(n, p0, Cv, Av, newVertexMap);
             v2 = createLinePlaneIntersection(n, p0, Cv, Bv, newVertexMap);
             addFace(v1, Av, Bv);
             addFace(v1, Bv, v2);
          }

       }else if (clip == 1 && plane == 1) {
          m_omMesh.delete_face(face, false);

          if (clipA) {
             if (planeB) {
                moveToLinePlaneIntersection(n, p0, Av, Cv);
             }else {
                moveToLinePlaneIntersection(n, p0, Av, Bv);
             }
          }else if (clipB) {
             if (planeA) {
                moveToLinePlaneIntersection(n, p0, Bv, Cv);
             }else {
                moveToLinePlaneIntersection(n, p0, Bv, Av);
             }
          }else if (clipC) {
             if (planeA) {
                moveToLinePlaneIntersection(n, p0, Cv, Bv);
             }else {
                moveToLinePlaneIntersection(n, p0, Cv, Av);
             }
          }

       }else if (clip == 2 && plane == 0) {
          m_omMesh.delete_face(face, false);

          if (!clipA) {
             v1 = createLinePlaneIntersection(n, p0, Cv, Av, newVertexMap);
             v2 = createLinePlaneIntersection(n, p0, Bv, Av, newVertexMap);
             addFace(v1, Av, v2);
          }else if (!clipB) {
             v1 = createLinePlaneIntersection(n, p0, Av, Bv, newVertexMap);
             v2 = createLinePlaneIntersection(n, p0, Cv, Bv, newVertexMap);
             addFace(v1, Bv, v2);
          }else if (!clipC) {
             v1 = createLinePlaneIntersection(n, p0, Bv, Cv, newVertexMap);
             v2 = createLinePlaneIntersection(n, p0, Av, Cv, newVertexMap);
             addFace(v1, Cv, v2);
          }

       }
   }
   
   m_omMesh.garbage_collection();
   removeDisconnectedVertices();
}


Mesh::Point Mesh::linePlaneIntersection(Normal const& n, Point const& p0, Point const& A, 
   Point const& B, double& d) 
{
   d = 0.0;
   Point AB(B-A);
   double numerator((n|(p0-A)));

   // Bail early if A is on the plane
   if (std::abs(numerator) < s_thresh) return A;

   double denominator(n|AB);

   if (std::abs(denominator) < s_thresh) { 
	  // AB is parallel to the plane, shouldn't happen as A & B should be on
	  // opposite sides of the plane.
      QLOG_WARN() << "Found parallel line segment in Mesh intersector";
      qDebug() << "       n" <<  n[0] <<  n[1] <<  n[2];
      qDebug() << "      p0" << p0[0] << p0[1] << p0[2];
      qDebug() << "       A" <<  A[0] <<  A[1] <<  A[2];
      qDebug() << "       B" <<  B[0] <<  B[1] <<  B[2];
      return A;
   }

   d = numerator/denominator;
   return A + AB*d;
}


Mesh::Vertex Mesh::createLinePlaneIntersection(Normal const& n, Point const& p0, 
   Vertex const& Av, Vertex const& Bv, NewVertexMap& map)
{
   if (map.contains(Edge(Av, Bv))) {
      return map.value(Edge(Av,Bv));
   }else if (map.contains(Edge(Bv,Av))) {
      return map.value(Edge(Bv,Av));
   }

   double d;
   Point  Ap(m_omMesh.point(Av));
   Point  Bp(m_omMesh.point(Bv));
   Normal An(m_omMesh.normal(Av));
   Normal Bn(m_omMesh.normal(Bv));

   Vertex v(m_omMesh.add_vertex(linePlaneIntersection(n, p0, Ap, Bp, d)));
   m_omMesh.set_normal(v, An*(1.0-d) + Bn*d);
   map.insert(Edge(Av,Bv), v);
   return v;
}


void Mesh::moveToLinePlaneIntersection(Normal const& n, Point const& p0, Vertex& Av, 
   Vertex const& Bv)
{
   double d;
   Point  Ap(m_omMesh.point(Av));
   Point  Bp(m_omMesh.point(Bv));
   Normal An(m_omMesh.normal(Av));
   Normal Bn(m_omMesh.normal(Bv));

   m_omMesh.set_point (Av, linePlaneIntersection(n, p0, Ap, Bp, d));
   m_omMesh.set_normal(Av, An*(1.0-d) + Bn*d);
}


double Mesh::surfaceArea() const
{
   double area(0.0);

   OMMesh::ConstFaceIter face;
   OMMesh::ConstFaceVertexIter vertex;

   for (face = m_omMesh.faces_begin(); face != m_omMesh.faces_end(); ++face) {
       vertex = m_omMesh.cfv_iter(face.handle());
       Vec a(m_omMesh.point(vertex)[0], m_omMesh.point(vertex)[1], m_omMesh.point(vertex)[2]);
       ++vertex;
       Vec b(m_omMesh.point(vertex)[0], m_omMesh.point(vertex)[1], m_omMesh.point(vertex)[2]);
       ++vertex;
       Vec c(m_omMesh.point(vertex)[0], m_omMesh.point(vertex)[1], m_omMesh.point(vertex)[2]);

       a = a-b;
       b = b-c;
       area += cross(a,b).norm();
   }

   return area;
}


void Mesh::dump() const
{
   qDebug() << "Mesh supports:";
   qDebug() << " - vertex normals    " << hasProperty(VertexNormals);
   qDebug() << " - face normals      " << hasProperty(FaceNormals);
   qDebug() << " - face centroids    " << hasProperty(FaceCentroids);
   qDebug() << " - scalar field      " << hasProperty(ScalarField);
   qDebug() << " - mesh index        " << hasProperty(MeshIndex);
   qDebug() << "   Number of vertices" << m_omMesh.n_vertices();
   qDebug() << "   Number of edges   " << m_omMesh.n_edges();
   qDebug() << "   Number of faces   " << m_omMesh.n_faces();
}

} } // end namespace IQmol::Data
