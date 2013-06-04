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

#include "SurfaceLayer.h"
#include "Preferences.h"
#include "QsLog.h"
#include "AmbientOcclusionEngine.h"
#include "QGLViewer/vec.h"
#include <QColorDialog>
#include <cmath>

#include <QFile>
#include <QTextStream>

using namespace qglviewer;

namespace IQmol {
namespace Layer {

Surface::Surface(Grid::DataType const type, int const quality, double const isovalue, 
   QColor const& positive, QColor const& negative, bool const upsample) 
   : m_type(type), m_quality(quality), m_isovalue(isovalue), m_drawMode(Fill), 
     m_upsample(upsample), m_configurator(this), m_callListPositive(0), 
     m_callListNegative(0), m_areaPositive(0.0), m_areaNegative(0.0), 
     m_cubeIsSigned(true), m_aoEngine(0)
{ 
   setFlags(Qt::ItemIsUserCheckable);
   setCheckState(Qt::Unchecked);
   setColor(positive, Positive); 
   setColor(negative, Negative); 
   setConfigurator(&m_configurator); 
   setText(m_type.info());
   m_configurator.setWindowTitle("Configure Surface: " + m_type.info());
   createToolTip();
   // This is slightly less than 1 to ensure all surfaces are drawn after the
   // Primitives (which have alpha = 1.0) when sorted in the ViewerModel, but
   // because it is > 0.99 it won't tigger the transparency overhead in draw();
   setAlpha(0.999); 
   updated();
}


Surface::~Surface()
{
   if (m_callListPositive) glDeleteLists(m_callListPositive, 1);
   if (m_callListNegative) glDeleteLists(m_callListNegative, 1);
}


void Surface::createToolTip(QString const& label)
{
   QString info = label.isEmpty() ? m_type.info() : label; 

   if (m_quality < 0) {
      info += "\nQuality   = default";
   }else {
      info += "\nQuality   = " + QString::number(m_quality);
   }
   info += "\nIsovalue = " + QString::number(m_isovalue, 'f', 3);
   setData(info, Qt::ToolTipRole);
}


void Surface::setSurfaceData(Surface::Data const& data, Sign sign)
{
   double surfaceArea(0.0);
   for (int i = 0; i < data.size(); i += 18) {
       Vec a( data[i+ 3], data[i+ 4], data[i+ 5] );
       Vec b( data[i+ 9], data[i+10], data[i+11] );
       Vec c( data[i+15], data[i+16], data[i+17] );
       a = a-b;
       b = b-c;
       surfaceArea += cross(a,b).norm();
   }

   surfaceArea *= 0.5;

   switch (sign) {
      case Positive:
         m_surfaceDataPositive = data;
         if (m_callListPositive) glDeleteLists(m_callListPositive, 1);
         m_callListPositive = compile(m_surfaceDataPositive);
         m_areaPositive = surfaceArea;
         break;
      case Negative:
         m_surfaceDataNegative = data;
         if (m_callListNegative) glDeleteLists(m_callListNegative, 1);
         m_callListNegative = compile(m_surfaceDataNegative);
         m_areaNegative = surfaceArea;
         break;
   }

   m_configurator.setArea(m_areaPositive+m_areaNegative);
}


void Surface::setAlpha(double alpha) 
{
   m_alpha = alpha;
   m_colorPositive[3] = alpha;
   m_colorNegative[3] = alpha;
   recompile();
}


void Surface::setColor(QColor const& color, Sign sign)
{
   switch (sign) {
      case Positive:
         m_colorPositive[0] = color.redF();
         m_colorPositive[1] = color.greenF();
         m_colorPositive[2] = color.blueF();
         break;
      case Negative:
         m_colorNegative[0] = color.redF();
         m_colorNegative[1] = color.greenF();
         m_colorNegative[2] = color.blueF();
         break;
   }
}


QColor Surface::color(Sign sign) const
{ 
   QColor color;
   switch (sign) {
      case Positive:
         color.setRgbF(m_colorPositive[0], m_colorPositive[1], m_colorPositive[2]); 
         break;
      case Negative:
         color.setRgbF(m_colorNegative[0], m_colorNegative[1], m_colorNegative[2]); 
         break;
   }
   return color;
}


void Surface::drawFast()
{
   draw();
}


void Surface::drawSelected()
{
   return;
   draw();
}


void Surface::draw() 
{
   if ( (checkState() != Qt::Checked) || !m_callListPositive || m_alpha < 0.01) return;
//------------------------------------------------------------

   GLint program;
   glGetIntegerv(GL_CURRENT_PROGRAM, &program);
   GLint aoLoc(glGetAttribLocation(program, "ambient_occlusion"));

//qDebug() << "GL currently using program" << program;
//qDebug() << "  with AO data in location" << aoLoc;

   GLuint buffer;

   if (aoLoc > 0 && m_includeAmbientOcclusion && !m_occlusionData.isEmpty()) {
      int nVertices(m_surfaceDataPositive.size() + m_surfaceDataNegative.size() );
      nVertices /= 6;
      qDebug() << "binding data" << m_occlusionData.first();
      
      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
//      glBufferData(GL_ARRAY_BUFFER, nVertices*sizeof(GLfloat), m_aoData, GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glVertexAttribPointer(aoLoc, 1, GL_FLOAT, 0, 0, 0);
      glEnableVertexAttribArray(aoLoc);
   }

//------------------------------------------------------------



   GLboolean lighting;
   GLboolean blend;

   glGetBooleanv(GL_LIGHTING, &lighting);
   glGetBooleanv(GL_BLEND, &blend);


   if (isTransparent()) {
      glEnable(GL_BLEND);
      glDepthMask(GL_TRUE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_CULL_FACE);
   }

   switch (m_drawMode) {
      case Fill:  
         glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
         break;
      case Lines: 
         glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glEnable(GL_LINE_SMOOTH);
         glLineWidth(2.0);
         break;
      case Dots:  
         glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); 
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glDisable(GL_LIGHTING);
         glEnable(GL_POINT_SMOOTH);
         glPointSize(3.0);
         break;
   }

   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   if (m_callListPositive) {
      if (isTransparent()) glCullFace(GL_FRONT);
      glColor4fv(m_colorPositive);
      glCallList(m_callListPositive);
      if (isTransparent()) {
         glCullFace(GL_BACK);
         glCallList(m_callListPositive);
      }
   }

