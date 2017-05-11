#ifndef IQMOL_GRID_BOUNDINGBOXDIALOG_H
#define IQMOL_GRID_BOUNDINGBOXDIALOG_H
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

#include "ui_BoundingBoxDialog.h"
#include "QGLViewer/vec.h"


namespace IQmol {

namespace Layer {
   class MolecularOrbitals;
}

   class BoundingBoxDialog : public QDialog {

      Q_OBJECT

      public:
         BoundingBoxDialog(qglviewer::Vec* min, qglviewer::Vec* max, QWidget* parent = 0);

      private Q_SLOTS:
         void copyToInput();
         void copyFromInput();

      private:
         Ui::BoundingBoxDialog m_dialog; 
         qglviewer::Vec* m_min;
         qglviewer::Vec* m_max;
   };


} // end namespace IQmol

#endif
