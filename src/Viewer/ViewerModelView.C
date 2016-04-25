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

#include "ViewerModelView.h"
#include "MoleculeLayer.h"
#include "AtomLayer.h"
#include "QVariantPointer.h"
#include <QContextMenuEvent>
#include <QModelIndex>
#include <QMenu>

#include <QDebug>


namespace IQmol {

ViewerModelView::ViewerModelView(QWidget* parent) : QTreeView(parent)
{ 
   setDragDropMode(QAbstractItemView::DropOnly); 
   setDropIndicatorShown(true);
   setIndentation(15);
   setSelectionMode(QAbstractItemView::ExtendedSelection);
   setEditTriggers( QAbstractItemView::EditKeyPressed);
   setExpandsOnDoubleClick(false);
}


void ViewerModelView::contextMenuEvent(QContextMenuEvent*)
{
   QItemSelectionModel* selectModel(selectionModel());
   QModelIndexList selection(selectModel->selectedRows());

   Layer::Base* base;
   QMenu menu(this);

   if (selection.size() == 1) {
      
      QModelIndex target(selection.first());
      base = QVariantPointer<Layer::Base>::toPointer(target.data(Qt::UserRole+1));
	  // Not sure if this is required, but QVariantPointer is not guaranteed to
	  // return a valid Layer::Base pointer.
      if ( (base = qobject_cast<Layer::Base*>(base)) ) {
         QList<QAction*> actions(base->getActions());
         QList<QAction*>::iterator iter;
         for (iter = actions.begin(); iter != actions.end(); ++iter) {
             menu.addAction(*iter);
         }
      }

   }else {
      // Custom actions when more than one Layer is selected, eg merge molecules
   }

   if (!menu.isEmpty()) menu.exec(QCursor::pos());
}

} // end namespace IQmol
