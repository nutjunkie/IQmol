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

#include "BondLayer.h"
#include "AtomLayer.h"
#include "GLShape.h"
#include "PovRayGen.h"

#include <QDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {

// Static Data
GLfloat Bond::s_defaultColor[]       = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat Bond::s_plasticColor[]       = {0.6f, 0.6f, 0.6f, 1.0f};
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
      case Primitive::Plastic:
         drawPlastic(selected);
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


void Bond::drawPlastic(bool selected)
{
    // We don't allow scaling cos it looks naff
   GLfloat offset(0.09);   // height of the cap
   GLfloat bondRadius(0.10);
   GLfloat capRadius(2*bondRadius);
   GLfloat aRadius = m_begin->smallHydrogen() ? 0.28 : 0.40;
   GLfloat bRadius = m_end->smallHydrogen()   ? 0.28 : 0.40;
   GLfloat aShift  = aRadius + offset - capRadius;
   GLfloat bShift  = bRadius + offset - capRadius;

   if (selected) {
      bondRadius += Primitive::s_selectOffset;
      capRadius  += Primitive::s_selectOffset;
      glColor4fv(Primitive::s_selectColor);
   }else {
      glColor4fv(s_plasticColor);
   }

   Vec a(m_begin->displacedPosition());
   Vec b(m_end  ->displacedPosition());
   Vec ab(b-a);

   GLfloat length(ab.normalize());

   GLUquadric* quad = gluNewQuadric();
   Frame frame(m_frame);

   switch (m_order) {
      case 20: {
         GLfloat s(0.40); // separation
         GLfloat r(0.5*(s+0.25*length*length)/s); // radius of torus
         bondRadius *= 0.7;

         Vec normal = cross(s_cameraPosition-a, s_cameraPosition-b);
         //Vec normal(cross(ab,s_cameraDirection));
         normal.normalize();

         Vec midPoint(a+0.5*length*ab);
         Vec displacement((r-s)*normal);
         Vec center(midPoint-displacement);
         Vec ca(a-center);
         Vec cb(b-center);

/*
// These lengths should be the same
qDebug() << "Lengths: ca:" << r << ca.norm() << cb.norm(); 
qDebug() << "Midpoint " << midPoint.x << midPoint.y << midPoint.z;
qDebug() << "A        " << a.x << a.y << a.z;
qDebug() << "B        " << b.x << b.y << b.z;
qDebug() << "Center   " << center.x << center.y << center.z;;
qDebug() << "Normal   " << normal.x << normal.y << normal.z;;
*/

         double arc(Quaternion(ca, cb).angle());

         // Align the frame x-axis to the ca
         frame.setPositionAndOrientation(center, Quaternion(Vec(1, 0, 0), ca));
         // Now determine the frame coordinates of b and 
         Vec fb(frame.coordinatesOf(b).unit());
         frame.rotate(Quaternion(Vec(1, 0, 0), atan2(fb.z, fb.y)));

         glPushMatrix();
         glMultMatrixd(frame.matrix());
         GLShape::Torus(r, bondRadius, 0.1, arc);

glColor3f(1.0, 0.0, 0.0);
         gluSphere(quad, 0.5*capRadius, Primitive::s_resolution, Primitive::s_resolution);
         glPopMatrix();

         //  and cap the ends
         if (length > aRadius+bRadius) {
            double thetaA(std::acos(1.0-0.5*(aRadius+offset)*(aRadius+offset)/(r*r)));
          //Vec capA(frame.inverseCoordinatesOf(Vec(r*std::cos(thetaA), r*std::sin(thetaA), 0.0)));
            Vec capA(r*std::cos(thetaA), r*std::sin(thetaA), 0.0);
            capA += capRadius*((Vec(r, 0, 0)-capA).unit());

            frame.setPosition(frame.inverseCoordinatesOf(capA));

            glPushMatrix();
            glMultMatrixd(frame.matrix());
glColor3f(0.0, 1.0, 0.0);
            gluSphere(quad, capRadius, Primitive::s_resolution, Primitive::s_resolution);
            glPopMatrix();


            //double thetaB(std::acos(1.0-bShift*bShift/(2*r*r)));
            double thetaB(std::acos(1.0-0.5*(bRadius+offset)*(bRadius+offset)/(r*r)));
            thetaB = arc-thetaB;

          //Vec capB(frame.inverseCoordinatesOf(Vec(r*std::cos(thetaB), r*std::sin(thetaB), 0.0)));
            Vec capB(r*std::cos(thetaB), r*std::sin(thetaB), 0.0);
            capB += capRadius*((Vec(r, 0, 0)-capB).unit());

            frame.setPosition(center);
            frame.setPosition(frame.inverseCoordinatesOf(capB));

            glPushMatrix();
            glMultMatrixd(frame.matrix());
glColor3f(0.0, 0.0, 1.0);
            gluSphere(quad, capRadius, Primitive::s_resolution, Primitive::s_resolution);
            glPopMatrix();
         }
 
glColor4fv(s_plasticColor);
  
         // second bond
         center = midPoint-displacement;
         ca = a-center;
		 cb = b-center;
         frame.setPositionAndOrientation(center, Quaternion(Vec(1, 0, 0), cb));
         Vec fa(frame.coordinatesOf(a).unit());
         frame.rotate(Quaternion(Vec(1, 0, 0), atan2(fa.z, fa.y)));

         glPushMatrix();
         glMultMatrixd(frame.matrix());
         GLShape::Torus(r, bondRadius, 0.1, arc);
         //gluSphere(quad, capRadius, Primitive::s_resolution, Primitive::s_resolution);
         glPopMatrix();

      } break;


      default: {
         // Draw regular bond
         glPushMatrix();
         glMultMatrixd(frame.matrix());
         gluCylinder(quad, bondRadius, bondRadius, length, Primitive::s_resolution, 1);
         glPopMatrix();

         //  and cap the ends
         if (length > aRadius+bRadius) {
            frame.translate(aShift*ab);
            glPushMatrix();
            glMultMatrixd(frame.matrix());
            gluSphere(quad, capRadius, Primitive::s_resolution, Primitive::s_resolution);
            glPopMatrix();

            frame.translate((length-(aShift+bShift))*ab);
            glPushMatrix();
            glMultMatrixd(frame.matrix());
            gluSphere(quad, capRadius, Primitive::s_resolution, Primitive::s_resolution);
            glPopMatrix();
         }
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


void Bond::povray(PovRayGen& povray)
{
   updateOrientation(); // This should only need calling if an atom is moving
   switch (m_drawMode) {
      case Primitive::BallsAndSticks:
         povrayBallsAndSticks(povray);
         break;
      case Primitive::Plastic:
         povrayPlastic(povray);
         break;
      case Primitive::Tubes:
         povrayTubes(povray);
         break;
      case Primitive::WireFrame:
         povrayWireFrame(povray);
         break;
      default:
         break;
   }
}


void Bond::povrayBallsAndSticks(PovRayGen& povray)
{
   double radius(s_radiusBallsAndSticks*m_scale);
   Vec A(m_begin->getPosition());
   Vec B(m_end->getPosition());
   Vec normal(cross(s_cameraPosition-A, s_cameraPosition-B).unit());

   switch (m_order) {
      case 1: {
         povray.writeBond(A, B, Qt::darkGray, radius);
      } break;
 
      case 2: {
         normal *= 0.08;  // Governs the offset for the bond lines
         radius *= 0.7;   // Make the bonds a bit thinner

         A -= normal;      B -= normal;
         povray.writeBond(A, B, Qt::darkGray, radius);

         A += 2.0*normal;  B += 2.0*normal;
         povray.writeBond(A, B, Qt::darkGray, radius);
      } break;

      case 3: {
         normal *= 0.11;  // Governs the offset for the bond lines
         radius *= 0.45;  // Make the bonds a bit thinner

         A -= normal;  B -= normal;
         povray.writeBond(A, B, Qt::darkGray, radius);
         
         A += normal;  B += normal;
         povray.writeBond(A, B, Qt::darkGray, radius);
         
         A += normal;  B += normal;
         povray.writeBond(A, B, Qt::darkGray, radius);
      } break;

      case 4: {
         normal *= 0.11;  // Governs the offset for the bond lines
         radius *= 0.40;  // Make the bonds a bit thinner

         A -= 1.5*normal;  B -= 1.5*normal;
         povray.writeBond(A, B, Qt::darkGray, radius);

         A += normal;  B += normal;
         povray.writeBond(A, B, Qt::darkGray, radius);
         
         A += normal;  B += normal;
         povray.writeBond(A, B, Qt::darkGray, radius);
         
         A += normal;  B += normal;
         povray.writeBond(A, B, Qt::darkGray, radius);
         
      } break;

      default: {
         radius *= 2;         // Fat bond indicates we don't know what we are doing
         povray.writeBond(A, B, Qt::darkGray, radius);
      } break;
   }
}


void Bond::povrayPlastic(PovRayGen& povray)
{ 
   // We don't allow scaling cos it looks naff
   double offset(0.09);   // height of the cap
   double bondRadius(0.10);
   double capRadius(2*bondRadius);
   double aRadius(m_begin->smallHydrogen() ? 0.28 : 0.40);
   double bRadius(m_end  ->smallHydrogen() ? 0.28 : 0.40);
   double aShift(aRadius + offset - capRadius);
   double bShift(bRadius + offset - capRadius);

   Vec A(m_begin->displacedPosition());
   Vec B(m_end  ->displacedPosition());
   Vec AB(B-A);

   double length(AB.normalize());

   // Draw regular bond
   povray.writeBond(A, B, Qt::lightGray, bondRadius);

   //  and cap the ends
   if (length > aRadius+bRadius) {
      A += aShift*AB;
      povray.writeAtom(A, Qt::lightGray, capRadius);
      A += (length-(aShift+bShift))*AB;
      povray.writeAtom(A, Qt::lightGray, capRadius);
   }
}



void Bond::povrayTubes(PovRayGen& povray)
{
   double radius(s_radiusTubes*m_scale);

   Vec A(m_begin->displacedPosition());
   Vec B(m_end  ->displacedPosition());
   Vec C(0.5*(A+B));

   povray.writeBond(A, C, m_begin->color(), radius);
   povray.writeBond(C, B, m_end->color(), radius);
}

void Bond::povrayWireFrame(PovRayGen& povray)
{
   double radius(0.02);

   Vec A(m_begin->displacedPosition());
   Vec B(m_end  ->displacedPosition());
   Vec C(0.5*(A+B));

   povray.writeBond(A, C, m_begin->color(), radius);
   povray.writeBond(C, B, m_end->color(), radius);
}



} } // end namespace IQmol::Layer
