#ifndef IQMOL_MANIPULATESELECTIONHANDLER_H
#define IQMOL_MANIPULATESELECTIONHANDLER_H
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

#include "BaseHandler.h"
#include "GLObjectLayer.h"


namespace IQmol {

namespace Command {
   class MoveObjects;
}

namespace Handler {
   /// Bindings for manipulate selection mode.  As with the ManipulateHandler, 
   /// most behaviour is passed onto the QGLViewer, here we mainly take care 
   /// of the cursor changes.
   class ManipulateSelection : public Base {

      public: 
         ManipulateSelection(Viewer* viewer) : Base(viewer) { }
         Cursors::Type cursorType() const { return Cursors::OpenHandSelection; }

         void mousePressEvent(QMouseEvent *);
         void mouseMoveEvent(QMouseEvent *);
         void mouseReleaseEvent(QMouseEvent *);
         void wheelEvent(QWheelEvent *);

      private:
         void reset();
         bool m_moved;
         GLObjectList m_objectsMoved;
         Command::MoveObjects* m_cmd;
   };

} } // end namespace IQmol::Handler

#endif
