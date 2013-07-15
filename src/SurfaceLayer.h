#ifndef IQMOL_SURFACELAYER_H
#define IQMOL_SURFACELAYER_H
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

#include "Grid.h"
#include "GLObjectLayer.h"
#include "DataLayer.h"
#include "SurfaceConfigurator.h"
#include <QColor>


namespace qglviewer {
   class Vec;
}

namespace IQmol {
namespace Layer {

   /// Representation of a OpenGL surface.  Note that a surface layer is 
   /// potentially made up of two separate surfaces (positive and negative) so
   /// that molecular orbitals can be treated as a single Layer.  Surfaces can
   /// have property data attached which can be used to color the surface (e.g. 
   /// for ESPs).
   class Surface : public GLObject {

      Q_OBJECT

      friend class Configurator::Surface;

      public:
         typedef QList<GLfloat> Data;


/*
      public: class Facet {

         public:
            Facet(Data::const_iterator& iter);
            Data data() const;
            Data clip(qglviewer::Vec const& normal, qglviewer::Vec const& point) const;

         private:
            qglviewer::Vec m_a, m_na;
            qglviewer::Vec m_b, m_nb;
            qglviewer::Vec m_c, m_nc;
      };
*/


      public: // Surface
         enum DrawMode { Fill, Lines, Dots };
         enum Sign { Positive, Negative };

         Surface(Grid::DataType const type, int const quality, double const isovalue, 
            QColor const& positive, QColor const& negative = QColor(), bool upsample = false);
         ~Surface();

         void draw();
         void drawFast();
         void drawSelected();

         void setSurfaceData(Data const&, Sign sign = Positive);

         int  quality() const { return m_quality; }
         double isovalue() const { return m_isovalue; }
         bool upsample() const { return m_upsample; }
         void createToolTip(QString const& label = QString());
         double minPropertyValue() const { return m_min; }
         double maxPropertyValue() const { return m_max; }
         void setAlpha(double alpha);
         void setDrawMode(DrawMode const mode) { m_drawMode = mode; }
         
         Grid::DataType gridDataType() const { return m_type; }
         bool cubeIsSigned() const { return m_cubeIsSigned; }
         void setCubeIsSigned(bool tf) { m_cubeIsSigned = tf; }

         bool operator==(Surface const& that);
         bool operator!=(Surface const& that) { return !(*this == that); }

      protected:
         void setGradient(Gradient::ColorList const& colors) { m_colors = colors; }
         void setColor(QColor const& color, Sign sign = Positive);
         void computePropertyData(Function3D);
         void clearPropertyData();

         QColor color(Sign sign = Positive) const;

         Grid::DataType m_type;
         int      m_quality;
         double   m_isovalue;
         DrawMode m_drawMode;
         bool     m_upsample;
         Configurator::Surface m_configurator;

      private:
         void recompile();
         GLuint compile(Data const&, Data const& property = Data());
         bool isTransparent() const { return 0.01 <= m_alpha && m_alpha < 0.99; }

         GLuint m_callListPositive; 
         GLuint m_callListNegative; 

         Data m_surfaceDataPositive;
         Data m_surfaceDataNegative;
         Data m_propertyDataPositive;
         Data m_propertyDataNegative;
         GLfloat m_colorPositive[4];
         GLfloat m_colorNegative[4];
         Gradient::ColorList m_colors;
         double m_min;
         double m_max;
         double m_areaPositive;
         double m_areaNegative;
         bool m_cubeIsSigned;

         Data m_occlusionData;
   };

} // end namespace Layer

typedef QList<Layer::Surface*> SurfaceList;

} // end namespace IQmol

#endif
