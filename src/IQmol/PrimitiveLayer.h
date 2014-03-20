#ifndef IQMOL_PRIMITIVE_H
#define IQMOL_PRIMITIVE_H
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

#include "GLObjectLayer.h"

#ifdef Q_WS_MAC
#include <OpenGL/glu.h>
#else 
#include <GL/glu.h>
#endif

class QGLViewer;
class QFontMetrics;

namespace IQmol {

namespace Data {
   class Geometry;
}

namespace Layer {

   /// Abstract base class for primitive 'components' of the molecular system.  
   /// These include things like Atoms, Bonds and Charges.  The Primitive class
   /// covers some of the common appearance such as the DrawMode and scaling.
   class Primitive : public GLObject {

      Q_OBJECT

      public:
         enum DrawMode { BallsAndSticks, Tubes, SpaceFilling, WireFrame };

		 Primitive(QString const& label = QString()) : GLObject(label), 
            m_drawMode(BallsAndSticks), m_scale(1.0), m_inGroup(false) { }
           
         virtual ~Primitive() { }

         virtual void drawLabel(QGLViewer&, QFontMetrics&) {  }
         virtual void setIndex(int const index) { m_index = index; }
         int  index() const { return m_index; }
         void setScale(double const scale) { m_scale = scale; }
         virtual void setDrawMode(DrawMode const drawMode) { m_drawMode = drawMode; }
         
         static double distance(Primitive* A, Primitive* B);
         static double angle(Primitive* A, Primitive* B, Primitive* C);
         static double torsion(Primitive* A, Primitive* B, Primitive* C, Primitive* D);


bool isInGroup() const { return m_inGroup; }
void setInGroup(bool tf) { m_inGroup = tf; }

      protected:
         static int s_resolution;
         static GLfloat s_selectColor[];
         static GLfloat s_selectOffset;
         static GLfloat s_selectOffsetWireFrame;
         void setLabel(QString const& label) { m_label = label; }
         DrawMode m_drawMode;
         QString m_label;
         int m_index;
         double m_scale;

bool m_inGroup;
   };


   class PrimitiveList : public QList<Primitive*> {
      public:
         PrimitiveList() : QList<Primitive*>() { }
         PrimitiveList(PrimitiveList const& list) : QList<Primitive*>(list) { }
         PrimitiveList(QList<Primitive*> const& list) : QList<Primitive*>(list) { }
         PrimitiveList(IQmol::Data::Geometry const&, bool includeBonds = true);
   };


} } // end namespace IQmol::Layer


#endif
