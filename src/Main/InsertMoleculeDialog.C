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

#include "InsertMoleculeDialog.h"
#include <QtDebug>


namespace IQmol {

InsertMoleculeDialog::InsertMoleculeDialog(QWidget* parent) : QDialog(parent) 
{
   m_dialog.setupUi(this);
   connect(this, SIGNAL(accepted()), this, SLOT(okButton_clicked()));
}


void InsertMoleculeDialog::okButton_clicked()
{
   QString id(m_dialog.identifier->text());

   if (m_dialog.smiles->isChecked()) {
      id = "SMILES:" + id;
      insertMoleculeById(id);
   }else if (m_dialog.inchi->isChecked()) {
      id = "INCHI:" + id;
      insertMoleculeById(id);
   }
}

} // end namespace IQmol
