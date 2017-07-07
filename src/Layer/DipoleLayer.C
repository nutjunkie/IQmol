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

#include "DipoleLayer.h"
#include "MoleculeLayer.h"
#include "Preferences.h"
#include <cmath>

#include <QDebug>

using namespace qglviewer;

namespace IQmol {
namespace Layer {


Dipole::Dipole(qglviewer::Vec const& dipoleMoment) : GLObject("Dipole"), 
   m_color(Qt::cyan), m_scale(0.25), m_configurator(*this), m_dipoleMoment(dipoleMoment), 
   m_sceneRadius(Preferences::DefaultSceneRadius())
{
   setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
   setCheckState(Qt::Unchecked);
   setConfigurator(&m_configurator);
}


void Dipole::setMolecule(Molecule* molecule)
{
  m_molecule = molecule;

  if (m_molecule) {
     connect(m_molecule, SIGNAL(centerOfNuclearChargeAvailable(qglviewer::Vec const&)), 
        this, SLOT(setPosition(qglviewer::Vec const&)));

     connect(m_molecule, SIGNAL(radiusAvailable(double const&)), 
        this, SLOT(setRadius(double const&)));

     connect(this, SIGNAL(updated()), m_molecule, SIGNAL(softUpdate()));
        

     setPosition(m_molecule->centerOfNuclearCharge());
     setRadius(m_molecule->radius());
  }
}


void Dipole::setRadius(double const radius) 
{ 
   m_sceneRadius = std::max(radius, Preferences::DefaultSceneRadius());
}


double Dipole::value() const
{
   return m_dipoleMoment.norm();
}


void Dipole::setValue(qglviewer::Vec const& dipole)
{
   m_dipoleMoment = dipole;
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
   Vec dipole(m_dipoleMoment);
   double value(dipole.normalize());

   if ( (checkState() != Qt::Checked) || value < 0.01) return;

   glPushMatrix();
   glMultMatrixd(m_frame.matrix());
   glColor3f(m_color.redF(), m_color.greenF(), m_color.blueF());
   drawArrow(dipole*m_scale*m_sceneRadius, -dipole*m_scale*m_sceneRadius);
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
      // Zero out very small vectors to tidy things up.
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
