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

#include "Cursors.h"
#include <QImage>
#include <QBitmap>
#include <QPixmap>
#include <QCursor>

#include <QtDebug>


namespace IQmol {

Cursors::Cursors() {
   m_cursors.append(new QCursor(Qt::ArrowCursor));
   m_cursors.append(load("open_hand"));
   m_cursors.append(load("open_hand_select"));
   m_cursors.append(load("closed_hand"));
   m_cursors.append(load("closed_hand_select"));
   m_cursors.append(load("build"));
   m_cursors.append(load("rotate"));
   m_cursors.append(load("select"));
   m_cursors.append(load("translate"));
   m_cursors.append(load("translate_selection"));
   m_cursors.append(load("zoom"));
}


Cursors::~Cursors() {
   QList<QCursor*>::iterator iter;
   for (iter = m_cursors.begin(); iter != m_cursors.end(); ++iter) {
       if (*iter) delete (*iter);
   }
}


QCursor* Cursors::load(QString const& name) {
   QString file(":/resources/cursors/");
   file += name + ".png";

   QImage image;
   QCursor* cursor(0);

   if (image.load(file)) {
      QPixmap pixmap( QPixmap::fromImage(image) );
      QBitmap mask( QBitmap::fromImage(image.createAlphaMask()) );
      pixmap.setMask(mask);
      cursor = new QCursor(pixmap, -1, -1);
   }else {
      qDebug() << "Failed to load cursor image file:" << file;
   }

   return cursor;
} 

} // end namespace IQmol
