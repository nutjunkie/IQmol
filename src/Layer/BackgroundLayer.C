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

#include "BackgroundLayer.h"
#include "QGLViewer/qglviewer.h"
#include "Preferences.h"


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
      glClear(GL_COLOR_BUFFER_BIT);
   } else {
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f); 
        glClear(GL_COLOR_BUFFER_BIT);
   }
}


void Background::drawGradient()
{
//   const float eps = 1.0e-6; 

/*
   glEnable(GL_LIGHTING);
   glDisable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH); 

   glMatrixMode(GL_MODELVIEW); 
   glPushMatrix();
   glLoadIdentity(); 
   glMatrixMode(GL_PROJECTION); 
   glPushMatrix();
   glLoadIdentity(); 
   glOrtho(-1.0, 1.0, -1.0, 1.0, 1.0, -1.0); 

   glBegin(GL_QUADS); 
      //glColor4f(0.125, 0.361, 0.659, 1.0);
      glColor4f(0.0, 0.0, 0.0, 1.0);
      glVertex3f(-1.0f, -1.0f,  1.0f); 
      glColor4f(0.0, 0.0, 0.0, 1.0);
      glVertex3f( 1.0f, -1.0f,  1.0f); 

      glColor4f(1.0, 1.0, 1.0, 1.0);
      //glColor4f(0.15, 0.433, 0.792, 1.0);
      glVertex3f( 1.0f,  1.0f,  1.0f); 
      glColor4f(1.0, 1.0, 1.0, 1.0);
      glVertex3f(-1.0f,  1.0f,  1.0f); 
   glEnd(); 

   //glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glEnable(GL_DEPTH_TEST);
   //glEnable(GL_LIGHTING);
*/
}

} } // end namespace IQmol::Layer
