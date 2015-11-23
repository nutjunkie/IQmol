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

#include "SymmetryToleranceDialog.h"
#include "Preferences.h"


namespace IQmol {

SymmetryToleranceDialog::SymmetryToleranceDialog(QWidget* parent, double value) 
 : QDialog(parent), m_value(value)
{
   m_symmetryToleranceDialog.setupUi(this);
   m_symmetryToleranceDialog.toleranceSlider->setValue(100*m_value);
}


void SymmetryToleranceDialog::on_toleranceSlider_valueChanged(int ival)
{
   m_value = ival/100.0;
   QString s(QString::number(m_value, 'f', 2));
   m_symmetryToleranceDialog.symmetryToleranceLabel->setText(s);
}


void SymmetryToleranceDialog::on_resetButton_clicked(bool)
{
   m_symmetryToleranceDialog.toleranceSlider->setValue(100*Preferences::SymmetryTolerance());
}

} // end namespace IQmol
