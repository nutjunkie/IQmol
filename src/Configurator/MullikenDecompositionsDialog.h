#ifndef IQMOL_CONFIGURATOR_MULLIKENDECOMPOSITIONSDIALOG_H
#define IQMOL_CONFIGURATOR_MULLIKENDECOMPOSITIONSDIALOG_H
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

#include "ui_MullikenDecompositionsDialog.h"
#include "QGLViewer/vec.h"
#include "Density.h"
#include "ShellList.h"


namespace IQmol {

   class MullikenDecompositionsDialog : public QDialog {

      Q_OBJECT

      public:
         MullikenDecompositionsDialog(Data::ShellList const& shellList, 
            Data::Density const& density, QWidget* parent = 0);

         void compute();

      Q_SIGNALS:
         void mullikenDecompositionsAvailable(Matrix const&);
      private:
         void fillTable(Matrix const& M);
         Ui::MullikenDecompositionsDialog m_dialog; 
         Data::ShellList m_shellList;
         Data::Density   m_density;
   };


} // end namespace IQmol

#endif
