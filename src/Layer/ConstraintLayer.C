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

#include "ConstraintConfigurator.h"
#include "ConstraintLayer.h"
#include "Constraint.h"
#include "GLShape.h"

#include <QDebug>

using namespace qglviewer;

namespace IQmol {
namespace Layer {

GLfloat const Constraint::s_alpha = 0.3;
GLfloat const Constraint::s_tubeRadius = 0.03;
GLfloat const Constraint::s_tubeResolution = 0.02;
GLfloat const Constraint::s_scanColor[]        = { 0.6, 0.4, 1.0, s_alpha };
GLfloat const Constraint::s_satisfiedColor[]   = { 0.0, 0.8, 0.0, s_alpha };
GLfloat const Constraint::s_unsatisfiedColor[] = { 0.8, 0.0, 0.0, s_alpha };


Constraint::Constraint(AtomList const& atoms)
{ 
   init();
   setAtomList(atoms);
}


Constraint::Constraint(Data::PositionConstraint const& /* constraint*/)
{
   init();
   qDebug() << "Warning: constraint not correctly copied";
   //m_targetPosition = constraint.position();
}


Constraint::Constraint(Data::DistanceConstraint const& /* constraint*/)
{
   init();
   qDebug() << "Warning: constraint not correctly copied";
   //m_targetValue = constraint.value();
}


Constraint::Constraint(Data::AngleConstraint const& /* constraint*/)
{
   init();
   qDebug() << "Warning: constraint not correctly copied";
   //m_targetValue = constraint.value();
}


Constraint::Constraint(Data::TorsionConstraint const& /* constraint*/)
{
   init();
   qDebug() << "Warning: constraint not correctly copied";
   //m_targetValue = constraint.value();
}


void Constraint::init()
{
   connect(newAction("Configure"), SIGNAL(triggered()), this, SLOT(configure())); 
   connect(newAction("Delete"), SIGNAL(triggered()), this, SIGNAL(invalid())); 
   setFlags( Qt::ItemIsSelectable | 
             Qt::ItemIsEnabled | 
             Qt::ItemIsEditable);

   m_type               = Invalid;
   m_optimizeConstraint = false;
   m_scanConstraint     = false;
   m_points             = 2;
   m_configurator       = 0;
   m_axes               = 0;
}


void Constraint::setAtomList(AtomList const& atoms)
{ 
   m_atoms = atoms;
   if (m_configurator) delete m_configurator;
   m_configurator = 0;

   switch (m_atoms.size()) {
      case 1:
         m_type = Position; 
         m_configurator = new Configurator::VectorConstraint(*this);
         break;
      case 2:
         m_type = Distance; 
         m_configurator = new Configurator::ScalarConstraint(*this);
         break;
      case 3:
         m_type = Angle; 
         m_configurator = new Configurator::ScalarConstraint(*this);
         break;
      case 4:
         m_type = Torsion; 
         m_configurator = new Configurator::ScalarConstraint(*this);
         break;
      default:
         m_type = Invalid; 
         setText("Invalid");
         return;
         break;
   }

   if (m_configurator) setConfigurator(m_configurator);

   AtomList::iterator iter;
   for (iter = m_atoms.begin(); iter != m_atoms.end(); ++iter) {
       connect(*iter, SIGNAL(orphaned()), this, SIGNAL(invalid()));
   }
}


Constraint::~Constraint()
{
   delete m_configurator;
}


bool Constraint::sameAtoms(AtomList const& atoms) const
{
   if (m_atoms.size() != atoms.size()) return false;
   if (m_atoms == atoms) return true;

   // check for reverse atom ordering as well
   int  n(m_atoms.size());
   bool equal(true);
   for (int i = 0; i < n; ++i) {
        equal = equal && m_atoms[i] == atoms[n-i-1];
   }

   return equal;
}


Constraint& Constraint::operator=(Constraint const& that)
{
   if (this != &that) copy(that);
   return *this;
}


void Constraint::copy(Constraint const& that)
{
   setAtomList(that.m_atoms);  // sets m_type too
   m_mesg               = that.m_mesg;
   m_optimizeConstraint = that.m_optimizeConstraint;
   m_scanConstraint     = that.m_scanConstraint;
   m_targetValue        = that.m_targetValue;
   m_maxValue           = that.m_maxValue;
   m_points             = that.m_points;
   m_targetPosition     = that.m_targetPosition;
   m_axes               = that.m_axes;

   m_configurator->init();
}


void Constraint::setTargetValue(unsigned char const axes, Vec const& position)
{
   m_axes = axes;
   m_targetPosition = position;
   updated();
}


void Constraint::setTargetValue(double const value)
{
   m_targetValue = value;
   updated();
}


Vec Constraint::targetDisplacement() const
{
   Vec d;
   Vec p(m_atoms[0]->getPosition());

   if (m_axes & XAxis) d.x = m_targetPosition.x - p.x;
   if (m_axes & YAxis) d.y = m_targetPosition.y - p.y;
   if (m_axes & ZAxis) d.z = m_targetPosition.z - p.z;
   return d;
}


void Constraint::configure() 
{
   if (!m_configurator) return;
   m_configurator->sync();
   m_accepted = (m_configurator->exec() == QDialog::Accepted);
   if (m_accepted) updated();
}


bool Constraint::satisfied() const
{
   bool ok(true);

   switch (m_type) {
      case Invalid:   
         ok = false;
         break;

      case Position: { 
         Vec pos(m_atoms[0]->getPosition());
         if (m_axes & XAxis) ok = ok && std::abs(pos.x-m_targetPosition.x) <= 0.001;
         if (m_axes & YAxis) ok = ok && std::abs(pos.y-m_targetPosition.y) <= 0.001;
         if (m_axes & ZAxis) ok = ok && std::abs(pos.z-m_targetPosition.z) <= 0.001;
         } break;

      case Distance:{
         double val(Layer::Atom::distance(m_atoms[0], m_atoms[1]));
         ok = std::abs(val-m_targetValue) <= 0.001;  
         } break;

      case Angle: {
         double val(Layer::Atom::angle(m_atoms[0], m_atoms[1], m_atoms[2]));
         ok = std::abs(val-m_targetValue) <= 0.01;   
         } break;

      case Torsion: {
         double val(Layer::Atom::torsion(m_atoms[0], m_atoms[1], m_atoms[2], m_atoms[3]));
         ok = std::abs(val-m_targetValue) <= 0.1;    
         } break;
   }

   return ok;
}


void Constraint::addTo(OpenBabel::OBFFConstraints& constraints) const
{
   switch (m_type) {
      case Invalid:
         break;

      case Position: 
         constraints.AddAtomConstraint(m_atoms[0]->getIndex());
         break;

      case Distance:
         constraints.AddDistanceConstraint(m_atoms[0]->getIndex(), 
                                           m_atoms[1]->getIndex(), m_targetValue);
         break;

      case Angle:
         constraints.AddAngleConstraint(m_atoms[0]->getIndex(), 
                                        m_atoms[1]->getIndex(),
                                        m_atoms[2]->getIndex(), m_targetValue);
         break;

      case Torsion:
         constraints.AddTorsionConstraint(m_atoms[0]->getIndex(), 
                                          m_atoms[1]->getIndex(),
                                          m_atoms[2]->getIndex(),
                                          m_atoms[3]->getIndex(), m_targetValue);
          break;
   }

}


QString Constraint::formatQChem() const
{
   QString str;
   int precision(3);

   switch (m_type) {
      case Invalid:
         break;

      case Position: 
         str += QString::number(m_atoms[0]->getIndex()) + " XYZ";
         break;

      case Distance:
         precision = 3;
         str += "stre  " + QString::number(m_atoms[0]->getIndex()) + "  "
                         + QString::number(m_atoms[1]->getIndex()) + "  "
                         + QString::number(m_targetValue, 'f', precision);
         break;

      case Angle:
         precision = 2;
         str += "bend  " + QString::number(m_atoms[0]->getIndex()) + "  "
                         + QString::number(m_atoms[1]->getIndex()) + "  "
                         + QString::number(m_atoms[2]->getIndex()) + "  "
                         + QString::number(m_targetValue, 'f', precision);
         break;

      case Torsion:
         precision = 1;
         str += "tors  " + QString::number(m_atoms[0]->getIndex()) + "  "
                         + QString::number(m_atoms[1]->getIndex()) + "  "
                         + QString::number(m_atoms[2]->getIndex()) + "  "
                         + QString::number(m_atoms[3]->getIndex()) + "  "
                         + QString::number(m_targetValue, 'f', precision);
          break;
   }


   if (m_scanConstraint) {
      double delta( (m_maxValue - m_targetValue)/(m_points-1));
      str += "  " + QString::number(m_maxValue, 'f', precision)
           + "  " + QString::number(delta, 'f', precision);
   }

   return str + "\n";
}

// ------------------------------------------------------------------------------


void Constraint::draw()
{
   if (m_scanConstraint) {
      m_color = s_scanColor;
   }else if (satisfied()) {
      m_color = s_satisfiedColor;
   }else {
      m_color = s_unsatisfiedColor;
   }

   glColor3fv(m_color);

   switch (m_type) {
      case Invalid:                    break;
      case Position:  drawPosition();  break;
      case Distance:  drawDistance();  break;
      case Angle:     drawAngle();     break;
      case Torsion:   drawTorsion();   break;
   }
}


void Constraint::drawPosition()
{
   Frame frame(m_atoms[0]->getPosition(), Quaternion());
   glPushMatrix();
   glMultMatrixd(frame.matrix());

   bool selected(false);
   double radius(m_atoms[0]->getRadius(selected)+0.1);

   if (m_axes & ZAxis) {
      glRotatef(00.0f, 1.0f, 0.0f, 0.0f);
      GLShape::Torus(radius, s_tubeRadius, s_tubeResolution);
   }
   if (m_axes & YAxis) {
      glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
      GLShape::Torus(radius, s_tubeRadius, s_tubeResolution);
   }
   if (m_axes & ZAxis) {
      glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
      GLShape::Torus(radius, s_tubeRadius, s_tubeResolution);
   }

   bool fill(true);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_LIGHTING);

