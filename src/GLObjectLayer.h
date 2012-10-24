#ifndef IQMOL_GLOBJECTLAYER_H
#define IQMOL_GLOBJECTLAYER_H
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

#include "BaseLayer.h"
#include "QGLViewer/vec.h"
#include "QGLViewer/frame.h"
#include "QGLViewer/quaternion.h"
#include <QList>


namespace IQmol {

class ManipulatedFrameSetConstraint;

namespace Layer {

   /// Abstract base class for any item that can be draw on the GL canvas.
   /// Derived classes must implement a draw() function and, where the
   /// appearance depends on the camera orientation, draw(cameraPosition) 
   /// should also be implemented.  An example of this is when double Bonds 
   /// are drawn (see BondLayer.h).
   ///
   /// GLObjects can also be selected and have transparency.  Transparency is
   /// implemented at this level to allow alpha sorting of all GLObjects to
   /// determine their draw order.
   class GLObject : public Base {

      Q_OBJECT

      friend class IQmol::ManipulatedFrameSetConstraint;

      public: 
         /// Note the constructor does not allow a QObject parent.  This
         /// means the destruction of GLObjects must be taken care of 
         /// explicitly and not through the Qt mechanism.
         explicit GLObject(QString const& text = QString()) : Base(text), 
            m_selected(false), m_alpha(1.0) { 
            setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
          }

         virtual ~GLObject() {  }

		 /// This is the key function for GLObjects and must be implemented
		 /// for the object to be able to be drawn on the GL canvas.  The
         /// cameraPosition can be just a dummy argument, but it allows for 
         /// drawing of objects that need to face the camera.
         virtual void draw(qglviewer::Vec const& cameraPosition) = 0;

		 /// Reimplement this for complex objects that require simplified
		 /// drawing when moving to maintain interactive frame rates.
         virtual void drawFast(qglviewer::Vec const& cameraPosition) {
            draw(cameraPosition);
         } 

         /// Reimplement this for objects that should appear different when 
         /// selected.  This is called after the object has been drawn and
         /// in most cases this function just needs to draw a transparent 
         /// 'halo' around the object.
         virtual void drawSelected(qglviewer::Vec const& cameraPosition) {
            draw(cameraPosition);
         }

         virtual void select() { m_selected = true; }
         virtual void deselect() { m_selected = false; }
         bool isSelected() const { return m_selected; }

         virtual void setAlpha(double alpha) { m_alpha = alpha; }
         virtual double getAlpha() const { return m_alpha; }

         qglviewer::Vec getPosition() { return m_frame.position(); }
         qglviewer::Quaternion getOrientation() { return m_frame.orientation(); }

		 /// Basic implmentation of an alpha sort so transparent objects can be
		 /// drawn last so that they are not eclipsed.
         static bool AlphaSort(GLObject* a, GLObject* b) { 
            return (a->getAlpha() > b->getAlpha()); 
         }

      public Q_SLOTS:
         virtual void setPosition(qglviewer::Vec const& pos) { 
            m_frame.setPosition(pos); 
         }

         virtual void setOrientation(qglviewer::Quaternion const& orient) { 
            m_frame.setOrientation(orient); 
         }

      protected:
         qglviewer::Frame m_frame;
         bool   m_selected;
         double m_alpha;   
   };

} // end namespace Layer

typedef QList<Layer::GLObject*> GLObjectList;

} // end namespace IQmol


#endif
