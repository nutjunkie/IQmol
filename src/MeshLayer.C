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

#include "MeshLayer.h"
#include "GLShape.h"
#include <QtOpenGL>
#include <QtDebug>
#include <cmath>


namespace IQmol {
namespace Layer {


Mesh::Mesh() : Global("Mesh"), m_configurator(this)
{
   init();
   setConfigurator(&m_configurator);
   connect(newAction("Configure"), SIGNAL(triggered()), this, SLOT(configure()));
}


void Mesh::init()
{
   m_xy = true;  m_xz = false;  m_yz = false;
   m_polar       = false;
   m_useStepSize = false; 
   m_ticks       = 10;
   m_stepSize    = 1.0;
   m_lineWidth   = 1.5; 
}


void Mesh::draw()
{
   if (checkState() != Qt::Checked) return;

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_LINE_SMOOTH);
   glDisable(GL_LIGHTING);

   glLineWidth(m_lineWidth);
   glColor3f(0.7, 0.7, 0.7);
   double step;
   int nSteps;

   if (m_useStepSize) {
      step   = m_stepSize;
      nSteps = (int)(m_sceneRadius/m_stepSize);
   }else {
      step   = m_sceneRadius/m_ticks;
      nSteps = m_ticks;
   }

   if (m_polar) {
      drawPolar(step, nSteps);
   }else if (m_useStepSize) {
      drawCartesian(0.5f*step, 2*nSteps);
   }else {
      drawCartesian(step, nSteps);
   }

   glDisable(GL_BLEND);
   glDisable(GL_LINE_SMOOTH);
   glEnable(GL_LIGHTING);
}


void Mesh::drawCartesian(double const stepSize, int const nSteps)
{
   glBegin(GL_LINES);
   double width(stepSize*nSteps);
   if (m_xy) {
      for (int i = 0; i <= nSteps; ++i) {
          double pos = width*(2.0f*i/nSteps-1.0f);
          glVertex3f(pos, -width, 0.0f);
          glVertex3f(pos, +width, 0.0f);
          glVertex3f(-width, pos, 0.0f);
          glVertex3f(+width, pos, 0.0f);
       }
   }
   if (m_xz) {
      for (int i = 0; i <= nSteps; ++i) {
          double pos = width*(2.0f*i/nSteps-1.0f);
          glVertex3f(pos, 0.0f, -width);
          glVertex3f(pos, 0.0f, +width);
          glVertex3f(-width, 0.0f, pos);
          glVertex3f(+width, 0.0f, pos);
       }
   }
   if (m_yz) {
      for (int i = 0; i <= nSteps; ++i) {
          double pos = width*(2.0f*i/nSteps-1.0f);
          glVertex3f(0.0f, pos, -width);
          glVertex3f(0.0f, pos, +width);
          glVertex3f(0.0f, -width, pos);
          glVertex3f(0.0f, +width, pos);
       }
   }
   glEnd();
}


void Mesh::drawPolar(double const stepSize, int const nSteps)
{
   glPushMatrix();
   glBegin(GL_LINES);
   double resolution(0.1);
   // Umm, if I don't do this the first grid is always in the XY plane.
   GLShape::Sector(0.0, resolution);

   if (m_xy) {
      glRotatef(00.0f, 1.0f, 0.0f, 0.0f);
      for (int i = 0; i <= nSteps; ++i) {
          GLShape::Sector(i*stepSize, resolution);
      }
      drawRadialLines(stepSize, nSteps*stepSize);
   }
   if (m_xz) {
      glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
      for (int i = 0; i <= nSteps; ++i) {
          GLShape::Sector(i*stepSize, resolution);
      }
      drawRadialLines(stepSize, nSteps*stepSize);
   }
   if (m_yz) {
      glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
      for (int i = 0; i <= nSteps; ++i) {
          GLShape::Sector(i*stepSize, resolution);
      }
      drawRadialLines(stepSize, nSteps*stepSize);
   }
   glEnd();
   glPopMatrix();
} 


void Mesh::drawRadialLines(double const inner, double outer) 
{ 
   int   n(8);
   double theta(2.0f*M_PI / n);
   double s, c;
    
   glBegin(GL_LINES); 
      for (int i = 0; i < n; ++i) {
          s = std::sin(i*theta);
          c = std::cos(i*theta);
          glVertex2f(inner*c, inner*s);
          glVertex2f(outer*c, outer*s);
      }
   glEnd(); 
}

} } // end namespace IQmol::Layer
