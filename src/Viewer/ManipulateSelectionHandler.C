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

#include "ManipulateSelectionHandler.h"
#include "UndoCommands.h"
#include "Viewer.h"


namespace IQmol {
namespace Handler {

void ManipulateSelection::mousePressEvent(QMouseEvent* e) 
{
   reset();

   switch (e->button()) {
      case Qt::LeftButton:
         m_viewer->setCursor(Cursors::ClosedHandSelection);
         m_objectsMoved = m_viewer->startManipulation(e);
         e->ignore();
         break;

      case Qt::RightButton:
         m_viewer->setCursor(Cursors::TranslateSelection);
         m_objectsMoved = m_viewer->startManipulation(e);
         e->ignore();
         break;

      case Qt::MidButton:
         e->accept();
         break;

      default:
         e->accept();
         break;
   }

   if (!m_objectsMoved.isEmpty()) {
      m_cmd = new Command::MoveObjects(m_objectsMoved);
   }
}


void ManipulateSelection::mouseMoveEvent(QMouseEvent* e) 
{
   m_moved = true;
   e->ignore();
}


void ManipulateSelection::mouseReleaseEvent(QMouseEvent* e) 
{
   m_viewer->setCursor(Cursors::OpenHandSelection);
   if (m_moved && m_cmd) {
      m_viewer->postCommand(m_cmd);
   }else if (m_cmd) {
      delete m_cmd;
   }
   reset();
   e->ignore();
}


void ManipulateSelection::wheelEvent(QWheelEvent* e) 
{
   e->accept();  // Makes no sense to zoom a selection
}


void ManipulateSelection::reset() 
{
   m_objectsMoved.clear();
   m_moved = false;
   m_cmd = 0;
}

} } // end namespace IQmol::Handler

