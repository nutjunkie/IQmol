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

#include "CustomPlot.h"
#include "Preferences.h"
#include <QDebug>
#include <QContextMenuEvent>
#include <QFileInfo>


namespace IQmol {

void CustomPlot::contextMenuEvent(QContextMenuEvent* event)
{
   QMenu *menu(new QMenu(this));
   menu->setAttribute(Qt::WA_DeleteOnClose);
   menu->addAction("Save Image As", this, SLOT(saveAs()));
   menu->popup(mapToGlobal(event->pos()));
}


void CustomPlot::saveAs()
{
   QFileInfo info(Preferences::LastFileAccessed());
   info.setFile(info.dir(), info.completeBaseName());

   while (1) {
      QString filter(tr("PNG") + " (*.png)");
      QStringList extensions;
      extensions << filter
                 << tr("JPG") + " (*.jpg)"
                 << tr("PDF") + " (*.pdf)";

      QString fileName(QFileDialog::getSaveFileName(this, tr("Save File"), 
         info.filePath(), extensions.join(";;"), &filter));

      if (fileName.isEmpty()) {
         // This will occur if the user cancels the action.
         return;
      }else {
         QRegExp rx("\\*(\\..+)\\)");
         if (rx.indexIn(filter) > 0) { 
            filter = rx.cap(1);
            if (!fileName.endsWith(filter, Qt::CaseInsensitive)) {
               fileName += filter;
            }    
         }    

         QSize dim(size());
         int upscale(2);

         if (filter == ".pdf") {
            qDebug() << "Saving with filter " << fileName << filter;
            bool noCosmeticPen = true;
            savePdf(fileName, noCosmeticPen, dim.width(), dim.height());
         }else if (filter == ".png") {
            qDebug() << "Saving with filter " << fileName << filter;
            savePng(fileName, dim.width(), dim.height(), upscale);
         }else if (filter == ".jpg") {
            qDebug() << "Saving with filter " << fileName << filter;
            saveJpg(fileName, dim.width(), dim.height(), upscale);
         }

         Preferences::LastFileAccessed(fileName);
         break;
      }    
   } 
}

} // end namespace IQmol
