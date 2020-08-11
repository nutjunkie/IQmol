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

#include "AxesLayer.h"
#include "OpenGL.h"
#include <QtOpenGL>
#include <QtDebug>


namespace IQmol {
namespace Layer {

Axes::Axes() 
 : Global("Axes"), m_xAxisOn(true), m_yAxisOn(true), m_zAxisOn(true), m_scale(1.0),
   m_configurator(*this)
{ 
   setConfigurator(&m_configurator);
   connect(newAction("Configure"), SIGNAL(triggered()), this, SLOT(configure()));
}
            
 

void Axes::draw()
{
   if (checkState() != Qt::Checked) return;

   GLfloat color[4]; 
   GLfloat blend(0.5f);  // So the colors don't look too garish

   GLfloat size(m_scale*m_sceneRadius);
   
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  

   // The colors of the axes are correct w.r.t. the world frame, and the labels
   // in the ui file, but they may look a little off here.
   if (m_xAxisOn) {
      color[0] = blend;  color[1] = blend;  color[2] = 1.0f;  color[3] = 1.0f;
      glColor4fv(color);
      drawArrow(size, 0.01f*size);
   }

   if (m_yAxisOn) {
      color[0] = 1.0f;  color[1] = blend;  color[2] = blend;  color[3] = 1.0f;
      glColor4fv(color);
      glPushMatrix();
      glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
      drawArrow(size, 0.01*size);
      glPopMatrix();
   }


   if (m_zAxisOn) {
      color[0] = blend;  color[1] = 1.0f;  color[2] = blend;  color[3] = 1.0f;
      glColor4fv(color);
      glPushMatrix();
      glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
      drawArrow(size, 0.01f*size);
      glPopMatrix();
   }
}


void Axes::drawArrow(double const length, double const radius, int const resolution)
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
