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

#include "SurfaceLayer.h"
#include "SurfaceType.h"
#include "MoleculeLayer.h"
#include "Preferences.h"
#include "QsLog.h"
#include "QGLViewer/vec.h"
#include "MeshDecimator.h"
#include "QMsgBox.h"
#include <QColorDialog>
#include <cmath>
#include <QFile>
#include <QTextStream>


using namespace qglviewer;

namespace IQmol {
namespace Layer {


Surface::Surface(Data::Surface& surface) : m_surface(surface), m_configurator(*this), 
   m_drawMode(Fill), m_callListPositive(0), m_callListNegative(0), m_drawVertexNormals(false),
   m_drawFaceNormals(false), m_balanceScale(false), m_decimator(0)
{
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled |
      Qt::ItemIsEditable);
               
   setCheckState(Qt::Unchecked);

   QList<QColor> const& colors(m_surface.colors());

   if (colors.isEmpty()) {
      setColors(Preferences::NegativeSurfaceColor(), 
                Preferences::PositiveSurfaceColor());
   }else if (colors.size() == 2) {
      setColors(colors.first(), colors.last());
   }

   setConfigurator(&m_configurator); 
   m_configurator.setWindowTitle("Configure Surface: " + m_surface.description());

   connect(newAction("Show Vertex Normals"), SIGNAL(triggered()), 
      this, SLOT(toggleVertexNormals()));
   connect(newAction("Show Face Normals"), SIGNAL(triggered()), 
      this, SLOT(toggleFaceNormals()));
   connect(newAction("Decimate"), SIGNAL(triggered()), 
      this, SLOT(decimate()));
   connect(newAction("Print Mesh Info"), SIGNAL(triggered()), 
      this, SLOT(dumpMeshInfo()));

   setText(m_surface.description());
   setData(m_surface.description(), Qt::ToolTipRole);

