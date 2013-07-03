/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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

#include "BuildEFPFragmentHandler.h"
#include "EFPFragmentLayer.h"
#include "Viewer.h"
#include "MoleculeLayer.h"
#include "UndoCommands.h"


using namespace qglviewer;

namespace IQmol {
namespace Handler {


void BuildEFPFragment::leftMousePressEvent(QMouseEvent* e) 
{
   // We can't stick EFP fragments on an existing atom
   if (m_beginAtom) return;
   addFragment(e);
   m_viewer->setMouseBinding(Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
   m_viewer->startManipulation(e);
   e->ignore();
}


void BuildEFPFragment::rightMousePressEvent(QMouseEvent* e) 
{
   if (m_beginAtom) return;
   m_viewer->setMouseBinding(Qt::RightButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
   e->ignore();
}


void BuildEFPFragment::addFragment(QMouseEvent* e)
{
   Layer::EFPFragment* efp = new Layer::EFPFragment(m_filePath);
   Vec v(m_viewer->worldCoordinatesOf(e));
   Vec c(m_viewer->camera()->position());
   efp->setOrientation(Quaternion(Vec(0.0,0.0,1.0), c));
   efp->setPosition(v);

   QList<Layer::Primitive*> primitives; 
   primitives << efp;
   Command::EditPrimitives* cmd(new Command::EditPrimitives("Add EFP fragment", m_molecule));
   cmd->add(primitives);
   m_viewer->postCommand(cmd);

   m_viewer->addToSelection(efp);
   m_manipulateOnly = false;
}


void BuildEFPFragment::leftMouseMoveEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildEFPFragment::rightMouseMoveEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildEFPFragment::leftMouseReleaseEvent(QMouseEvent*) 
{
   m_viewer->setMouseBinding(Qt::LeftButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
}


void BuildEFPFragment::rightMouseReleaseEvent(QMouseEvent*) 
{
   m_viewer->setMouseBinding(Qt::RightButton, QGLViewer::CAMERA, QGLViewer::TRANSLATE);
}

} } // end namespace IQmol::Handler
