#ifndef IQMOL_BASEHANDLER_H
#define IQMOL_BASEHANDLER_H
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

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include "Cursors.h"


namespace IQmol {

class Viewer;

namespace Handler {

   /// Base class for interpreting input events from a QGLViewer widget.   To
   /// use these handlers in a class derived from QGLViewer, events need to be
   /// sent to the handler:
   /// 
   ///    Handler::Base* handler;
   ///    void mousePressEvent(QMouseEvent* e) {
   ///       if (handler)  handler->mousePressEvent(e);
   ///    }
   /// 
   /// The advantage of doing things this way is that the handlers can be swapped
   /// in and out to give rise to different behaviors depending on the mode.  It also
   /// factors out the handling code into the derived handlers.
   class Base {

      public: 
         Base(Viewer* viewer) : m_viewer(viewer) { }
         virtual ~Base() { }
         virtual Cursors::Type cursorType() const { return Cursors::OpenHand; }

         virtual void mousePressEvent(QMouseEvent* e) {
            e->ignore();
         }

         virtual void mouseMoveEvent(QMouseEvent* e) {
            e->ignore();
         }

         virtual void mouseReleaseEvent(QMouseEvent* e) {
            e->ignore();
         }

         virtual void mouseDoubleClickEvent(QMouseEvent* e) {
            e->ignore();
         }
   
         virtual void wheelEvent(QWheelEvent* e) {
            e->ignore();
         }

         virtual void keyPressEvent(QKeyEvent* e) {
            e->ignore();
         }

         virtual void keyReleaseEvent(QKeyEvent* e) {
            e->ignore();
         }

      protected:
         Viewer* m_viewer;
   };

} } // end namespace IQmol::Handler

#endif
