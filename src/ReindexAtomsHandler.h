#ifndef IQMOL_REINDEXATOMSHANDLER_H
#define IQMOL_REINDEXATOMSHANDLER_H
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


namespace IQmol {

namespace Layer {
   class Molecule;
}

namespace Handler {

   /// Implements the event bindings for a mode that allows the user to reorder
   /// the indices of the atoms.
   class ReindexAtoms : public Base {

      public: 
         ReindexAtoms(Viewer* viewer) : Base(viewer) { }
         Cursors::Type cursorType() const { return Cursors::Arrow; }

         void reset(Layer::Molecule*);
         void mousePressEvent(QMouseEvent* e);
         void keyReleaseEvent(QKeyEvent* e);

      private:
         Layer::Molecule* m_molecule;
   };

} } // end namespace IQmol::Handler

#endif
