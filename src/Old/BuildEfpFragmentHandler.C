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

#include "BuildEfpFragmentHandler.h"
#include "EfpFragmentLayer.h"
#include "EfpFragment.h"
#include "Viewer.h"
#include "MoleculeLayer.h"
#include "UndoCommands.h"
#include "QsLog.h"
#include <QFileInfo>

#include <QDebug>

using namespace qglviewer;

namespace IQmol {
namespace Handler {

void BuildEfpFragment::setBuildFile(QString const& filePath) 
{
   QFileInfo info(filePath);
   m_name = info.completeBaseName(); 
} 


void BuildEfpFragment::leftMousePressEvent(QMouseEvent* e) 
{
   // We can't stick EFP fragments on an existing atom
   if (m_beginAtom) return;
   addFragment(e);
   m_viewer->setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME, 
      QGLViewer::ROTATE);
   m_viewer->startManipulation(e);
   e->ignore();
}


void BuildEfpFragment::rightMousePressEvent(QMouseEvent* e) 
{
   if (m_beginAtom) return;
   m_viewer->setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::CAMERA, 
      QGLViewer::ROTATE);
   e->ignore();
}


void BuildEfpFragment::addFragment(QMouseEvent* e)
{
qDebug() << "Adding EFP fragment with name" << m_name;
   Data::EfpFragment* data(new Data::EfpFragment(m_name));
   if (!data) {
      QLOG_ERROR() << "Failed to create EFP fragment data in builder:" << m_name;
      return;
   }
   Layer::EfpFragment* efp = new Layer::EfpFragment(*data);

qDebug() << "EfpData to go to molecule  Data::Bank?";

   Vec v(m_viewer->worldCoordinatesOf(e));
   Vec c(m_viewer->camera()->position());
   efp->setOrientation(Quaternion(Vec(0.0, 0.0, 1.0), c));
   efp->setPosition(v);

   QList<Layer::Primitive*> primitives; 
   primitives << efp;
   Command::EditPrimitives* cmd(new Command::EditPrimitives("Add EFP fragment", m_molecule));
   cmd->add(primitives);
   m_viewer->postCommand(cmd);

   m_viewer->addToSelection(efp);
   m_manipulateOnly = false;
}


void BuildEfpFragment::leftMouseMoveEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildEfpFragment::rightMouseMoveEvent(QMouseEvent* e) 
{
   e->ignore();
}


void BuildEfpFragment::leftMouseReleaseEvent(QMouseEvent*) 
{
   m_viewer->setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::CAMERA, 
      QGLViewer::ROTATE);
}


void BuildEfpFragment::rightMouseReleaseEvent(QMouseEvent*) 
{
   m_viewer->setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::CAMERA, 
      QGLViewer::TRANSLATE);
}

} } // end namespace IQmol::Handler
