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

#include "ViewerModel.h"
#include "BuildHandler.h"
#include "MoleculeLayer.h"
#include "UndoCommands.h"
#include "QMsgBox.h"


using namespace qglviewer;

namespace IQmol {
namespace Handler {

void Build::mousePressEvent(QMouseEvent* e) 
{
   m_viewer->clearSelection();
   m_viewer->setSelectionHighlighting(false);
   m_buildObjects.clear();
   m_manipulateOnly = true;
   m_beginAtom = 0;
   m_molecule  = 0;
   m_button    = e->button();

   // Check to see if we clicked on an atom
   m_selectionMode = None;
   m_viewer->QGLViewer::select(e);
   for (int i = 0; i < m_viewer->m_selectedObjects.size(); ++i) {
      m_beginAtom = qobject_cast<Layer::Atom*>(m_viewer->m_selectedObjects[i]);
      if (m_beginAtom) break;
   }

   // Determine the parent molecule
   if (m_beginAtom == 0) {
      m_molecule = m_viewer->m_viewerModel.activeMolecule();
   }else {
      MoleculeList parents(m_beginAtom->findLayers<Layer::Molecule>(Layer::Parents));
      if (parents.isEmpty()) {
         m_molecule = m_viewer->m_viewerModel.activeMolecule();
      }else {
         m_molecule = parents.first();
      }
   }

   if (!m_molecule) {
      QMsgBox::warning(0, "IQmol", "No molecule available.\n"
          "Please select one in the Model View");
      return;
   }

   switch (m_button) {
      case Qt::LeftButton:
         leftMousePressEvent(e);
         break;
      case Qt::RightButton:
         rightMousePressEvent(e);
         break;
      default:
         e->ignore();
         break;
   }
}


void Build::mouseMoveEvent(QMouseEvent* e) 
{
   if (!m_molecule || m_manipulateOnly) {
      e->ignore();
      return;
   }

   if (m_button == Qt::LeftButton) {
      leftMouseMoveEvent(e);
   }else if (m_button == Qt::RightButton) {
      rightMouseMoveEvent(e);
   }else if (m_button == Qt::MidButton) {
      e->ignore();
   }
}


void Build::mouseReleaseEvent(QMouseEvent* e) 
{
   if (!m_molecule || m_manipulateOnly) {
      e->ignore();
      return;
   }

   if (e->button() == Qt::LeftButton) {
      leftMouseReleaseEvent(e);
   }else if (e->button() == Qt::RightButton) {
      rightMouseReleaseEvent(e);
   }else if (m_button == Qt::MidButton) {
      e->ignore();
   }

   m_viewer->clearSelection();
   m_buildObjects.clear();
   m_viewer->setSelectionHighlighting(true);
   m_manipulateOnly = true;
   m_molecule  = 0;
   m_beginAtom = 0;
   m_viewer->updateGL();
   e->accept();
}


} } // end namespace IQmol::Handler
