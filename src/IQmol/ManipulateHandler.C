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

#include "Viewer.h"
#include "QGLViewer/manipulatedFrame.h"


namespace IQmol {
namespace Handler {

void Manipulate::mousePressEvent(QMouseEvent* e) 
{
   m_viewer->setMouseBinding(Qt::LeftButton, QGLViewer::CAMERA, QGLViewer::ROTATE);
   m_viewer->setMouseBinding(Qt::RightButton, QGLViewer::CAMERA, QGLViewer::TRANSLATE);

   m_viewer->manipulatedFrame()->setSpinningSensitivity(100.0);
   if (e->button() == Qt::LeftButton) {
      m_viewer->setCursor(Cursors::ClosedHand);
   }else if (e->button() == Qt::RightButton) {
      m_viewer->setCursor(Cursors::Translate);
   }else if (e->button() == Qt::MidButton) {
      m_viewer->setCursor(Cursors::Zoom);
   }
   e->ignore();
}


void Manipulate::mouseReleaseEvent(QMouseEvent* e) 
{
   m_viewer->setCursor(Cursors::OpenHand);
   e->ignore();
}

} } // end namespace IQmol::Handler
