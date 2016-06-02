#ifndef IQMOL_POVRAYGEN_H
#define IQMOL_POVRAYGEN_H
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

#include "QGLViewer/camera.h"
#include <QTextStream>
#include <QColor>
#include <QFile>


namespace IQmol {

namespace Layer {
   class ClippingPlane;
}

   class PovRayGen {

      public:
         PovRayGen(QString const& filename, QVariantMap const& settings, 
            QMap<QString, QString> const& textures);
         ~PovRayGen();

         void setCamera(qglviewer::Camera*);
         void setBackground(QColor const&);
         void setClippingPlane(Layer::ClippingPlane const&);
         void setShaderSettings(QVariantMap const& settings);

         void writeAtom(qglviewer::Vec const& pos, QColor const&, double const radius);

         void writeBond(qglviewer::Vec const& begin, qglviewer::Vec const& end, 
            QColor const& col, double const radius);
   
         void writeMesh(QList<qglviewer::Vec> const& vertices, 
            QList<qglviewer::Vec> const& normals,   QList<int> const& faces, 
            QColor const&, bool clip);
           

         void writeMesh(QList<qglviewer::Vec> const& edges, QColor const&, bool clip);

      private:
         void writeHeader();
         void writeLight(qglviewer::Vec const& position, QColor const& color = Qt::white);
         void writeAtomMacro();
         void writeBondMacro();
         void writeSurfaceMacro();
         void writeAxes();
         void writeSky();
         void writeAreaLight(double const size);
         void writeTexture(QString const& name);

         bool m_lightFront;
         bool m_lightHighlight;
         bool m_lightLeft;
         bool m_lightLower;

         QFile m_file;
         QTextStream m_stream;
         unsigned m_meshCount;

         QString formatVector(qglviewer::Vec const&);
         QString formatColor(QColor const&);

         QVariantMap m_settings;
         QMap<QString,QString>  m_textures;
   };

} // end namespace IQmol

#endif
