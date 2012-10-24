#ifndef IQMOL_CHARGELAYER_H
#define IQMOL_CHARGELAYER_H
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

#include "PrimitiveLayer.h"
#include "DataLayer.h"


class QFontMetrics;
class QGLViewer;;
class QColor;;

namespace IQmol {
namespace Layer {

   /// Concrete Primitive class that represents a Charge.
   class Charge : public Primitive {

      Q_OBJECT

      friend class Molecule;

      public:
         Charge(double const charge, qglviewer::Vec const& position = 
            qglviewer::Vec(0.0, 0.0, 0.0));
         ~Charge() { }

         void draw(qglviewer::Vec const& cameraPosition = qglviewer::Vec(0.0, 0.0, 0.0));
         void drawSelected(qglviewer::Vec const& cameraPosition = qglviewer::Vec(0.0, 0.0, 0.0));
         void drawLabel(QGLViewer& viewer, QFontMetrics& fontMetrics);
         void setCharge(double const charge);

      protected:
         GLfloat m_color[4];

      private:
         double getRadius(bool selected);
         void drawPrivate(bool selected);
         void drawCube(GLfloat size);
         void drawOctahedron(GLfloat size);

         // Static Data
         static GLfloat s_radius;           // Default tube radius in angstroms
         static GLfloat s_radiusWireFrame;  // Default wire frame radius in pixels
         static GLfloat s_colorSaturation;  

         // Data
         int    m_index;
         double m_charge;
   };


   class Charges : public Data {

      Q_OBJECT;

      public:
         Charges() : Data("Charges") { }
         QList<Charge*> getCharges() { return findLayers<Charge>(Children); }
   };


}

typedef QList<Layer::Charge*> ChargeList;

} // end namespace IQmol::Layer

#endif
