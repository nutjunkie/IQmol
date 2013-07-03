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

#include "BondLayer.h"
#include "AtomLayer.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {

// Static Data
GLfloat Bond::s_defaultColor[]       = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat Bond::s_radiusBallsAndSticks =  0.10;
GLfloat Bond::s_radiusTubes          =  0.10;
GLfloat Bond::s_radiusWireFrame      =  3.00;  // pixels
Vec     Bond::s_zAxis                = Vec(0.0, 0.0, 1.0);


Bond::Bond(Atom* begin, Atom* end) : Primitive("Bond"),
   m_begin(begin), m_end(end), m_order(1) { }


void Bond::setIndex(int const index)
{
   m_index = index;
   setText(QString::number(index).rightJustified(2) + "   " + 
      m_begin->symbol() + QString::number(m_begin->m_index) + " - " +
      m_end->symbol()   + QString::number(m_end->m_index) ) ;
}


double Bond::length()
{
   Vec a(m_begin->displacedPosition());
   Vec b(m_end  ->displacedPosition());
   return (a-b).norm();
}


void Bond::draw() 
{
   bool selectedOnly(false);
   drawPrivate(selectedOnly); 
}


void Bond::drawSelected() 
{
   bool selectedOnly(true);
   drawPrivate(selectedOnly);
}


void Bond::drawPrivate(bool selected) 
{
   updateOrientation(); // This should only need calling if an atom is moving
   switch (m_drawMode) {
      case Primitive::BallsAndSticks:
         drawBallsAndSticks(selected);
         break;
      case Primitive::Tubes:
         drawTubes(selected);
         break;
      case Primitive::WireFrame:
         drawWireFrame(selected);
         break;
      default:
         break;
   }
}


