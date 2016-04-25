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

#include "PovRayGen.h"
#include "ClippingPlaneLayer.h"
#include "QsLog.h"
#include "Numerical.h"
#include <QtDebug>


using namespace qglviewer;

namespace IQmol {

PovRayGen::PovRayGen(QVariantMap const& settings) : m_lightFront(true), m_lightHighlight(true),
   m_lightLeft(true), m_lightLower(false), m_meshCount(0), m_settings(settings)
{
   qDebug() << "Generating PovRay file";
   QString fileName("iqmol.pov");
   m_file.setFileName(fileName);

   if (m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      m_stream.setDevice(&m_file);
      writeHeader();
      //writeAxes();
      writeAtomMacro();
      writeBondMacro();
   }else {
      QLOG_WARN() << "Unable to open PovRay file" << fileName;
   }
}


PovRayGen::~PovRayGen()
{
   m_file.close();
}


void PovRayGen::writeHeader()
{
   m_stream << "//--------------------------------------------------\n";
   m_stream << "// POV-Ray 3.6/3.7 Scene File generated using IQmol\n";
   m_stream << "//--------------------------------------------------\n";
   m_stream << "\n";
   m_stream << "//------------------------------------------\n";
   m_stream << "#version 3.6; // 3.7\n";
   m_stream << "#include \"colors.inc\"\n";
   m_stream << "#include \"textures.inc\"\n";
   m_stream << "//------------------------------------------\n";
   m_stream << "\n\n";
   m_stream << "global_settings {\n";
   m_stream << "   assumed_gamma 3.0\n";
   m_stream << "}\n\n";
}


void PovRayGen::setShaderSettings(QVariantMap const& settings)
{
   bool ok(true);

   float ambient(settings.value("user_Ambient").toFloat(&ok));
   if (!ok) ambient = 0.6;

   float diffuse(settings.value("user_Diffuse").toFloat(&ok));
   if (!ok) diffuse = 0.75;

   float specular(settings.value("user_Highlights").toFloat(&ok));
   if (!ok) specular = 0.56;

   float phong(settings.value("user_Shininess").toFloat(&ok));
   if (!ok) phong = 0.75;

   m_lightFront     = settings.value("user_light_Front").toBool();
   m_lightHighlight = settings.value("user_light_Highlight").toBool();
   m_lightLeft      = settings.value("user_light_Left").toBool();
   m_lightLower     = settings.value("user_light_Lower").toBool();

   m_stream << "#default {\n";
   m_stream << "   finish {\n";
   m_stream << "      phong      " << 0.75     << "\n";
   m_stream << "      ambient    " << ambient  << "\n";
   m_stream << "      diffuse    " << diffuse  << "\n";
   m_stream << "      specular   " << specular << "\n";
   m_stream << "      reflection " << phong    << "\n";
   m_stream << "   }   \n";
   m_stream << "}\n\n";
}


void PovRayGen::writeAtomMacro()
{
// rough texture, bumps scale 0.1->0.01
//normal { bumps 1.0 scale 0.05 }
//normal { dents 0.5 scale 0.05 }
//normal { ripples 0.5 scale 0.05 }
/*
    texture {
       //Peel scale 0.5 translate<2,0,0>
      pigment { FBM_Clouds }
   }
*/
   m_stream << "#macro Atom(pos, col, rad)\n";
   m_stream << "sphere {\n";
   m_stream << "   pos, rad \n";

   writeAtomTexture();

   m_stream << "}\n";
   m_stream << "#end\n\n";
/*
      normal {
         bumps 0.4  // controls depth of bumps
         scale .05  // controls width of bumps
      }
*/
}


void PovRayGen::writeBondMacro()
{
   m_stream << "#macro Bond(beginAtom, endAtom, col, rad)\n";
   m_stream << "cylinder {\n";
   m_stream << "   beginAtom, endAtom, rad\n";
   m_stream << "   texture {\n";
   m_stream << "      pigment {\n";
   m_stream << "         color rgbt col\n";
   m_stream << "      }\n";
   m_stream << "   }\n";
   m_stream << "}\n";
   m_stream << "#end\n\n";
}


void PovRayGen::writeAxes()
{
   m_stream << "cylinder {\n";
   m_stream << "   <0,0,0>, <5,0,0>, 0.1 \n";
   m_stream << "   texture {\n";
   m_stream << "      pigment {\n";
   m_stream << "         color <1.0, 0.0, 0.0>\n";
   m_stream << "      }\n";
   m_stream << "   }\n";
   m_stream << "}\n";

   m_stream << "cylinder {\n";
   m_stream << "   <0,0,0>, <0,5,0>, 0.1 \n";
   m_stream << "   texture {\n";
   m_stream << "      pigment {\n";
   m_stream << "         color <0.0, 1.0, 0.0>\n";
   m_stream << "      }\n";
   m_stream << "   }\n";
   m_stream << "}\n";

   m_stream << "cylinder {\n";
   m_stream << "   <0,0,0>, <0,0,-5>, 0.1 \n";
   m_stream << "   texture {\n";
   m_stream << "      pigment {\n";
   m_stream << "         color <0.0, 0.0, 1.0>\n";
   m_stream << "      }\n";
   m_stream << "   }\n";
   m_stream << "}\n";
}



void PovRayGen::setCamera(qglviewer::Camera* camera)
{
   Vec pos(camera->position());
   double angle(camera->fieldOfView());
   angle *= 180/M_PI;
   
   //angle += 15;  // The angle seem to come out different for POVRay

   // The camera coordinate system
   Vec x(camera->rightVector());
   Vec y(camera->upVector());
   //Vec z(camera->viewDirection());
   double r(camera->sceneRadius());

   //qDebug() << "Camera position " << pos.x << pos.y << pos.z;
   //qDebug() << "Camera direction" << z.x << z.y << z.z;
   //qDebug() << "Camera up       " << y.x << y.y << y.z;
   //qDebug() << "Camera right    " << x.x << x.y << x.z;

   m_stream << "camera {\n";
   m_stream << "   location " << formatVector(pos) << "\n";
   m_stream << "   sky      " << formatVector(y)   << "\n";
   m_stream << "   look_at  <0, 0, 0>\n";
   m_stream << "   angle " << Util::round(angle) <<  "\n";
   m_stream << "}\n\n";

   r *= 100.0;

   pos.normalize();
   pos.z *= -1.0;
   y.z   *= -1.0;
   x.z   *= -1.0;

   // Light directions are taken from the phong.vert shader
   Vec light;

   if (m_lightFront) {
      //const vec4 lightDirection0 = vec4( 0.4,  0.0,  1.0);  
      light = 0.4*x + 0.0*y + 1.0*pos;
      light = r * light.unit();
      m_stream << "// Light: front\n";
      writeLight(light, Qt::lightGray);
   }

   if (m_lightHighlight) {
      //const vec4 lightDirection1 = vec4( 0.3,  0.8, -0.5);  // highlight
      light = 0.3*x + 0.8*y - 0.5*pos;
      light = r * light.unit();
      m_stream << "// Light: highlight\n";
      writeLight(light, Qt::lightGray);
   }

   if (m_lightLeft) {
      //const vec4 lightDirection2 = vec4(-0.5,  0.0,  0.0);  // left
      light = -0.5*x + 0.0*y + 0.0*pos;
      light = r * light.unit();
      m_stream << "// Light: left\n";
      writeLight(light, Qt::lightGray);
   }

   if (m_lightLower) {
      //const vec4 lightDirection3 = vec4( 0.0, -1.0,  0.2);  // lower
      light = 0.0*x - 1.0*y + 0.2*pos;
      light = r * light.unit();
      m_stream << "// Light: lower\n";
      writeLight(light, Qt::lightGray);
   }
}


void PovRayGen::writeLight(Vec const& pos, QColor const& col)
{
   m_stream << "light_source {\n";
   m_stream << "   <" << pos.x << ", " << pos.y << ", " << pos.z << ">\n";
   m_stream << "   color   rgb<" <<  col.redF()  << ", " <<  col.greenF() << ", " 
            <<  col.blueF() << ">\n";
   m_stream << "}\n\n";
}


void PovRayGen::writeAreaLight(double const r)
{
   m_stream << "light_source{ <0,0,0>\n";
   m_stream << "   color rgb<1,1,1>\n";
   m_stream << "   area_light\n";
   m_stream << "   <" << -r << ", 0, 0> <" << +r << ", 0, 0>\n";
   m_stream << "   1,10 // numbers in directions\n";
   m_stream << "   adaptive 0  // 0,1,2,3...\n";
   m_stream << "   jitter // random softening\n";
   m_stream << "   translate<0, " << 5*r << ",  0>\n";
   m_stream << "}\n";
}


void PovRayGen::writeSky()
{
// sky -------------------------------------
   m_stream << "plane { \n";
   m_stream << "   <0,1,0>,1 hollow\n";
   m_stream << "   texture {\n";
   m_stream << "      pigment{ \n";
   m_stream << "         bozo turbulence 0.92\n";
   m_stream << "         color_map{\n";
   m_stream << "                 [0.00 rgb<0.05,0.15,0.45>]\n";
   m_stream << "                 [0.50 rgb<0.05,0.15,0.45>]\n";
   m_stream << "                 [0.70 rgb<1,1,1>        ]   \n";
   m_stream << "                 [0.85 rgb<0.2,0.2,0.2>  ]\n";
   m_stream << "                 [1.00 rgb<0.5,0.5,0.5>  ]\n";
   m_stream << "         } //\n";
   m_stream << "         scale<1,1,1.5>*2.5\n";
   m_stream << "         translate<0,0,0>\n";
   m_stream << "      }   \n";
   m_stream << "      finish {\n";
   m_stream << "         ambient 2 \n";
   m_stream << "         diffuse 0\n";
   m_stream << "      }   \n";
   m_stream << "   }   \n";
   m_stream << "   scale 10000\n";
   m_stream << "}\n";
}


void PovRayGen::setBackground(QColor const& color)
{
   m_stream << "background {\n";
   m_stream << "   color Black\n";
   //m_stream << "   color <" << color.redF() << ", " << color.greenF() << ", " 
   //         << color.blueF() << ">\n";
   m_stream << "}\n\n";
}


void PovRayGen::setClippingPlane(Layer::ClippingPlane const& plane) 
{
   Vec normal(plane.normal());
   double distance(plane.distance());
   m_stream << "#declare Clipping_Plane =\n";
   m_stream << "object { \n";
   m_stream << "   plane { \n";
   m_stream << "      <" << -normal.x << ", " << -normal.y << ", " << normal.z 
            << ">, " << distance << "\n";
   m_stream << "   }\n";
   m_stream << "}\n\n";
}

// Note this inverts the z direction for conversion betwen LHS & RHS
QString PovRayGen::formatVector(qglviewer::Vec const& vec)
{
   QString s("<");
   s += QString::number( vec.x) + ", ";
   s += QString::number( vec.y) + ", ";
   s += QString::number(-vec.z) + ">";
   return s;
}


// Note this inverts the alpha component 
QString PovRayGen::formatColor(QColor const& color)
{
   QString s("<");
   s += QString::number(color.redF())   + ", ";
   s += QString::number(color.greenF()) + ", ";
   s += QString::number(color.blueF())  + ", ";
   s += QString::number(1.0-color.alphaF()) + ">";
   return s;
}


void PovRayGen::writeBond(qglviewer::Vec const& begin, qglviewer::Vec const& end, 
            QColor const& col, double const radius)
{
   m_stream << "Bond(" << formatVector(begin) << ", " << formatVector(end) << ", "
            << formatColor(col) << ", " << radius << ")\n";
}


void PovRayGen::writeAtom(Vec const& pos, QColor const& col, double const rad)
{
   m_stream << "Atom(" << formatVector(pos)  << ", " << formatColor(col) <<  ", " 
           << rad << ")\n";
}


void PovRayGen::writeMesh(QList<qglviewer::Vec> const& edges, QColor const& color, bool clip)
{
   QString id("Mesh_");
   id += QString::number(m_meshCount);
   ++m_meshCount;
   
   m_stream << "#declare " << id << "=\n";
   m_stream << "union {\n";

   for (int i = 0; i < edges.size(); i += 2) {
      m_stream << "   cylinder {" << formatVector(edges[i]) << ", " 
               << formatVector(edges[i+1]) << ", 0.01}\n";
   }

   m_stream << "}\n\n";

   m_stream << "object {\n";
   m_stream << "   " << id << "\n";
   m_stream << "   texture {\n";
   m_stream << "      pigment {\n";
   m_stream << "         color rgbt " << formatColor(color) << "\n";
   m_stream << "      }   \n";
   m_stream << "   }\n";
   if (clip) m_stream << "   clipped_by { Clipping_Plane }\n";
   m_stream << "}\n";
}


void PovRayGen::writeMesh(QList<qglviewer::Vec> const& vertices, 
   QList<qglviewer::Vec> const& normals, QList<int> const& faces, QColor const& color, bool clip)
{
   unsigned nVertices(vertices.size());
   unsigned nFaces(faces.size()/3);

   QString id("Mesh_");
   id += QString::number(m_meshCount);
   ++m_meshCount;
   
   m_stream << "#declare " << id << "=\n";
   m_stream << "mesh2 {\n";

   m_stream << "   vertex_vectors {\n";
   m_stream << "      " << nVertices;
   for (int i = 0; i < nVertices; ++i) {
       m_stream << ",\n   " << formatVector(vertices.at(i));
   }
   m_stream << "\n   }\n\n";

   m_stream << "   normal_vectors {\n";
   m_stream << "      " << nVertices;
   for (int i = 0; i < nVertices; ++i) {
       Vec n(normals.at(i));
       //m_stream << ",\n   " << formatVector(normals.at(i));
       m_stream << ",\n   " << formatVector(Vec(n.x, n.y, -n.z));
   }
   m_stream << "\n   }\n\n";

   m_stream << "   face_indices{\n";
   m_stream << "      " << nFaces;
   for (int i = 0; i < faces.size(); i += 3) {
       m_stream << ",\n   <" << faces.at(i) << ", " << faces.at(i+1) << ", " 
                << faces.at(i+2) << ">";
   }
   m_stream << "\n   }\n\n";
   m_stream << "\n}\n\n";

   m_stream << "object {\n";
   m_stream << "   " << id << "\n";

   writeSurfaceTexture(color);

   if (clip) m_stream << "   clipped_by { Clipping_Plane }\n";
   m_stream << "}\n";
}


void PovRayGen::writeSurfaceTexture(QColor const& color)
{
   qDebug() << m_settings;
   QString name(m_settings.value("surfaceTexture").toString());

   qDebug() << "Surface Texture set to" << name;
   if (name == "Clouds") {
      m_stream << "   texture {\n";
      m_stream << "      pigment { FBM_Clouds }\n";
      m_stream << "      finish {\n";
      m_stream << "         phong      0.75\n";
      m_stream << "         ambient    0.66\n";
      m_stream << "         diffuse    0.91\n";
      m_stream << "         specular   0.31\n";
      m_stream << "         reflection 0.26\n";
      m_stream << "      }\n";
      m_stream << "   }\n";
   }else {
      m_stream << "   texture {\n";
      m_stream << "      pigment {\n";
      m_stream << "         color rgbt " << formatColor(color) << "\n";
      m_stream << "      }\n";
      m_stream << "   }\n";
   }
}


void PovRayGen::writeAtomTexture()
{
   qDebug() << "==================";
   qDebug() << m_settings;
   qDebug() << "==================";
   QString name(m_settings.value("moleculeTexture").toString());

   qDebug() << "Molecule Texture set to" << name;
   if (name == "Speckle") {
      m_stream << "   texture {\n";
      m_stream << "      pigment { color rgbt col }\n";
      m_stream << "      normal  { bumps 1.0 scale 0.05 }\n";
      m_stream << "   }\n";
   }else {
      m_stream << "   texture {\n";
      m_stream << "      pigment { color rgbt col }\n";
      m_stream << "   }\n";
   }
}


} // end namespace IQmol
