#ifndef IQMOL_BUILDHANDLER_H
#define IQMOL_BUILDHANDLER_H
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

#include "BaseHandler.h"


namespace qglviewer {
   class Vec;
}

namespace IQmol {

namespace Layer {
   class Atom;
   class Bond;
   class Molecule;
   class GLObject;
}

namespace Handler {

   /// The build handler controls how the mouse actions are interpreted when in
   /// the Build ViewerMode.  
   class Build : public Base {

      public: 
         Build(Viewer* viewer) : Base(viewer) { }
         Cursors::Type cursorType() const { return Cursors::Build; }

         void mousePressEvent(QMouseEvent *);
         void mouseMoveEvent(QMouseEvent *);
         void mouseReleaseEvent(QMouseEvent *);

      private:
         void leftMousePressEvent(QMouseEvent *e);
         void rightMousePressEvent(QMouseEvent *e);
         void leftMouseMoveEvent(QMouseEvent *e);
         void rightMouseMoveEvent(QMouseEvent *e);
         void leftMouseReleaseEvent(QMouseEvent *e);
         void rightMouseReleaseEvent(QMouseEvent *e);

         Qt::MouseButton m_button;
         Layer::Bond* m_bond;
         Layer::Atom *m_existingAtom, *m_beginAtom, *m_endAtom;
         Layer::Molecule* m_molecule;
         Layer::GLObject* m_deleteTarget;
         bool m_buildIsActive;
   };

} } // end namespace IQmol::Handler

#endif