   if (m_axes & ZAxis) {
      glRotatef(00.0f, 1.0f, 0.0f, 0.0f);
      GLShape::Sector(radius, s_tubeResolution, fill);
   }
   if (m_axes & YAxis) {
      glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
      GLShape::Sector(radius, s_tubeResolution, fill);
   }
   if (m_axes & ZAxis) {
      glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
      GLShape::Sector(radius, s_tubeResolution, fill);
   }
 
   glDisable(GL_BLEND);
   glEnable(GL_LIGHTING);
   glPopMatrix();
}


void Constraint::drawDistance()
{
   Vec a(m_atoms[0]->getPosition());
   Vec b(m_atoms[1]->getPosition());
   Vec ab(b-a);

   GLShape::Tube(a, b, s_tubeRadius, s_tubeResolution);

   Quaternion orientation(Vec(0.0, 0.0, 1.0), ab);
   Frame frame(a, orientation);

   glPushMatrix();
   glMultMatrixd(frame.matrix());

   glTranslatef(0.0, 0.0, ab.norm()/2);
   double radius(0.2);
   GLShape::Torus(radius, s_tubeRadius, s_tubeResolution);

   glColor4fv(m_color);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_LIGHTING);

   bool fill(true);
   GLShape::Sector(radius, s_tubeResolution, fill);

