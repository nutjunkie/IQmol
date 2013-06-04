#ifndef IQMOL_BUILDFUNCTIONALGROUPHANDLER_H
#define IQMOL_BUILDFUNCTIONALGROUPHANDLER_H
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

#include "BuildHandler.h"
#include "GroupLayer.h"


namespace IQmol {

namespace Layer {
   class Atom;
   class Bond;
   class Group;
}

namespace Handler {

   /// The build handler controls how the mouse actions are interpreted when in
   /// the Build ViewerMode.  
   class BuildFunctionalGroup : public Build {

      public: 
         BuildFunctionalGroup(Viewer* viewer) : Build(viewer) { }

      private:
         void leftMousePressEvent(QMouseEvent *e);
         void leftMouseMoveEvent(QMouseEvent *e);
         void leftMouseReleaseEvent(QMouseEvent *e);

         void rightMousePressEvent(QMouseEvent *e);
         void rightMouseMoveEvent(QMouseEvent *e);
         void rightMouseReleaseEvent(QMouseEvent *e);

         void initPointers();

         Layer::Atom*  m_existingAtom;
         Layer::Bond*  m_bond;
         Layer::Group* m_group;
   };

} } // end namespace IQmol::Handler

#endif
