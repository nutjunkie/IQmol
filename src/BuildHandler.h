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
#include "GLObjectLayer.h"


namespace IQmol {

namespace Layer {
   class Atom;
   class Molecule;
}

namespace Handler {

   /// This is the base class for all the building handlers
   class Build : public Base {

      public: 
         enum Mode { Atom, Group, EFP, Molecule };
         Build(Viewer* viewer) : Base(viewer)  { }
         Cursors::Type  cursorType() const { return Cursors::Build; }
         GLObjectList buildObjects() const { return m_buildObjects; }

         void setBuildFile(QString const& filePath) { m_filePath = filePath; } 
         void mousePressEvent(QMouseEvent*);
         void mouseMoveEvent(QMouseEvent*);
         void mouseReleaseEvent(QMouseEvent*);

      protected:
         virtual void leftMousePressEvent(QMouseEvent*)    = 0;
         virtual void leftMouseMoveEvent(QMouseEvent*)     = 0;
         virtual void leftMouseReleaseEvent(QMouseEvent*)  = 0;
         virtual void rightMousePressEvent(QMouseEvent*)   = 0;
         virtual void rightMouseMoveEvent(QMouseEvent*)    = 0;
         virtual void rightMouseReleaseEvent(QMouseEvent*) = 0;

         GLObjectList m_buildObjects;
         QString m_filePath;
         Layer::Atom* m_beginAtom;
         Layer::Molecule* m_molecule;
         Qt::MouseButton m_button;
         // Only set this if we are manipulating (i.e. no build action);
         bool m_manipulateOnly;
   };

} } // end namespace IQmol::Handler

#endif