   glDisable(GL_BLEND);
   glEnable(GL_LIGHTING);
   glPopMatrix();
}


void Constraint::drawAngle()
{
   Vec a(m_atoms[0]->getPosition());
   Vec b(m_atoms[1]->getPosition());
   Vec c(m_atoms[2]->getPosition());
   Vec ab((a-b).unit());
   Vec cb((c-b).unit());

   double radius(0.7);              // radius of sector
   double resolution(0.1f);         // arc-length of sector 
   double angle(std::acos(ab*cb));  // angle of sector

   GLShape::Tube(a, b, s_tubeRadius, s_tubeResolution);
   GLShape::Tube(c, b, s_tubeRadius, s_tubeResolution);

   // Align the frame x-axis to the ab bond
   Quaternion orientation(Vec(1.0,0.0,0.0), ab);
   Frame frame(b, orientation);
   // Now determine the frame coordinates of c and 
   // rotate it so that it aligns with the frame y-axis
   c = frame.coordinatesOf(c).unit();
   frame.rotate(Quaternion(Vec(1.0,0.0,0.0), atan2(c.z,c.y)));

   glPushMatrix();
   glMultMatrixd(frame.matrix());

   GLShape::Torus(radius, (double)s_tubeRadius, (double)s_tubeResolution, angle);

   glEnable(GL_BLEND);
   glDisable(GL_LIGHTING);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glColor4fv(m_color);
   bool fill(true);
   GLShape::Sector(radius, resolution, fill, angle);

   glEnable(GL_LIGHTING);
   glDisable(GL_BLEND);
   glPopMatrix();
}


