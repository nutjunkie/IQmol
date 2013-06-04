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

#include "BuildMoleculeFragmentHandler.h"
#include "Viewer.h"
#include "UndoCommands.h"


using namespace qglviewer;

namespace IQmol {
namespace Handler {

void BuildMoleculeFragment::leftMousePressEvent(QMouseEvent* e) 
{
   // We can't stick molecule fragments on an existing atom
   if (m_beginAtom) return;
   addMolecule(e);
   m_viewer->setMouseBinding(Qt::LeftButton, QGLViewer::FRAME, QGLViewer::ROTATE);
   m_viewer->startManipulation(e);
   e->ignore();
}


void BuildMoleculeFragment::rightMousePressEvent(QMouseEvent* e) 
{
   if (m_beginAtom) return;
   m_viewer->setMouseBinding(Qt::RightButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
   e->ignore();
}


void BuildMoleculeFragment::addMolecule(QMouseEvent* e)
{
   Layer::Group* molecule = new Layer::Group;
   molecule->loadFromFile(m_filePath);
   Vec v(m_viewer->worldCoordinatesOf(e));
   Vec c(m_viewer->camera()->position());
   molecule->setOrientation(Quaternion(Vec(0.0,0.0,1.0), c));
   molecule->setPosition(v);

   QList<Layer::Primitive*> primitives;
   primitives << molecule->ungroup();
   Command::EditPrimitives* cmd(new Command::EditPrimitives("Add molecule", m_molecule));
   cmd->add(primitives);
   m_viewer->postCommand(cmd);

   PrimitiveList::iterator iter;
   for (iter = primitives.begin(); iter != primitives.end(); ++ iter) {
       m_viewer->addToSelection(*iter);
   }
   m_manipulateOnly = false;
}


void BuildMoleculeFragment::leftMouseMoveEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildMoleculeFragment::rightMouseMoveEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildMoleculeFragment::leftMouseReleaseEvent(QMouseEvent*) 
{
   m_viewer->setMouseBinding(Qt::LeftButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
}


void BuildMoleculeFragment::rightMouseReleaseEvent(QMouseEvent*) 
{
   m_viewer->setMouseBinding(Qt::RightButton, QGLViewer::CAMERA, QGLViewer::TRANSLATE);
}

} } // end namespace IQmol::Handler
