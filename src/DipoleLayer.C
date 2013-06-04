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

#include "DipoleLayer.h"
#include "IQmol.h"
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace Layer {


Dipole::Dipole(Vec const& dipole, Vec const& center) : GLObject("Dipole"), m_color(Qt::cyan),
   m_scale(0.25), m_configurator(this), m_radius(DefaultSceneRadius)
{
   setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
   setCheckState(Qt::Unchecked);
   setPosition(center);
   setValue(dipole);
   setConfigurator(&m_configurator);
}
            

void Dipole::setRadius(double const radius) 
{ 
   m_radius = std::max(radius, DefaultSceneRadius);
}


void Dipole::setValue(qglviewer::Vec const& value)
{
   m_value = value.norm();
   m_direction = value.unit();
   m_configurator.sync();
}


void Dipole::draw()
{
   drawPrivate();
}


void Dipole::drawFast()
{
   drawPrivate();
}


void Dipole::drawPrivate() 
{
   if ( (checkState() != Qt::Checked) || m_value < 0.01) return;
   glPushMatrix();
   glMultMatrixd(m_frame.matrix());
   glColor3f(m_color.redF(), m_color.greenF(), m_color.blueF());
   drawArrow(m_direction*m_scale*m_radius, -m_direction*m_scale*m_radius);
   glPopMatrix();
}


void Dipole::drawArrow(Vec const& from, Vec const& to, int const resolution)
{
   glPushMatrix();
   glTranslatef(from[0], from[1], from[2]);
   Vec dir(to-from);
   glMultMatrixd(Quaternion(Vec(0,0,1), dir).matrix());
   drawArrow(dir.norm(), resolution);
   glPopMatrix();
}


void Dipole::drawArrow(float length, int const resolution)
{
   GLUquadric* quadric = gluNewQuadric();
   const GLfloat radius(0.04f*length);

   GLfloat coneRadius(3.20f*radius);
   GLfloat headLength(1.80f*coneRadius);
   GLfloat tubeLength(length-headLength);

   if (2.0*length < headLength) {
      // Zero out very small vectors to tydy things up.
      return;

   }else if (length < headLength) {
      coneRadius *= length/headLength;
      headLength  = length;
      tubeLength  = 0.0f;
   }else {
      // Only Vectors longer than headLength have a tube.
      gluQuadricOrientation(quadric, GLU_OUTSIDE);
      gluCylinder(quadric, radius, radius, tubeLength, resolution, 1);
      gluQuadricOrientation(quadric, GLU_INSIDE);
      gluDisk(quadric, 0.0, radius, resolution, 1);

      glTranslatef(0.0, 0.0, 0.5*headLength);
      gluDisk(quadric, radius, coneRadius, resolution, 1);
      gluQuadricOrientation(quadric, GLU_OUTSIDE);
      gluCylinder(quadric, coneRadius, coneRadius, radius, resolution, 1);
      glTranslatef(0.0, 0.0, radius);
      gluDisk(quadric, radius, coneRadius, resolution, 1);
      glTranslatef(0.0, 0.0, -0.5*headLength-radius);
   }

   glTranslatef(0.0, 0.0, tubeLength);
   gluQuadricOrientation(quadric, GLU_OUTSIDE);
   gluCylinder(quadric, coneRadius, 0.0, headLength, resolution, 1);
   gluQuadricOrientation(quadric, GLU_INSIDE);
   gluDisk(quadric, 0.0, coneRadius, resolution, 1);

   gluDeleteQuadric(quadric);
}


} } // end namespace IQmol::Layer
