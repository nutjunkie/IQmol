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

#include "Viewer.h"


namespace IQmol {
namespace Handler {

void Select::mousePressEvent(QMouseEvent* e) 
{
   m_rectangle = QRect(e->pos(), e->pos());
   m_selectionMode = Add;
   e->accept();
}


void Select::mouseMoveEvent(QMouseEvent* e) 
{
   m_rectangle.setBottomRight(e->pos());
   m_viewer->updateGL();
   e->accept();
}


void Select::mouseReleaseEvent(QMouseEvent* e) 
{
   m_rectangle = m_rectangle.normalized();
   m_viewer->setSelectRegionWidth(m_rectangle.width());
   m_viewer->setSelectRegionHeight(m_rectangle.height());

   // Movement of less than 3 pixels is considered a click
   bool click((m_rectangle.width() < 3) && (m_rectangle.width() < 3));

   if (e->button() == Qt::LeftButton) {
      if (click) {
         m_selectionMode = AddClick;
      }else {
         m_selectionMode = Add;
      }
   }else if (e->button() == Qt::RightButton) {
      if (click) {
         m_selectionMode = RemoveClick;
      }else {
         m_selectionMode = Remove;
      }
   }else {
      if (click) {
         m_selectionMode = ToggleClick;
      }else {
         m_selectionMode = Toggle;
      }
   }
 
   m_viewer->QGLViewer::select(m_rectangle.center());
   m_selectionMode = None;
   m_rectangle.setSize(QSize(1,1));
   m_viewer->updateGL();
   e->accept();
}


void Select::mouseDoubleClickEvent(QMouseEvent* e)
{
   m_viewer->clearSelection();
   e->accept();
}


} } // end namespace IQmol::Handler
