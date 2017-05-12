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
#include "MoleculeLayer.h"
#include "BuildAtomHandler.h"
#include "UndoCommands.h"
#include "QGLViewer/camera.h"
#include "QGLViewer/vec.h"

#include <QDebug>


using namespace qglviewer;

namespace IQmol {
namespace Handler {


void BuildAtom::initPointers()
{
   m_deleteTarget = 0;
   m_existingAtom = 0;
   m_endAtom      = 0;
   m_bond         = 0;
}


void BuildAtom::leftMousePressEvent(QMouseEvent* e) 
{
   int nObjects(m_viewer->m_objects.size());
   if ((m_beginAtom == 0) && 
       (nObjects > 0)     && 
       (e->modifiers() != Viewer::s_buildModifier)) {
      // Manipulating in Build mode
      e->ignore();
      return;
   }

   initPointers();
   m_manipulateOnly = false;

   if (m_beginAtom == 0) {
	  // We need to create the first atom, this only happens at the start of a
	  // build or the first atom of a fragment.  In either case, once we have
	  // created the atom we post the command and do not activate the build.
	  // This ensures only a single atom is created.
      Vec v(m_viewer->worldCoordinatesOf(e));
      m_beginAtom = m_molecule->createAtom(m_buildElement, v);
      m_buildObjects.append(m_beginAtom);
   }

   m_viewer->setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, 
      QGLViewer::TRANSLATE);
   m_viewer->updateGL();
   e->accept();
}


void BuildAtom::rightMousePressEvent(QMouseEvent* e) 
{
   int nObjects(m_viewer->m_objects.size());
   if ((m_beginAtom == 0) && 
       (nObjects > 0) && 
       (e->modifiers() != Viewer::s_buildModifier)) {
      // Manipulating in Build mode
      e->ignore();
      return;
   }

   initPointers();
   if (m_viewer->m_selectedObjects.isEmpty()) return;

   m_deleteTarget  = m_beginAtom;

   if (m_deleteTarget == 0) {
      // Should really check for a primitive
      m_deleteTarget = m_viewer->m_selectedObjects[0];
   }

   // Determine the parent molecule
   if (m_molecule == 0) {
      MoleculeList parents(m_deleteTarget->findLayers<Layer::Molecule>(Layer::Parents));
      if (parents.isEmpty()) {
         m_molecule = m_viewer->m_viewerModel.activeMolecule();
      }else {
         m_molecule = parents.first();
      }
   }

   if (m_deleteTarget) m_manipulateOnly = false;
   if (!m_molecule) qDebug() << "Molecule not set in buidler";
   e->accept();
}



void BuildAtom::leftMouseMoveEvent(QMouseEvent* e) 
{
   m_viewer->clearSelection();
   m_viewer->QGLViewer::select(e);

   // Remove from the selection any objects we may have created during 
   // the current mouse action.  (I think this is redundant now)
   m_viewer->m_selectedObjects.removeAll(m_endAtom);
   m_viewer->m_selectedObjects.removeAll(m_bond);

   // Any atom left over must be an existing one
   m_existingAtom = 0;
   for (int i = 0; i < m_viewer->m_selectedObjects.size(); ++i) {
      m_existingAtom = qobject_cast<Layer::Atom*>(m_viewer->m_selectedObjects[i]);
      if (m_existingAtom) break;
   }

   Vec startingMousePosition(m_beginAtom->getPosition());
   Vec currentMousePosition(m_viewer->worldCoordinatesOf(e, startingMousePosition));
   double shift((startingMousePosition-currentMousePosition).norm());

   if (shift < 0.2) {
      // We haven't moved far enough from where we started to create 
      // a second atom or bond.
      if (m_endAtom) {
         m_buildObjects.removeAll(m_endAtom);
         delete m_endAtom;
         m_endAtom = 0;
      }

      if (m_bond) {
         m_buildObjects.removeAll(m_bond);
         delete m_bond;
         m_bond = 0;
      }
   }else if (m_existingAtom ) {
	  // We have landed on an existing atom so we do not need either 
      // m_endAtom or m_bond;
      if (m_endAtom) {
         m_buildObjects.removeAll(m_endAtom);
         delete m_endAtom;
         m_endAtom = 0;
      }

      if (m_bond) {
         m_buildObjects.removeAll(m_bond);
         delete m_bond;
         m_bond = 0;
      }

      if (m_existingAtom != m_beginAtom) {
         if (!m_molecule->getBond(m_existingAtom, m_beginAtom)) {
            // bond does not already exist
            MoleculeList molecules(m_existingAtom->findLayers<Layer::Molecule>(Layer::Parents));
            if (!molecules.isEmpty() && molecules.first() == m_molecule) {
               m_bond = m_molecule->createBond(m_beginAtom, m_existingAtom);
               m_buildObjects.append(m_bond);
            }
         }
      }

   }else {
	  // We are floating out in space, create a 
      // new atom if we haven't already done so
      if (!m_endAtom) {
         // take care of any bond we may have formed
         if (m_bond) {
            m_buildObjects.removeAll(m_bond);
            delete m_bond;
            m_bond = 0;
         }

         Vec pos(m_viewer->worldCoordinatesOf(e, m_beginAtom->getPosition()));
         m_endAtom = m_molecule->createAtom(m_buildElement, pos);
         m_bond    = m_molecule->createBond(m_beginAtom, m_endAtom);
         m_buildObjects.append(m_endAtom);
         m_buildObjects.append(m_bond);
      }
      // Move the atom to where the mouse is
      m_endAtom->setPosition(m_viewer->worldCoordinatesOf(e, m_beginAtom->getPosition()));
   }

   m_viewer->updateGL();
   e->accept();
}


void BuildAtom::rightMouseMoveEvent(QMouseEvent* e) 
{
   if (!m_manipulateOnly) e->accept();
}


void BuildAtom::leftMouseReleaseEvent(QMouseEvent* e) 
{
   m_viewer->setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::CAMERA, 
      QGLViewer::ROTATE);
   if (!m_molecule) return;
   Q_UNUSED(e);

