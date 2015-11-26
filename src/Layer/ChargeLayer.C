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

#include "ChargeLayer.h"
#include "Preferences.h"
#include "QGLViewer/qglviewer.h"

#include <QColor>
#include <cmath>
#include <OpenGL.h>
//#include <GLUT/glut.h>

#include <QtDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {


// Static Data
GLfloat Charge::s_radiusWireFrame = 4.00;  // pixels
GLfloat Charge::s_radius          = 0.15;
GLfloat Charge::s_colorSaturation = 0.80;


Charge::Charge(double const charge, Vec const& position) : Primitive("Charge")
{
   setCharge(charge);
   setPosition(position);
}

// TODO - This shouldn't be here
QString Charge::toString() 
{
   QString s;
   qglviewer::Vec position(getPosition());
   s += QString::number(position.x, 'f', 6) + "   ";
   s += QString::number(position.y, 'f', 6) + "   ";
   s += QString::number(position.z, 'f', 6) + "   ";
   s += QString::number(m_charge,   'f', 6);
   return s;
}


void Charge::setCharge(double const charge)
{
   m_charge = charge;
   m_label = QString::number(m_charge, 'f', 2);
   setText("q = " + m_label);

   QColor red, green, blue;
   red.setHsvF(0.000, s_colorSaturation, 1.0);
   blue.setHsvF(0.666, s_colorSaturation, 1.0);
   green.setHsvF(0.333, 0.0, 1.0);

   double x(std::min(std::abs(m_charge), 1.0));
   x = std::sqrt(x);

   if (m_charge < 0.0) {
      m_color[0] = x*red.redF()   + (1.0-x)*green.redF();
      m_color[1] = x*red.greenF() + (1.0-x)*green.greenF();
      m_color[2] = x*red.blueF()  + (1.0-x)*green.blueF();
   }else {
      m_color[0] = x*blue.redF()   + (1.0-x)*green.redF();
      m_color[1] = x*blue.greenF() + (1.0-x)*green.greenF();
      m_color[2] = x*blue.blueF()  + (1.0-x)*green.blueF();
   }
   m_color[3] = m_alpha;
}


void Charge::draw()
{
   bool selectedOnly(false);
   drawPrivate(selectedOnly);
}


void Charge::drawSelected()
{
   bool selectedOnly(true);
   drawPrivate(selectedOnly);
}


void Charge::drawPrivate(bool selected) 
{
   glPushMatrix();
   glMultMatrixd(m_frame.matrix());
   
   if (selected) {
      glColor4fv(Primitive::s_selectColor);
   }else {
      glColor4fv(m_color);
   }

   if (m_drawMode == Primitive::WireFrame) {
      glPointSize(getRadius(selected));
      glDisable(GL_LIGHTING);
      glBegin(GL_POINTS);
         glVertex3f(0.0, 0.0, 0.0);
      glEnd();
      glEnable(GL_LIGHTING);
   }else {
      GLfloat scale(getRadius(selected));
      //drawCube(scale);
      drawOctahedron(scale);
   }

   glPopMatrix();
}


void Charge::drawOctahedron(GLfloat sz)
{

    GLfloat v[6][3] = {
       { 0.0, 0.0,  sz },
       {  sz, 0.0, 0.0 },
       { 0.0,  sz, 0.0 },
       { -sz, 0.0, 0.0 },
       { 0.0, -sz, 0.0 },
       { 0.0, 0.0, -sz }
    };

    //static GLfloat mag(0.5773502692);
    static GLfloat mag(0.5);
    static GLfloat n[8][3] = {
        { mag,  mag,  mag},
        {-mag,  mag,  mag},
        {-mag, -mag,  mag},
        { mag, -mag,  mag},
        { mag,  mag, -mag},
        {-mag,  mag, -mag},
        {-mag, -mag, -mag},
        { mag, -mag, -mag},
    };

    static GLint faces[8][3] = {
        {0, 1, 2},
        {0, 2, 3},
        {0, 3, 4},
        {0, 4, 1},
        {5, 2, 1},
        {5, 3, 2},
        {5, 4, 3},
        {5, 1, 4}
    };

    for (GLint i = 0; i < 8; ++i) {
        glBegin(GL_TRIANGLES);
        glNormal3fv(&n[i][0]);
        glVertex3fv(&v[ faces[i][0] ][0]);
        glVertex3fv(&v[ faces[i][1] ][0]);
        glVertex3fv(&v[ faces[i][2] ][0]);
        glEnd();
    }
}


void Charge::drawCube(GLfloat size)
{
    static GLfloat n[6][3] = {
        {-1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 0.0, -1.0}
    };
    static GLint faces[6][4] = {
        {0, 1, 2, 3},
        {3, 2, 6, 7},
        {7, 6, 5, 4},
        {4, 5, 1, 0},
        {5, 6, 2, 1},
        {7, 4, 0, 3}
    };
    GLfloat v[8][3];
    GLint i;

    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

    for (i = 5; i >= 0; i--) {
        glBegin(GL_QUADS);
        glNormal3fv(&n[i][0]);
        glVertex3fv(&v[faces[i][0]][0]);
        glVertex3fv(&v[faces[i][1]][0]);
        glVertex3fv(&v[faces[i][2]][0]);
        glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }
}


double Charge::getRadius(bool const selected)
{
   double r;
   switch (m_drawMode) {
      case Primitive::WireFrame:
         r = s_radiusWireFrame * m_scale;
         if (selected) r += Primitive::s_selectOffsetWireFrame; 
         break;
      default:
         r =  s_radius * m_scale;
         if (selected) r += Primitive::s_selectOffset; 
         break;
   }
   return  r;
}



void Charge::drawLabel(QGLViewer& viewer, QFontMetrics& fontMetrics) 
{
   Vec pos(getPosition());
   Vec shift = viewer.camera()->position() - pos;
   shift.normalize();

   pos = pos + 1.05 * shift * getRadius(true);
   pos = viewer.camera()->projectedCoordinatesOf(pos);
   pos.x -= fontMetrics.width(m_label)/2;
   pos = viewer.camera()->unprojectedCoordinatesOf(pos);
   viewer.renderText(pos[0], pos[1], pos[2], m_label);
}



Charges::Charges() : Base("Charges") 
{ 
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
   setCheckState(Qt::Checked);
}

} } // end namespace IQmol::Layer
