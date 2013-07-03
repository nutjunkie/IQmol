#ifndef IQMOL_GRIDINFODIALOG_H
#define IQMOL_GRIDINFODIALOG_H
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

#include "ui_GridInfoDialog.h"
#include "Grid.h"
#include <QPoint>


namespace IQmol {

namespace Layer {
   class Molecule;
}

   class GridInfoDialog : public QDialog {

      Q_OBJECT

      public:
         GridInfoDialog(GridList*, Layer::Molecule*, QWidget* parent = 0);

      Q_SIGNALS:
         void updated();  // to trigger a redraw

      private Q_SLOTS:
         void contextMenu(QPoint const&);
         void deleteGrid();
         void exportCubeFilePositive() { exportCubeFile(false); }
         void exportCubeFileNegative() { exportCubeFile(true); }

      private:
         void exportCubeFile(bool const invertSign);
        GridList* m_gridList;
        Layer::Molecule* m_molecule;
        GridList getSelectedGrids();
        Ui::GridInfoDialog m_dialog;
        void loadGridInfo();
   };


} // end namespace IQmol

#endif
