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

#include "Viewer.h"
#include "MoleculeLayer.h"
#include "BuildFunctionalGroupHandler.h"
#include "UndoCommands.h"
#include "QGLViewer/camera.h"
#include "QGLViewer/vec.h"
#include "QMsgBox.h"
#include "QsLog.h"


using namespace qglviewer;

namespace IQmol {
namespace Handler {


void BuildFunctionalGroup::initPointers()
{
   m_existingAtom = 0;
   m_group        = 0;
   m_bond         = 0;
}


void BuildFunctionalGroup::leftMousePressEvent(QMouseEvent* e) 
{
   // We must have an inital atom to attach the group to
   if (m_beginAtom == 0) {
      e->ignore();
      return;
   }

   initPointers();
   m_manipulateOnly = false;

   m_viewer->setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, 
      QGLViewer::TRANSLATE);
   m_viewer->updateGL();
   e->accept();
}



void BuildFunctionalGroup::rightMousePressEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildFunctionalGroup::leftMouseMoveEvent(QMouseEvent* e) 
{
   m_viewer->clearSelection();
   m_viewer->QGLViewer::select(e);

   // Remove from the selection an object we may have created during the
   // current mouse action.
   m_viewer->m_selectedObjects.removeAll(m_group);
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
	  // We are still sitting on the first atom, so we get rid of anything we
	  // may have added
      if (m_group) {
         m_buildObjects.removeAll(m_group);
         delete m_group;
         m_group = 0;
      }

      if (m_bond) {
         m_buildObjects.removeAll(m_bond);
         delete m_bond;
         m_bond = 0;
      }

   }else if (m_existingAtom) {
	  // We have landed on an existing atom, so we do not need either the
	  // m_group or m_bond
      if (m_group) {
         m_buildObjects.removeAll(m_group);
         delete m_group;
         m_group = 0;
      }

      if (m_bond) {
         m_buildObjects.removeAll(m_bond);
         delete m_bond;
         m_bond = 0;
      }

      if (m_existingAtom != m_beginAtom) {
		 // We have dragged from one atom to another, if a bond already exists
		 // between these atoms then we should increment the bond order (this
		 // is done on the mouseReleaseEvent), otherwise we create a new bond
		 // between the atoms just like the BuildAtomHandler.
         if (!m_molecule->getBond(m_existingAtom, m_beginAtom)) {
            MoleculeList molecules(m_existingAtom->findLayers<Layer::Molecule>(Layer::Parents));
            if (!molecules.isEmpty() && molecules.first() == m_molecule) {
               m_bond = m_molecule->createBond(m_beginAtom, m_existingAtom);
               m_buildObjects.append(m_bond);
            }
         }
      }

   }else {
      // We are floating out in space, create a 
      // new group if we haven't already done so
      if (!m_group) {
         // take care of any bond we may have formed
         if (m_bond) {
            m_buildObjects.removeAll(m_bond);
            delete m_bond;
            m_bond = 0;
         }

         //Vec pos(m_viewer->worldCoordinatesOf(e, m_beginAtom->getPosition()));
         m_group = new Layer::Group;
         m_group->loadFromFile(m_filePath);
         Layer::Atom* rootAtom(m_group->rootAtom());
         if (rootAtom) {
            m_bond = m_molecule->createBond(m_beginAtom, rootAtom);
            m_buildObjects.append(m_group);
            m_buildObjects.append(m_bond);
         }
      }
	  // Move the group to where the mouse is and orient it so that the z axis
	  // lies along the new bond.  Note that this relies on the geometries for
	  // the functional groups having the root atom at the origin and the
	  // vacant valence along the -z direction.
      Vec position(m_viewer->worldCoordinatesOf(e, m_beginAtom->getPosition()));
      Quaternion orientation(Vec(0.0, 0.0, 1.0), m_beginAtom->getPosition()-position);
      m_group->setPosition(position);
      m_group->setOrientation(orientation);
   }

   m_viewer->updateGL();
   e->accept();
}


void BuildFunctionalGroup::rightMouseMoveEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildFunctionalGroup::leftMouseReleaseEvent(QMouseEvent* e) 
{
   m_viewer->setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::CAMERA, 
      QGLViewer::ROTATE);
   if (!m_molecule) return;
   Q_UNUSED(e);

   Layer::PrimitiveList added;
   Layer::PrimitiveList removed;

   if (m_bond == 0) {
      if (m_beginAtom != 0 ) {

         if (m_beginAtom == m_existingAtom || m_existingAtom == 0) {
            // Effectively a left click on a single atom, change the atom to
            // the root atom of the group
            if (!m_group) {
               m_group = new Layer::Group;
               m_group->loadFromFile(m_filePath);
            }

            Layer::Atom* rootAtom(m_group->rootAtom());
            if (!rootAtom) {
               QLOG_DEBUG() << "No root atom found for functional group:" << m_filePath;
               return;
            }

            Quaternion orientation(Vec(0.0, 0.0, 1.0), m_molecule->getBuildAxis(m_beginAtom));
            m_group->setPosition(m_beginAtom->getPosition());
            m_group->setOrientation(orientation);

            removed << m_beginAtom;
            added   << m_group->ungroup();

            BondList bonds(m_molecule->getBonds(m_beginAtom));
            BondList::iterator bond;
            for (bond = bonds.begin(); bond != bonds.end(); ++bond) {
                if ((*bond)->beginAtom() == m_beginAtom) {
                   Layer::Bond* b = new Layer::Bond((*bond)->endAtom(), rootAtom);
                   removed << *bond;
                   added << b;
                }else if ((*bond)->endAtom() == m_beginAtom) {
                   Layer::Bond* b = new Layer::Bond((*bond)->beginAtom(), rootAtom);
                   removed << *bond;
                   added << b;
                }
            }
 
         }else if (m_existingAtom != 0 && m_group == 0) {
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

   } else {
	  // A bond has been created, so the user must have clicked on m_beginAtom
	  // and dragged out into space.
      if (!m_group) {
         QLOG_DEBUG() << "No functional group found in builder:" << m_filePath;
         return;
         
      }
      added.append(m_bond);
      added << m_group->ungroup();
   }

   if (!added.isEmpty()) {
       QString msg("Add functional group");
       Command::EditPrimitives* cmd(new Command::EditPrimitives(msg, m_molecule));
       cmd->remove(removed).add(added);
       m_viewer->postCommand(cmd);
    }
}


void BuildFunctionalGroup::rightMouseReleaseEvent(QMouseEvent* e) 
{
   e->ignore();
}

} } // end namespace IQmol::Handler
