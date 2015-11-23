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

#include "BackgroundLayer.h"
#include "QGLViewer/qglviewer.h"
#include "Preferences.h"

#include <QDebug>

namespace IQmol {
namespace Layer {

Background::Background() : m_backgroundColor(Preferences::BackgroundColor()), 
   m_foregroundColor(Preferences::ForegroundColor()), m_configurator(*this) 
{ 
   setText("Background");
   setCheckState(Qt::Checked);
   setConfigurator(&m_configurator);
   connect(newAction("Edit Color"), SIGNAL(triggered()), this, SLOT(configure()));
}


void Background::draw()
{
   if (checkState() == Qt::Checked) {
      glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), 
         m_backgroundColor.blueF(), m_backgroundColor.alphaF());
   } else {
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f); 
   }

   glClear(GL_COLOR_BUFFER_BIT);

   if (false) {
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
      glClear(GL_COLOR_BUFFER_BIT);
      drawGradient();
   }
}


void Background::drawCircleGradient()
{
   float corner[4];
   corner[0] = m_backgroundColor.redF();
   corner[1] = m_backgroundColor.greenF();
   corner[2] = m_backgroundColor.blueF();
   corner[3] = 1.0;
   float center[4];
   center[0] = 1.0;
   center[1] = 1.0;
   center[2] = 1.0;
   center[3] = 1.0;

   int n(20);

   double Pi2(2.0*3.141592);

   glBegin(GL_TRIANGLE_FAN);
      glColor4fv(center);
      glVertex2f(0.0, 0.0); // center of circle
      glColor4fv(corner);
      for (int i = 0; i <= n; i++) { 
          glVertex2f(1.43*cos(i*Pi2/n), 1.43*sin(i*Pi2/n));
      }
	glEnd();
}


void Background::drawGradient()
{
qDebug() << "Drawing gradient";
   //glDisable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH); 

   glMatrixMode(GL_MODELVIEW); 
   glPushMatrix();
   glLoadIdentity(); 
   glMatrixMode(GL_PROJECTION); 
   glPushMatrix();
   glLoadIdentity(); 
   glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, -1.0); 

   float center[4];
   center[0] = m_backgroundColor.redF();
   center[1] = m_backgroundColor.greenF();
   center[2] = m_backgroundColor.blueF();
   center[3] = 1.0;
   float corner[4];
   corner[0] = 0.0;
   corner[1] = 0.0;
   corner[2] = 0.0;
   corner[3] = 1.0;


/*
   glBegin(GL_QUADS); 
      glColor4fv(corner);
      glVertex3f(-1.0f,  0.0f,  1.0f); 
      glVertex3f(-1.0f, -1.0f,  1.0f); 
      glVertex3f( 0.0f, -1.0f,  1.0f); 
      glColor4fv(center);
      glVertex3f( 0.0f,  0.0f,  1.0f); 

      glColor4fv(corner);
      glVertex3f( 0.0f,  1.0f,  1.0f); 
      glVertex3f(-1.0f,  1.0f,  1.0f); 
      glVertex3f(-1.0f,  0.0f,  1.0f); 
      glColor4fv(center);
      glVertex3f( 0.0f,  0.0f,  1.0f); 

      glColor4fv(corner);
      glVertex3f( 1.0f,  0.0f,  1.0f); 
      glVertex3f( 1.0f,  1.0f,  1.0f); 
      glVertex3f( 0.0f,  1.0f,  1.0f); 
      glColor4fv(center);
      glVertex3f( 0.0f,  0.0f,  1.0f); 

      glColor4fv(corner);
      glVertex3f( 0.0f, -1.0f,  1.0f); 
      glVertex3f( 1.0f, -1.0f,  1.0f); 
      glVertex3f( 1.0f,  0.0f,  1.0f); 
      glColor4fv(center);
      glVertex3f( 0.0f,  0.0f,  1.0f); 
   glEnd(); 
*/

   drawCircleGradient();

   //glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
}

} } // end namespace IQmol::Layer