void Constraint::drawTorsion()
{   
   Vec a(m_atoms[0]->getPosition());
   Vec b(m_atoms[1]->getPosition());
   Vec c(m_atoms[2]->getPosition());
   Vec d(m_atoms[3]->getPosition());
   Vec e(a+c-b);
   Vec f(d+b-c);

   GLShape::Tube(a, b, s_tubeRadius, s_tubeResolution);
   GLShape::Tube(c, b, s_tubeRadius, s_tubeResolution);
   GLShape::Tube(a, e, s_tubeRadius, s_tubeResolution);
   GLShape::Tube(c, e, s_tubeRadius, s_tubeResolution);

   GLShape::Tube(d, c, s_tubeRadius, s_tubeResolution);
   GLShape::Tube(b, c, s_tubeRadius, s_tubeResolution);
   GLShape::Tube(d, f, s_tubeRadius, s_tubeResolution);
   GLShape::Tube(b, f, s_tubeRadius, s_tubeResolution);

   GLShape::Sphere(e, s_tubeRadius, s_tubeResolution);
   GLShape::Sphere(f, s_tubeRadius, s_tubeResolution);

   Vec cb(c-b);

   // Align the frame z-axis to the cb bond
   Quaternion orientation(Vec(0.0,0.0,1.0), cb);
   Frame frame(b+0.5*cb, orientation);
   // Now determine the frame coordinates of a and 
   // rotate it so that it aligns with the frame y-axis
   Vec fa(frame.coordinatesOf(a).unit());

   frame.rotate(Quaternion(Vec(0.0,0.0,1.0), atan2(fa.y,fa.x)));

   glPushMatrix();
   glMultMatrixd(frame.matrix());
   double radius(0.5);
   double angle(Layer::Atom::torsion(m_atoms[0], m_atoms[1], m_atoms[2], m_atoms[3]));
   angle = angle * M_PI / 180.0;
   GLShape::Torus(radius, s_tubeRadius, s_tubeResolution, angle);

   glPopMatrix();

   glEnable(GL_BLEND);
   glDisable(GL_LIGHTING);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glColor4fv(m_color);

   glBegin(GL_QUAD_STRIP);
      glVertex3f(a.x, a.y, a.z);
      glVertex3f(e.x, e.y, e.z);
      glVertex3f(b.x, b.y, b.z);
      glVertex3f(c.x, c.y, c.z);
      glVertex3f(f.x, f.y, f.z);
      glVertex3f(d.x, d.y, d.z);
   glEnd();
/*
   glBegin(GL_TRIANGLE_STRIP);
      glVertex3f(a.x, a.y, a.z);
      glVertex3f(c.x, c.y, c.z);
      glVertex3f(b.x, b.y, b.z);
      glVertex3f(d.x, d.y, d.z);
   glEnd();
*/

   glEnable(GL_LIGHTING);
   glDisable(GL_BLEND);
}


} } // end namespace IQmol::Layer
