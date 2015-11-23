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

#include "ReindexAtomsHandler.h"
#include "MoleculeLayer.h"
#include "Viewer.h"


namespace IQmol {
namespace Handler {

void ReindexAtoms::reset(Layer::Molecule* molecule)
{
   m_molecule = molecule;
   m_viewer->clearSelection();
}


void ReindexAtoms::mousePressEvent(QMouseEvent* e) 
{
   m_viewer->setSelectionHighlighting(false);

   if (e->button() == Qt::LeftButton) {
      m_selectionMode = AddClick;
   }else if (e->button() == Qt::RightButton) {
      m_selectionMode = RemoveClick;
   }else {
      m_selectionMode = ToggleClick;
   }

   m_viewer->QGLViewer::select(e);

   // Manipulate if we don't select anything
   if (m_viewer->selectionHits() == 0) {
      e->ignore();
      return;
   }

   GLObjectList selection(m_viewer->m_selectedObjects);
   GLObjectList::iterator iter;
   int index(1);
   Layer::Atom* atom;

   for (iter = selection.begin(); iter != selection.end(); ++iter) {
       if ( (atom = qobject_cast<Layer::Atom*>(*iter)) ) {
          atom->setReorderIndex(index++);
       }
   }

   m_viewer->updateGL();
   e->accept();
}


void ReindexAtoms::keyReleaseEvent(QKeyEvent* e)
{
   // We only accept the new order if the return key is pressed
   if ( e->key() == Qt::Key_Return ) {
      GLObjectList objects(m_viewer->m_objects);
      GLObjectList::iterator iter;
      Layer::Atom* atom;

	  // Zero out the unselected indices so updateAtomOrder() knows which ones
	  // have not been reorderd.
      int zero(0);
      for (iter = objects.begin(); iter != objects.end(); ++iter) {
          if ((*iter)->isSelected()) { 
             (*iter)->deselect();
          }else if ( (atom = qobject_cast<Layer::Atom*>(*iter)) ) {
             atom->setReorderIndex(zero);
          }
      }

	  // We must clear the selection before reordering the atoms, otherwise the
	  // selection model gets confused.
      m_viewer->clearSelection();
      m_molecule->updateAtomOrder();
   }else {
      m_viewer->clearSelection();
   }

   m_viewer->setActiveViewerMode(Viewer::Manipulate);
   m_viewer->setSelectionHighlighting(true);
   m_viewer->setLabelType(Layer::Atom::Index);
   m_viewer->displayMessage("");
   e->accept();
}

} } // end namespace IQmol::Handler
