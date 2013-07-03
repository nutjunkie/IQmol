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

#include "QGLViewer/qglviewer.h"
#include "GLShape.h"
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace GLShape {

double const TwoPi = 2.0*M_PI;

GLUquadric* Base::s_quadric = 0;
GLfloat Base::s_thresh = 0.000001;

Base::Base(QColor const& color) : m_red(-1), m_green(-1), m_blue(-1)
{
   if (color.isValid()) {
      m_red   = color.red();
      m_green = color.green();
      m_blue  = color.blue();
   }

   if (s_quadric == 0) s_quadric = gluNewQuadric();
}


bool Base::isoChromic(Base const& rhs) 
{
   return m_red == rhs.m_red  &&  m_green == rhs.m_green  &&  m_blue == rhs.m_blue;
}


GLfloat Base::LengthScale(GLint const resolution)
{
   return pow(2.0f, -0.5f*resolution);
}


// ---------- Sphere -----------

GLSphere::GLSphere(GLfloat const radius, int const resolution, QColor const& color) :
   Base(color), m_radius(radius) 
{ 
   m_segments = ceil(M_PI*m_radius / LengthScale(resolution));
}


bool GLSphere::operator==(GLSphere const& rhs)
{
   return ( (m_radius-rhs.m_radius) < s_thresh && 
            (m_segments == rhs.m_segments)    &&
            isoChromic(rhs));
}


void GLSphere::draw()
{
   if (m_red >= 0) glColor3i(m_red, m_blue, m_green);
   gluQuadricOrientation(s_quadric, GLU_OUTSIDE);
   gluSphere(s_quadric, m_radius, 2*m_segments, m_segments);
}



// ---------- Torus -----------

GLTorus::GLTorus(GLfloat const majorRadius, GLfloat const minorRadius, int const resolution,
   GLfloat const angle, QColor const& color) : Base(color), m_majorRadius(majorRadius),
   m_minorRadius(minorRadius), m_angle(angle)
{
   GLfloat d(LengthScale(resolution));
   m_majorSegments = ceil(fabs(m_angle)*(m_majorRadius+m_minorRadius)/d);
   m_minorSegments = ceil(TwoPi*m_minorRadius/d);
}


bool GLTorus::operator==(GLTorus const& rhs)
{
   return ( (m_majorRadius-rhs.m_majorRadius) < s_thresh && 
            (m_minorRadius-rhs.m_minorRadius) < s_thresh && 
            (m_angle      -rhs.m_angle      ) < s_thresh && 
            (m_majorSegments == rhs.m_majorSegments)     &&
            (m_minorSegments == rhs.m_minorSegments)     &&
            isoChromic(rhs));
}


void GLTorus::draw()
{
   GLfloat R(m_majorRadius);
   GLfloat r(m_minorRadius);
   GLfloat dA(m_angle/m_majorSegments);
   GLfloat da(TwoPi/m_minorSegments);
   GLfloat cosTheta[2], sinTheta[2], cosPhi, sinPhi;
   Vec v, n;
   
   for (int i = 0; i < m_majorSegments; ++i) {
       cosTheta[0] = cos( i   *dA);
       cosTheta[1] = cos((i+1)*dA);
       sinTheta[0] = sin( i   *dA);
       sinTheta[1] = sin((i+1)*dA);

       glBegin(GL_QUAD_STRIP);
       for (int j = 0; j <= m_minorSegments; ++j) {
           cosPhi = cos(j*da);
           sinPhi = sin(j*da);

           for (int k = 0; k <= 1; ++k) {
               v.x = (R+r*cosPhi) * cosTheta[k];
               v.y = (R+r*cosPhi) * sinTheta[k];
               v.z = r*sinPhi;

               n.x = R*cosTheta[k];
               n.y = R*sinTheta[k];
               n.z = 0;
               n = (v-n).unit();

               glNormal3f(n.x, n.y, n.z); 
               glVertex3f(v.x, v.y, v.z);
           }
       }
       glEnd();
   }     
}




////-------------------------------------------------------
////-------------------------------------------------------
////-------------------------------------------------------
////-------------------------------------------------------
void Torus(double const majorRadius, double const minorRadius, double const resolution, 
   double const angle)
{
   double majorArcLength(fabs(angle)*(majorRadius+minorRadius));
   double minorArcLength(TwoPi*minorRadius);
   int majorSegments(ceil(majorArcLength/resolution));
   int minorSegments(ceil(minorArcLength/resolution));
   return Torus(majorRadius, minorRadius, majorSegments, minorSegments, angle);
}


void Torus(double const majorRadius, double const minorRadius, int const majorSegments,
   int const minorSegments, double const angle)
{
   GLfloat R(majorRadius);
   GLfloat r(minorRadius);
   GLfloat dA(angle/majorSegments);
   GLfloat da(TwoPi/minorSegments);
   GLfloat cosTheta[2], sinTheta[2], cosPhi, sinPhi;
   Vec v, n;
   
   for (int i = 0; i < majorSegments; ++i) {
       cosTheta[0] = cos( i   *dA);
       cosTheta[1] = cos((i+1)*dA);
       sinTheta[0] = sin( i   *dA);
       sinTheta[1] = sin((i+1)*dA);

       glBegin(GL_QUAD_STRIP);
       for (int j = 0; j <= minorSegments; ++j) {
           cosPhi = cos(j*da);
           sinPhi = sin(j*da);

           for (int k = 0; k <= 1; ++k) {
               v.x = (R+r*cosPhi) * cosTheta[k];
               v.y = (R+r*cosPhi) * sinTheta[k];
               v.z = r*sinPhi;

               n.x = R*cosTheta[k];
               n.y = R*sinTheta[k];
               n.z = 0;
               n = (v-n).unit();

               glNormal3f(n.x, n.y, n.z); 
               glVertex3f(v.x, v.y, v.z);
           }
       }
       glEnd();
   }     

   /* DEBUG
   glLineWidth(2.0);
   glBegin(GL_LINES);
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(1.0, 0.0, 0.0);
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(0.0, 2.0, 0.0);
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(0.0, 0.0, 3.0);
   glEnd();
   */

   bool capEnds(false);
   if (angle < TwoPi && capEnds) {
      glPushMatrix();
      GLUquadric* quadric = gluNewQuadric();
      glTranslatef(R, 0.0, 0.0);
      gluSphere(quadric, r, minorSegments, minorSegments);
      glTranslatef(R*(cos(angle)-1.0), R*sin(angle), 0.0);
      gluSphere(quadric, r, minorSegments, minorSegments);
      glPopMatrix();
      gluDeleteQuadric(quadric);
   }
}


void Sector(double const radius, double const resolution, bool const fill, double const angle)
{
   double arcLength(angle*radius);
   int nSegments(ceil(arcLength/resolution));
   return Sector(radius, nSegments, fill, angle);
}


void Sector(double const radius, int const nSegments, bool const fill, double const angle)
{
   double theta(angle/nSegments);      // angle of each segment
   double tanTheta(tan(theta));
   double radFactor(cos(theta));
   double x(radius), y(0.0f), tx, ty;

   if (fill) {
      glBegin(GL_TRIANGLE_FAN);
      glVertex2f(0, 0);
   }else {
      glBegin(GL_LINE_LOOP);
   }

   // The algorithm for drawing circles is based on
   // http://slabode.exofire.net/circle_draw.shtml
   for (int i = 0; i <= nSegments; ++i) {
       glVertex2f(x, y);
       tx = -y;
       ty =  x;
       x += tx * tanTheta;
       y += ty * tanTheta;
       x *= radFactor;
       y *= radFactor;
   }
   glEnd();
}


void Arrow(double const length, double const radius, double const head, 
   int const resolution)
{
   GLfloat tubeRadius = (radius > 0.0f) ? radius : 0.02f * length;
   GLfloat headRadius = 3.0*tubeRadius;
   GLfloat headLength = (head > 0.0f) ? head : 3.0*headRadius;
   GLfloat tubeLength = length-headLength;

   GLUquadric* quadric = gluNewQuadric();

   if (2.0*length < headLength) {
      // Don't draw small arrows to keep things tidy
      return;
   } else if (length < headLength) {
      headRadius *= length/headLength;
      headLength  = length;
      tubeLength  = 0.0f;
   }else {
      // only arrows longer than headLength need a tube;
      gluQuadricOrientation(quadric, GLU_OUTSIDE);
      gluCylinder(quadric, tubeRadius, tubeRadius, tubeLength, resolution, 1);
      gluQuadricOrientation(quadric, GLU_INSIDE);
      gluDisk(quadric, 0.0, tubeRadius, resolution, 1);
   }

   // Draw the head as a cone
   glPushMatrix();
   glTranslatef(0.0, 0.0, tubeLength);
   gluQuadricOrientation(quadric, GLU_OUTSIDE);
   gluCylinder(quadric, headRadius, 0.0, headLength, resolution, 1);
   gluQuadricOrientation(quadric, GLU_INSIDE);
   gluDisk(quadric, 0.0, headRadius, resolution, 1);
   glPopMatrix();

   gluDeleteQuadric(quadric);
}


void Tube(Vec const& a, Vec const& b, double const radius, double const resolution,
   bool const capped)
{
   double circumference(TwoPi*radius);
   int nSegments(ceil(circumference/resolution));
   return Tube(a, b, radius, nSegments, capped);
}


void Tube(Vec const& a, Vec const& b, double const radius, int const divisions, 
   bool const capped)
{
   double length((a-b).norm());
   Quaternion orientation(Vec(0.0, 0.0, 1.0), b-a);
   Frame frame(a, orientation);

   glPushMatrix();
      glMultMatrixd(frame.matrix());
      GLUquadric* quadric = gluNewQuadric();
      gluQuadricOrientation(quadric, GLU_OUTSIDE);
      gluCylinder(quadric, radius, radius, length, divisions, 1);

      if (capped) {
         gluQuadricOrientation(quadric, GLU_INSIDE);
         gluDisk(quadric, 0.0, radius, divisions, 1);
         glTranslatef(0.0, 0.0, length);
         gluQuadricOrientation(quadric, GLU_OUTSIDE);
         gluDisk(quadric, 0.0, radius, divisions, 1);
      }

      gluDeleteQuadric(quadric);
   glPopMatrix();
}


void Sphere(Vec const& center, double const radius, double const resolution)
{
   double circumference(TwoPi*radius);
   int nSegments(ceil(circumference/resolution));
   return Sphere(center, radius, nSegments);
}


void Sphere(Vec const& center, double const radius, int const divisions)
{
   glPushMatrix();
   glTranslatef(center.x, center.y, center.z);
   GLUquadric* quadric = gluNewQuadric();
   gluQuadricOrientation(quadric, GLU_OUTSIDE);
   gluSphere(quadric, radius, divisions, divisions);
   gluDeleteQuadric(quadric);
   glPopMatrix();
}

} } // end namespace IQmol::GLShape