   if (m_callListNegative) {
      if (isTransparent()) glCullFace(GL_FRONT);
      glColor4fv(m_colorNegative);
      glCallList(m_callListNegative);
      if (isTransparent()) {
         glCullFace(GL_BACK);
         glCallList(m_callListNegative);
         glDisable(GL_CULL_FACE);
      }
   }

   if (m_includeAmbientOcclusion) glDeleteBuffers(1, &buffer);

   glPopMatrix();
   if (!blend) glDisable(GL_BLEND);
   if (lighting) glEnable(GL_LIGHTING);
   glDisable(GL_CULL_FACE);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
}


bool Surface::operator==(Surface const& that)
{
   return ( (m_type == that.m_type) && (m_quality == that.m_quality) && 
            (m_isovalue == that.m_isovalue) );
}


void Surface::recompile()
{
   if (m_callListPositive) glDeleteLists(m_callListPositive, 1);
   if (m_callListNegative) glDeleteLists(m_callListNegative, 1);
   m_callListPositive = compile(m_surfaceDataPositive, m_propertyDataPositive);
   m_callListNegative = compile(m_surfaceDataNegative, m_propertyDataNegative);
}


GLuint Surface::compile(Surface::Data const& data, Surface::Data const& property)
{
   GLint setID = glGenLists(1);
   glNewList(setID, GL_COMPILE);
      glBegin(GL_TRIANGLES);
         if (property.isEmpty()) {
            for (int i = 0; i < data.size(); i += 6) {
                glNormal3f( data[i+0], data[i+1], data[i+2] );
                glVertex3f( data[i+3], data[i+4], data[i+5] );
            }
         }else {
            double min(m_min), max(m_max);
            Gradient::Function gradient(m_colors, min, max);
            QColor color;
            for (int i = 0; i < property.size(); ++i) {
                color = gradient.colorAt(property[i]);
                glNormal3f( data[6*i+0], data[6*i+1], data[6*i+2] );
                glColor4f ( color.redF(), color.greenF(), color.blueF(), m_alpha); 
                glVertex3f( data[6*i+3], data[6*i+4], data[6*i+5] );
            }
         }
      glEnd();
   glEndList();
   //qDebug() << "Surface compiled with" << property.size() << "vertices";

   return setID;
}


void Surface::clearPropertyData()
{
   m_propertyDataPositive.clear();
   m_propertyDataNegative.clear();
   recompile();
}


void Surface::computePropertyData(Function3D function) 
{
   if (m_surfaceDataPositive.isEmpty() && m_surfaceDataNegative.isEmpty()) {
      QLOG_DEBUG() << "Layer::Surface::computePropertyData() called with no data";
      return;
   }

   m_propertyDataPositive.clear();
   m_propertyDataNegative.clear();

   double f;
   if (m_surfaceDataPositive.isEmpty()) {
      f = function(m_surfaceDataNegative[3], 
                   m_surfaceDataNegative[4], 
                   m_surfaceDataNegative[5]);
   }else {
      f = function(m_surfaceDataPositive[3], 
                   m_surfaceDataPositive[4], 
                   m_surfaceDataPositive[5]);
   }

   m_min = f;
   m_max = f;

   for (int i = 0; i < m_surfaceDataPositive.size(); i += 6) {
       f = function(m_surfaceDataPositive[i+3], m_surfaceDataPositive[i+4], 
             m_surfaceDataPositive[i+5]);
       m_propertyDataPositive.append(f);
       m_min = std::min(m_min, f);
       m_max = std::max(m_max, f);
   }

   for (int i = 0; i < m_surfaceDataNegative.size(); i += 6) {
       f = function(m_surfaceDataNegative[i+3], m_surfaceDataNegative[i+4], 
             m_surfaceDataNegative[i+5]);
       m_propertyDataNegative.append(f);
       m_min = std::min(m_min, f);
       m_max = std::max(m_max, f);
   }

   recompile(); 
}


void Surface::addAmbientOcclusion(bool tf) 
{
   m_includeAmbientOcclusion = tf;
   if (m_includeAmbientOcclusion && m_occlusionData.isEmpty()) {
      computeAmbientOcclusion();
   }else {
      recompile();
   }
}


void Surface::computeAmbientOcclusion()
{
   if (m_aoEngine) return;
   Data bothSurfaces(m_surfaceDataPositive);
   bothSurfaces.append(m_surfaceDataNegative);
   m_aoEngine = new AmbientOcclusionEngine(bothSurfaces, 15);
   connect(m_aoEngine, SIGNAL(finished()), this, SLOT(ambientOcclusionDataAvailable()));
   m_aoEngine->start();
}


void Surface::ambientOcclusionDataAvailable()
{
   qDebug() << "Ambient Occlusion data calculated in" << m_aoEngine->timeTaken() << "seconds";
   m_occlusionData = m_aoEngine->occlusionData();

   int nPos(m_surfaceDataPositive.size()/6);
   int nNeg(m_surfaceDataNegative.size()/6);

   qDebug() << "Total AO data" << m_occlusionData.size();
   qDebug() << "     positive" << nPos;
   qDebug() << "     negative" << nNeg;

   m_propertyDataPositive.clear();
   m_propertyDataNegative.clear();

   for (int i = 0; i < nPos; ++i) {
       m_propertyDataPositive << m_occlusionData[i];
   }
   for (int i = nPos; i < nPos+nNeg; ++i) {
       m_propertyDataNegative << m_occlusionData[i];
   }

   m_min = 0.0;
   m_max = 1.0;
   recompile();

   m_aoEngine->deleteLater();
   m_aoEngine = 0;
}


/*
Surface::Facet::Facet(Data::const_iterator& iter)
{
   GLfloat x, y, z;

   x = *iter;  y = *(iter++);  z = *(iter++);
   m_a.setValue(x,y,z);
   x = *(iter++);  y = *(iter++);  z = *(iter++);
   m_na.setValue(x,y,z);
}


// Implements a clipping plane defined by a normal vector and point p on the plane.
Data Surface::Facet::clip(Vec const& normal, Vec const& point)
{
   Data newData;
   Vec n(normal.unit());

   double D = - n.p;


    //  Determine the equation of the plane as
    //  Ax + By + Cz + D = 0
   l = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
   double A(n.x/l);
   B = n.y / l;
   C = n.z / l;
   D = -(n.x*p0.x + n.y*p0.y + n.z*p0.z);

   // Evaluate the equation of the plane for each vertex
   // If side < 0 then it is on the side to be retained
   // else it is to be clipped
   side[0] = A*p[0].x + B*p[0].y + C*p[0].z + D;
   side[1] = A*p[1].x + B*p[1].y + C*p[1].z + D;
   side[2] = A*p[2].x + B*p[2].y + C*p[2].z + D;

   Vec p1, p2, p3;
   Vec n1, n2, n3;

   Data::const_iterator iter;
   for (int i = 0; i < data.size(); i += 18) {
       t1.setValue(data[i   ], data[i+1], data[i+2]);
       n1.setValue(data[i+ 3], data[i+4], data[i+5]);
       t2.setValue(data[i+ 6], data[i+1], data[i+2]);
       n2.setValue(data[i+ 9], data[i+1], data[i+2]);
       t3.setValue(data[i+12], data[i+1], data[i+2]);
       n3.setValue(data[i+15], data[i+1], data[i+2]);

       data[i];
       newData += clipTriangle();
   }

   return newData;
}

*/


/*
   Clip a 3 vertex facet in place
   The 3 point facet is defined by vertices p[0],p[1],p[2], "p[3]"
      There must be a fourth point as a 4 point facet may result
   The normal to the plane is n
   A point on the plane is p0
   The side of the plane containing the normal is clipped away
   Return the number of vertices in the clipped polygon
*/

/*
Data Surface::clipTriangle(Vec* p, Vec const& n, Vec const& p0)
{
   double side[3];
   Vec q;

   // Determine the equation of the plane as Ax + By + Cz + D = 0
   double l(sqrt(n.x*n.x + n.y*n.y + n.z*n.z));
   double A(n.x/l);
   double B(n.y/l);
   double C(n.z/l);
   double D(-(n.x*p0.x + n.y*p0.y + n.z*p0.z));

   // Evaluate the equation of the plane for each vertex
   // If side < 0 then it is on the side to be retained
   // else it is to be clipped

   side[0] = A*p[0].x + B*p[0].y + C*p[0].z + D;
   side[1] = A*p[1].x + B*p[1].y + C*p[1].z + D;
   side[2] = A*p[2].x + B*p[2].y + C*p[2].z + D;

   // Are all the vertices are on the clipped side
   if (side[0] >= 0 && side[1] >= 0 && side[2] >= 0) {
      return(0);
   }

   // Are all the vertices on the not-clipped side
   if (side[0] <= 0 && side[1] <= 0 && side[2] <= 0)
      return(3);

   // Is p0 the only point on the clipped side
   if (side[0] > 0 && side[1] < 0 && side[2] < 0) {
      //q = p[0] - side[0]*(p[2]-p[0]) / (side[2]-side[1]);
      q.x = p[0].x - side[0] * (p[2].x - p[0].x) / (side[2] - side[0]);
      q.y = p[0].y - side[0] * (p[2].y - p[0].y) / (side[2] - side[0]);
      q.z = p[0].z - side[0] * (p[2].z - p[0].z) / (side[2] - side[0]);
      p[3] = q;
      q.x = p[0].x - side[0] * (p[1].x - p[0].x) / (side[1] - side[0]);
      q.y = p[0].y - side[0] * (p[1].y - p[0].y) / (side[1] - side[0]);
      q.z = p[0].z - side[0] * (p[1].z - p[0].z) / (side[1] - side[0]);
      p[0] = q;
      return(4);
   }

   // Is p1 the only point on the clipped side
   if (side[1] > 0 && side[0] < 0 && side[2] < 0) {
      p[3] = p[2];
      q.x = p[1].x - side[1] * (p[2].x - p[1].x) / (side[2] - side[1]);
      q.y = p[1].y - side[1] * (p[2].y - p[1].y) / (side[2] - side[1]);
      q.z = p[1].z - side[1] * (p[2].z - p[1].z) / (side[2] - side[1]);
      p[2] = q;
      q.x = p[1].x - side[1] * (p[0].x - p[1].x) / (side[0] - side[1]);
      q.y = p[1].y - side[1] * (p[0].y - p[1].y) / (side[0] - side[1]);
      q.z = p[1].z - side[1] * (p[0].z - p[1].z) / (side[0] - side[1]);
      p[1] = q;
      return(4);
   }

   // Is p2 the only point on the clipped side
   if (side[2] > 0 && side[0] < 0 && side[1] < 0) {
      q.x = p[2].x - side[2] * (p[0].x - p[2].x) / (side[0] - side[2]);
      q.y = p[2].y - side[2] * (p[0].y - p[2].y) / (side[0] - side[2]);
      q.z = p[2].z - side[2] * (p[0].z - p[2].z) / (side[0] - side[2]);
      p[3] = q;
      q.x = p[2].x - side[2] * (p[1].x - p[2].x) / (side[1] - side[2]);
      q.y = p[2].y - side[2] * (p[1].y - p[2].y) / (side[1] - side[2]);
      q.z = p[2].z - side[2] * (p[1].z - p[2].z) / (side[1] - side[2]);
      p[2] = q;
      return(4);
   }

   // Is p0 the only point on the not-clipped side
   if (side[0] < 0 && side[1] > 0 && side[2] > 0) {
      q.x = p[0].x - side[0] * (p[1].x - p[0].x) / (side[1] - side[0]);
      q.y = p[0].y - side[0] * (p[1].y - p[0].y) / (side[1] - side[0]);
      q.z = p[0].z - side[0] * (p[1].z - p[0].z) / (side[1] - side[0]);
      p[1] = q;
      q.x = p[0].x - side[0] * (p[2].x - p[0].x) / (side[2] - side[0]);
      q.y = p[0].y - side[0] * (p[2].y - p[0].y) / (side[2] - side[0]);
      q.z = p[0].z - side[0] * (p[2].z - p[0].z) / (side[2] - side[0]);
      p[2] = q;
      return(3);
   }

   // Is p1 the only point on the not-clipped side
   if (side[1] < 0 && side[0] > 0 && side[2] > 0) {
      q.x = p[1].x - side[1] * (p[0].x - p[1].x) / (side[0] - side[1]);
      q.y = p[1].y - side[1] * (p[0].y - p[1].y) / (side[0] - side[1]);
      q.z = p[1].z - side[1] * (p[0].z - p[1].z) / (side[0] - side[1]);
      p[0] = q;
      q.x = p[1].x - side[1] * (p[2].x - p[1].x) / (side[2] - side[1]);
      q.y = p[1].y - side[1] * (p[2].y - p[1].y) / (side[2] - side[1]);
      q.z = p[1].z - side[1] * (p[2].z - p[1].z) / (side[2] - side[1]);
      p[2] = q;
      return(3);
   }

   // Is p2 the only point on the not-clipped side 
   if (side[2] < 0 && side[0] > 0 && side[1] > 0) {
      q.x = p[2].x - side[2] * (p[1].x - p[2].x) / (side[1] - side[2]);
      q.y = p[2].y - side[2] * (p[1].y - p[2].y) / (side[1] - side[2]);
      q.z = p[2].z - side[2] * (p[1].z - p[2].z) / (side[1] - side[2]);
      p[1] = q;
      q.x = p[2].x - side[2] * (p[0].x - p[2].x) / (side[0] - side[2]);
      q.y = p[2].y - side[2] * (p[0].y - p[2].y) / (side[0] - side[2]);
      q.z = p[2].z - side[2] * (p[0].z - p[2].z) / (side[0] - side[2]);
      p[0] = q;
      return(3);
   }

   // Shouldn't get here
   return(-1);
}

*/

} } // end namespace IQmol::Layer
