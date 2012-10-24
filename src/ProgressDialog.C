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

#include "ProgressDialog.h"


namespace IQmol {

ProgressDialog::ProgressDialog(QString const& title, QWidget* parent) : QDialog(parent)
{
   m_progressDialog.setupUi(this);
   setWindowTitle(title); 
// TODO this needs to be connected to an abort signal, which actually does something
   m_progressDialog.abortButton->hide();
}

void ProgressDialog::setRange(int min, int max)
{
   m_progressDialog.progressBar->setRange(min, max);
}

void ProgressDialog::setInfo(QString const& label)
{
   m_progressDialog.progressBar->reset();
   m_progressDialog.label->setText(label);
}


void ProgressDialog::updateProgress(double progress)
{
   m_progressDialog.progressBar->setValue(int(100.0*progress));
}


void ProgressDialog::setIndefinite(bool tf)
{
}


} // end namespace IQmol
