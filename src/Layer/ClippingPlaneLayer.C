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

#include "ClippingPlaneLayer.h"
#include "OpenGL.h"
#include "Preferences.h"


namespace IQmol {
namespace Layer {

ClippingPlane::ClippingPlane() : GLObject ("Clipping Plane"), 
   m_sceneRadius(Preferences::DefaultSceneRadius()), m_configurator(0)
{
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
   setCheckState(Qt::Unchecked);
   setAlpha(0.5f);
   m_configurator = new Configurator::ClippingPlane(*this);
   setConfigurator(m_configurator);
}


ClippingPlane::~ClippingPlane()
{
   if (m_configurator) delete m_configurator;
}


void ClippingPlane::setEquation()
{
   // Since the Clipping Plane equation is multiplied by the current modelView,
   // we can define a constant equation (plane normal along Z and passing by
   // the origin) since we are here in the manipulatedFrame coordinates system
   // (we glMultMatrixd() with the manipulatedFrame matrix() ).

   static const GLdouble ClipEquation[] = { 0.0, 0.0, 1.0, 0.0 };

   glPushMatrix();
   glMultMatrixd(m_frame.matrix());
   glClipPlane(GL_CLIP_PLANE0, ClipEquation); 
   glPopMatrix();
}


qglviewer::Vec ClippingPlane::normal() const
{
   qglviewer::Quaternion orientation(getFrame().orientation());
   return orientation*qglviewer::Vec(0.0, 0.0, 1.0);
}


double ClippingPlane::distance() const
{
   qglviewer::Vec n(normal());
   qglviewer::Vec p(getFrame().position());
   
   return n*p;
}


void ClippingPlane::drawSelected()
{
   glColor4f(0.5f, 0.0f, 0.0f, m_alpha);
   drawPrivate();
}


void ClippingPlane::draw()
{
   glColor4f(0.8f, 0.8f, 0.8f, m_alpha);
   drawPrivate();
}


void ClippingPlane::drawPrivate()
{
   if (checkState() != Qt::Checked) return;

   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   // Draw a plane representation: Its normal...
   drawArrow(0.5f*m_sceneRadius, 0.02f*m_sceneRadius);
   // ...and a quad (with a slightly shifted z so that it is not clipped).
   glEnable(GL_BLEND);

   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glBegin(GL_QUADS);
   glVertex3f(-m_sceneRadius, -m_sceneRadius, 0.001f);
   glVertex3f(-m_sceneRadius,  m_sceneRadius, 0.001f);
   glVertex3f( m_sceneRadius,  m_sceneRadius, 0.001f);
   glVertex3f( m_sceneRadius, -m_sceneRadius, 0.001f);
   glEnd();

   glDisable(GL_BLEND);
   glPopMatrix();
}



void ClippingPlane::drawArrow(double const length, double const radius, int const resolution)
{
   GLUquadric* quadric = gluNewQuadric();
   const  GLfloat r = (radius < 0.0f) ?  0.05f * length : radius;
   const  GLfloat head = 2.5f*(r / length) + 0.1f;
   const  GLfloat coneRadiusCoef = 4.0f - 5.0f * head;

   gluQuadricOrientation(quadric, GLU_OUTSIDE);
   gluCylinder(quadric, r, r, length * (1.0f - head/coneRadiusCoef), resolution, 1); 
   glTranslatef(0.0f, 0.0f, length * (1.0f - head));
   gluCylinder(quadric, coneRadiusCoef * radius, 0.0f, head * length, resolution, 1); 
   gluQuadricOrientation(quadric, GLU_INSIDE);
   gluDisk(quadric, 0.0f, coneRadiusCoef * radius, resolution, 1); 
   glTranslatef(0.0f, 0.0f, -length * (1.0f - head));
   gluDeleteQuadric(quadric);
}

} } // end namespace IQmol::Layer
