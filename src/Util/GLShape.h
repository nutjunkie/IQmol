#ifndef IQMOL_GLSHAPE_H
#define IQMOL_GLSHAPE_H
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

#include <cmath>
#include "QGLViewer/vec.h"

class GLUquadric;

namespace IQmol {
namespace GLShape {

   class Base {
      public:
         Base(QColor const& color = QColor());
         virtual void draw() = 0;
         virtual ~Base() { }
         static GLfloat LengthScale(GLint const resolution);
         
      protected:
         bool   isoChromic(Base const&);
         static GLfloat s_thresh;
         static GLUquadric* s_quadric;
         GLint  m_red;
         GLint  m_green;
         GLint  m_blue;
   };


   class GLSphere : public Base {

      public:
         GLSphere(GLfloat const radius, int const resolution, QColor const& color = QColor());
         bool operator==(GLSphere const&);
         void draw();

      private:
         GLfloat m_radius;
         GLint   m_segments;
   };


   class GLTorus : public Base {

      public:
         GLTorus(GLfloat const majorRadius, GLfloat const minorRadius, int const resolution,
            GLfloat const angle = 2.0f*M_PI, QColor const& color = QColor());
         bool operator==(GLTorus const&);
         void draw();

      private:
        GLfloat m_majorRadius;
        GLfloat m_minorRadius;
        GLfloat m_angle;
        GLint   m_majorSegments;
        GLint   m_minorSegments;
        GLboolean m_capEnds;
   };
   



   /// Draws a torus with the maximum length of a polygon set to resolution.
   /// The angle is in radians and if less than 2pi a partial torus is drawn.
   void Torus(double const majorRadius, double const minorRadius, double const resolution, 
      double const angle = 2.0*M_PI);

   /// Draws a torus with majorSegments strips each with minorSegments quads.
   /// The angle is in radians and if less than 2pi a partial torus is drawn.
   void Torus(double const majorRadius, double const minorRadius, int const majorSegments,
      int const minorSegments, double const angle = 2.0*M_PI);

   void Circle(double const radius, double const resolution, double const angle = 2.0*M_PI);

   /// Draws a 2D sector starting from the x-axis going counter-clockwise.  
   /// The line segments have a maximum length of resolution.
   void Sector(double const radius, double const resolution, bool const fill = false,
      double const angle = 2.0*M_PI);

   /// Draws a 2D sector starting from the x-axis going counter-clockwise.  
   void Sector(double const radius, int const nSegments, bool const fill = false,
      double const angle = 2.0*M_PI);


   /// Draws an arrow of the given length and tube radius.
   void Arrow(double const length, double const radius = -1.0, double const head = -1.0,
      int const resolution = 24);


   /// Convenience function that draws a cylinder between two points
   void Tube(qglviewer::Vec const& a, qglviewer::Vec const& b, double const radius,
      double const resolution, bool const capped = false);

   /// Convenience function that draws a cylinder between two points
   void Tube(qglviewer::Vec const& a, qglviewer::Vec const& b, double const radius,
      int const divisions, bool const capped = false);


   /// Convenience function that draws a sphere at a point
   void Sphere(qglviewer::Vec const& center, double const radius, double const resolution);

   /// Convenience function that draws a sphere at a point
   void Sphere(qglviewer::Vec const& center, double const radius, int const divisions);



} } // end namespace IQmol::GLShape

#endif
