#ifndef IQMOL_SYMMETRYTOLERANCEDIALOG_H
#define IQMOL_SYMMETRYTOLERANCEDIALOG_H
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

#include "ui_SymmetryToleranceDialog.h"


namespace IQmol {

   /// A simple dialog that allows the user to adjust the tolerance value
   /// passed to the SymMol program for symmetrizing nuclear coordinates.
   class SymmetryToleranceDialog : public QDialog {

      Q_OBJECT

      public: 
         SymmetryToleranceDialog(QWidget* parent, double value);
         ~SymmetryToleranceDialog() { }

         double value() { return m_value; }

      Q_SIGNALS:
         void symmetrizeRequest(double const);

      private Q_SLOTS:
         void on_toleranceSlider_valueChanged(int);
         void on_resetButton_clicked(bool);
         void on_applyButton_clicked() { symmetrizeRequest(m_value); }

      private:
         Ui::SymmetryToleranceDialog m_symmetryToleranceDialog;
         double m_value;
   };

} // end namespace IQmol


#endif