   setAlpha(m_surface.opacity()); 
   updated();
}


void Surface::setMolecule(Molecule* molecule) 
{
   m_molecule = molecule;
   connect(this, SIGNAL(updated()), molecule, SIGNAL(softUpdate()));

}


bool Surface::isVdW() const
{
   //hack
   Data::SurfaceType type(Data::SurfaceType::VanDerWaals);
   return text() == type.toString();
}


void Surface::setCheckStatus(Qt::CheckState const state)
{
   m_surface.setVisibility(state == Qt::Checked);
}


void Surface::getPropertyRange(double& min, double& max) const
{
   m_surface.getPropertyRange(min, max);
   if (m_balanceScale && min < 0.0 && max > 0.0) {
      min = std::min(min, -max);
      max = std::max(max, -min);
   }
}


Surface::~Surface()
{
   if (m_callListPositive) glDeleteLists(m_callListPositive, 1);
   if (m_callListNegative) glDeleteLists(m_callListNegative, 1);
}


void Surface::setAlpha(double alpha) 
{
   m_alpha = alpha;
   m_surface.setOpacity(m_alpha);
   m_colorPositive[3] = m_alpha;
   m_colorNegative[3] = m_alpha;
   recompile(); // this is really only needed if there is a property
}


QList<QColor> const& Surface::colors() const
{
   return m_surface.colors();
}


void Surface::setColors(QList<QColor> const& colors)
{
   m_surface.setColors(colors);
}


void Surface::setColors(QColor const& negative, QColor const& positive)
{
   m_colorNegative[0] = negative.redF();
   m_colorNegative[1] = negative.greenF();
   m_colorNegative[2] = negative.blueF();

   m_colorPositive[0] = positive.redF();
   m_colorPositive[1] = positive.greenF();
   m_colorPositive[2] = positive.blueF();

qDebug() << "Need to sync colors to Data::Surface";
return;
   // For some bizarre reason, calling the Data::Surface::setColors causes the
   // negative and positive colors (which are supposed to be const!) to
   // change.  Not only that, the call the setColors causes a crash when swapping
   // colors on a surface from an obj file.
   QList<QColor> colors;
   colors << negative << positive;
   //m_surface.setColors(colors);
}


QColor Surface::colorPositive() const
{
   QColor color;
   color.setRgbF(m_colorPositive[0], m_colorPositive[1],
                 m_colorPositive[2], m_colorPositive[3]);
   return color;
}


QColor Surface::colorNegative() const
{
   QColor color;
   color.setRgbF(m_colorNegative[0], m_colorNegative[1],
                 m_colorNegative[2], m_colorNegative[3]);
   return color;
}



void Surface::drawFast()
{
   draw();
}


void Surface::drawSelected()
{
   return;
}


void Surface::draw() 
{
   if ( (checkState() != Qt::Checked) || m_alpha < 0.01) return;

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

   glPopMatrix();
   if (!blend) glDisable(GL_BLEND);
   if (lighting) glEnable(GL_LIGHTING);
   glDisable(GL_CULL_FACE);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  

   if (m_drawVertexNormals) drawVertexNormals();
   if (m_drawFaceNormals) drawFaceNormals();
}


void Surface::balanceScale(bool const tf)
{
   m_balanceScale = tf;
   recompile();
   updated();
}


void Surface::toggleVertexNormals() 
{
   m_drawVertexNormals = !m_drawVertexNormals;
   updated();
}


void Surface::toggleFaceNormals() 
{
   m_drawFaceNormals = !m_drawFaceNormals;
//   if (m_drawFaceNormals && !m_surface.hasFaceCentroids()) {
//      if (!m_surface.computeFaceCentroids()) {
//         m_drawFaceNormals = false;
//         QLOG_WARN() << "Unable to compute face centroids";
//         return;
//      }
//   }
   updated();
}


void Surface::drawVertexNormals()
{
   glColor3f(0.000f, 0.000f, 1.000f);
   glDisable(GL_LIGHTING);
   glLineWidth(1.0);

   glBegin(GL_LINES);
      drawVertexNormals(m_surface.meshPositive());
      drawVertexNormals(m_surface.meshNegative());
   glEnd();
}


void Surface::drawVertexNormals(Data::Mesh const& mesh)
{
   Data::OMMesh::ConstVertexIter vertex;
   Data::OMMesh::Point  p;
   Data::OMMesh::Normal n;

   float scale(0.08);

   for (vertex = mesh.vbegin(); vertex != mesh.vend(); ++vertex) {
       p = mesh.vertex(vertex);
       n = mesh.normal(vertex);
       glVertex3fv( &p[0] );
       p += scale*n;
       glVertex3fv( &p[0]);
   }
}


void Surface::drawFaceNormals()
{
   glColor3f(1.000f, 0.000f, 0.000f);
   glDisable(GL_LIGHTING);
   glLineWidth(1.0);

   glBegin(GL_LINES);
      drawFaceNormals(m_surface.meshPositive());
      drawFaceNormals(m_surface.meshNegative());
   glEnd();
}


void Surface::drawFaceNormals(Data::Mesh const& mesh)
{
   Data::OMMesh::ConstFaceIter face;
   Data::OMMesh::Point  p;
   Data::OMMesh::Normal n;

   float scale(0.12);

   for (face = mesh.fbegin(); face != mesh.fend(); ++face) {
       p = mesh.faceCentroid(*face);
       n = mesh.normal(*face);
       glVertex3fv( &p[0] );
       p += scale*n;
       glVertex3fv( &p[0]);
   }
}
 

void Surface::recompile()
{
   if (m_decimator) {
      //QMsgBox::information(0, "IQmol", "Decimation in progress - Unable to modify surface");
      return;
   }
   if (m_callListPositive) glDeleteLists(m_callListPositive, 1);
   if (m_callListNegative) glDeleteLists(m_callListNegative, 1);
   m_callListPositive = compile(m_surface.meshPositive());
   m_callListNegative = compile(m_surface.meshNegative());
}


GLuint Surface::compile(Data::Mesh const& mesh)
{
   Data::OMMesh const& data(mesh.data());
   Data::OMMesh::ConstFaceIter face;
   Data::OMMesh::ConstFaceVertexIter vertex;
   bool hasColor(mesh.hasProperty(Data::Mesh::ScalarField));

   GLint setID = glGenLists(1);

   glNewList(setID, GL_COMPILE);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, data.points());
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GL_FLOAT, 0, data.vertex_normals());

      if (hasColor) {

         double min, max, property;
         getPropertyRange(min, max);
         ColorGradient::Function gradient(m_surface.colors(), min, max);
         QColor color;

         glBegin(GL_TRIANGLES);
            for (face = data.faces_begin(); face != data.faces_end(); ++face) {
                vertex   = data.cfv_iter(*face);
                property = mesh.scalarFieldValue(vertex);
                color    = gradient.colorAt(property);
                glColor4f( color.redF(), color.greenF(), color.blueF(), m_alpha );
                glArrayElement(vertex->idx());

                ++vertex;
                property = mesh.scalarFieldValue(vertex);
                color    = gradient.colorAt(property);
                glColor4f( color.redF(), color.greenF(), color.blueF(), m_alpha );
                glArrayElement( vertex->idx());

                ++vertex;
                property = mesh.scalarFieldValue(vertex);
                color    = gradient.colorAt(property);
                glColor4f( color.redF(), color.greenF(), color.blueF(), m_alpha );
                glArrayElement( vertex->idx());
            }
         glEnd();

      }else {

         glBegin(GL_TRIANGLES);
            for (face = data.faces_begin(); face != data.faces_end(); ++face) {
                vertex = data.cfv_iter(*face);
                glArrayElement(vertex->idx());
                ++vertex;
                glArrayElement(vertex->idx());
                ++vertex;
                glArrayElement(vertex->idx());
            }
         glEnd();

      }

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);

   glEndList();

   return setID;
}


void Surface::clearPropertyData()
{
   m_surface.clearSurfaceProperty();
   recompile();
}


void Surface::computePropertyData(Function3D const& function) 
{
   m_surface.computeSurfaceProperty(function);
   recompile(); 
}


void Surface::computeIndexField() 
{
   m_surface.computeIndexProperty();
   recompile(); 
}


void Surface::decimate()
{
   if (m_decimator) {
      QMsgBox::information(0, "IQmol", "Mesh simplification in progress");
      return;
   }   

   m_decimator = new MeshDecimatorTask(m_surface.meshPositive().data(),
      m_surface.meshNegative().data());

   connect(m_decimator, SIGNAL(finished()), this, SLOT(decimateFinished()));
   QLOG_INFO() << "Commencing mesh decimation";
   m_decimator->start();
}


void Surface::decimateFinished()
{
   if (!m_decimator) return;

   double time(m_decimator->timeTaken());
   QLOG_INFO() << "Mesh decimation time" << time;

   delete m_decimator;
   m_decimator = 0;
   recompile(); 
   updated();
}


void Surface::dumpMeshInfo() const
{
   m_surface.dump();
}

} } // end namespace IQmol::Layer