void Bond::drawBallsAndSticks(bool selected)
{
   GLfloat radius(s_radiusBallsAndSticks*m_scale);

   if (selected) {
      radius += Primitive::s_selectOffset;
      glColor4fv(Primitive::s_selectColor);
   }else {
      glColor4fv(s_defaultColor);
   }

   Vec a(m_begin->displacedPosition());
   Vec b(m_end  ->displacedPosition());
   Vec normal = cross(s_cameraPosition-a, s_cameraPosition-b);
   normal.normalize();
   GLfloat length((a-b).norm());

   Frame frame(m_frame);
   GLUquadric* quad = gluNewQuadric();

   switch (m_order) {
      case 1: {
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();
      } break;
 
      case 2: {
         normal *= 0.08;  // Governs the offset for the bond lines
         radius *= 0.7;   // Make the bonds a bit thinner

         frame.translate(-normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         frame.translate(2.0*normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();
      } break;

      case 3: {
         normal *= 0.11;  // Governs the offset for the bond lines
         radius *= 0.45;  // Make the bonds a bit thinner

         frame.translate(-normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         frame.translate(normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         frame.translate(normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();
      } break;

      case 4: {
         normal *= 0.11;  // Governs the offset for the bond lines
         radius *= 0.40;  // Make the bonds a bit thinner

         frame.translate(-1.5*normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         frame.translate(normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         frame.translate(normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         frame.translate(normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();
      } break;

      case 5: {  // Aromatic
         normal *= 0.08;  // Governs the offset for the bond lines

         frame.translate(-normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         radius *= 0.5;   // Make the second bond a bit thinner
         frame.translate(2.0*normal);
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();
      } break;

      default: {
         radius *= 2;         // Fat bond indicates we don't know what we are doing
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
         glPopMatrix();
      } break;
      
   }

   gluDeleteQuadric(quad); 
}


void Bond::drawTubes(bool selected) 
{
   GLfloat radius(s_radiusTubes*m_scale);

   Vec a(m_begin->displacedPosition());
   Vec b(m_end  ->displacedPosition());
   GLfloat length((a-b).norm());

   glPushMatrix();
   glMultMatrixd(m_frame.matrix());
   GLUquadric* quad = gluNewQuadric();

   if (selected) {
      radius += Primitive::s_selectOffset;
      glColor4fv(Primitive::s_selectColor);
      gluCylinder(quad, radius, radius, length, Primitive::s_resolution, 1);
   }else {
      glColor4fv(m_end->m_color);
      gluCylinder(quad, 0.99*radius, 0.99*radius, length, Primitive::s_resolution, 1);
      glColor4fv(m_begin->m_color);
      gluCylinder(quad, radius, radius, length/2, Primitive::s_resolution, 1);
   }

   glPopMatrix();
   gluDeleteQuadric(quad); 
}



void Bond::drawWireFrame(bool selected) 
{
   GLfloat radius(s_radiusWireFrame*m_scale);

   Vec a(m_begin->displacedPosition());
   Vec b(m_end  ->displacedPosition());
   GLfloat length((a-b).norm());

   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   GLboolean lightingEnabled(glIsEnabled(GL_LIGHTING));
   glDisable(GL_LIGHTING);

   if (!selected) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LINE_SMOOTH);
   }

   Vec normal = cross(s_cameraPosition-a, s_cameraPosition-b);
   normal.normalize();

   a.setValue(0.0, 0.0, 0.0); 
   b.setValue(0.0, 0.0, length); 
   Vec m(0.5*(a+b)); // bond midpoint

   if (selected) {

      glColor4fv(Primitive::s_selectColor);
      radius += Primitive::s_selectOffsetWireFrame;
      glLineWidth(radius);
      glBegin(GL_LINES);
         glVertex3f(a.x, a.y, a.z); 
         glVertex3f(b.x, b.y, b.z); 
      glEnd();

   }else {

      glLineWidth(radius);
   
      switch (m_order) {
         case 2: 
            normal *= 0.03;  // Governs the offset for the bond lines
            glBegin(GL_LINES);
               a -= normal;
               b -= normal;
               m -= normal;
               glColor4fv(m_begin->m_color);
               glVertex3f(a.x, a.y, a.z); 
               glVertex3f(m.x, m.y, m.z); 
               glColor4fv(m_end->m_color);
               glVertex3f(m.x, m.y, m.z); 
               glVertex3f(b.x, b.y, b.z); 

               a += 2.0*normal;
               b += 2.0*normal;
               m += 2.0*normal;
               glColor4fv(m_begin->m_color);
               glVertex3f(a.x, a.y, a.z); 
               glVertex3f(m.x, m.y, m.z); 
               glColor4fv(m_end->m_color);
               glVertex3f(m.x, m.y, m.z); 
               glVertex3f(b.x, b.y, b.z); 
            glEnd();
         break;

         case 3: 
            normal *= 0.04;  // Governs the offset for the bond lines
            glBegin(GL_LINES);
               a -= normal;
               b -= normal;
               m -= normal;
               glColor4fv(m_begin->m_color);
               glVertex3f(a.x, a.y, a.z); 
               glVertex3f(m.x, m.y, m.z); 
               glColor4fv(m_end->m_color);
               glVertex3f(m.x, m.y, m.z); 
               glVertex3f(b.x, b.y, b.z); 

               a += normal;
               b += normal;
               m += normal;
               glColor4fv(m_begin->m_color);
               glVertex3f(a.x, a.y, a.z); 
               glVertex3f(m.x, m.y, m.z); 
               glColor4fv(m_end->m_color);
               glVertex3f(m.x, m.y, m.z); 
               glVertex3f(b.x, b.y, b.z); 

               a += normal;
               b += normal;
               m += normal;
               glColor4fv(m_begin->m_color);
               glVertex3f(a.x, a.y, a.z); 
               glVertex3f(m.x, m.y, m.z); 
               glColor4fv(m_end->m_color);
               glVertex3f(m.x, m.y, m.z); 
               glVertex3f(b.x, b.y, b.z); 
            glEnd();
         break;

         default: 
            glBegin(GL_LINES);
               glColor4fv(m_begin->m_color);
               glVertex3f(a.x, a.y, a.z); 
               glVertex3f(m.x, m.y, m.z); 
               glColor4fv(m_end->m_color);
               glVertex3f(m.x, m.y, m.z); 
               glVertex3f(b.x, b.y, b.z); 
            glEnd();
         break;
      }
   }

   if (!selected) {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
   }
   if (lightingEnabled) glEnable(GL_LIGHTING);

   glPopMatrix();
}


void Bond::updateOrientation() 
{
   Vec a(m_begin->displacedPosition());
   Vec b(m_end  ->displacedPosition());
   Quaternion orient(s_zAxis, b-a); 
   setPosition(a);
   setOrientation(orient); 
}


} } // end namespace IQmol::Layer
