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

#include "SshFileDialog.h"
#include "Preferences.h"


namespace IQmol {

SshFileDialog::SshFileDialog(QWidget* parent) : QDialog(parent)
{
   m_dialog.setupUi(this);
   m_dialog.knownHosts->setText(Preferences::SSHKnownHostsFile());
   m_dialog.publicKey->setText(Preferences::SSHPublicIdentityFile());
   m_dialog.privateKey->setText(Preferences::SSHPrivateIdentityFile());
   connect(this, SIGNAL(accepted()), this, SLOT(updatePreferences()));
}


void SshFileDialog::updatePreferences()
{
    Preferences::SSHKnownHostsFile(m_dialog.knownHosts->text());
    Preferences::SSHPublicIdentityFile(m_dialog.publicKey->text());
    Preferences::SSHPrivateIdentityFile(m_dialog.privateKey->text());
}


} // end namespace IQmol