   if (m_bond == 0) {
      if (m_beginAtom != 0 ) {
         if (m_existingAtom == 0 || m_beginAtom == m_existingAtom) {
		    // Effectivly a left click on a single atom, change atom number to
            // current build element
            if (m_beginAtom->getAtomicNumber() != (int)m_buildElement) {
               Command::ChangeAtomType* cmd = new Command::ChangeAtomType(m_beginAtom);
               m_beginAtom->setAtomicNumber(m_buildElement);
               m_viewer->postCommand(cmd);
            }
            //m_viewer->updateLabels();
         }else if (m_existingAtom != 0 && m_endAtom == 0) {
		    // we have dragged from one existing atom to another, so we increase
            // the bond order.
            Layer::Bond* bond(m_molecule->getBond(m_beginAtom, m_existingAtom));
            if (bond) {
               Command::ChangeBondOrder* cmd = new Command::ChangeBondOrder(bond);
               bond->setOrder(bond->getOrder()+1);
               m_viewer->postCommand(cmd);
            }
         }
      }
   }

   Layer::PrimitiveList primitives;
   Layer::Primitive* primitive;
   for (int i = 0; i < m_buildObjects.size(); ++i) {
       primitive = qobject_cast<Layer::Primitive*>(m_buildObjects[i]);
       if (primitive) primitives.append(primitive);
   }

   // If the user clicks and releases after moving the cursor only slightly, it
   // is possible that two atoms have been created more or less on top of each
   // other.  This is not detected by the Viewer selection routine as the objects
   // in m_buildObjects do not participate in the selection.

   if (!primitives.isEmpty()) {
      QString msg("Add atoms/bonds");
      Command::EditPrimitives* cmd(new Command::EditPrimitives(msg, m_molecule));
      cmd->add(primitives);
      m_viewer->postCommand(cmd);
   }
}


void BuildAtom::rightMouseReleaseEvent(QMouseEvent* e) 
{
   if (!m_molecule) return;
   m_viewer->clearSelection();
   m_viewer->QGLViewer::select(e);

   if (m_viewer->m_selectedObjects.size() > 0) {
      // Only delete if we click and release on the same target
      if (m_deleteTarget == m_viewer->m_selectedObjects[0]) {
         Layer::Primitive* primitive(qobject_cast<Layer::Primitive*>(m_deleteTarget));
         if (primitive && m_molecule) {
            m_viewer->clearSelection();
            primitive->select();
            m_molecule->deleteSelection();
         }
      }
   }
}

} } // end namespace IQmol::Handler
