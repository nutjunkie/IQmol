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
#include "ServerConfiguration.h"


namespace IQmol {
namespace Process {

SshFileDialog::SshFileDialog(ServerConfiguration* configuration, QWidget* parent) 
 : QDialog(parent), m_configuration(configuration)
{
   m_dialog.setupUi(this);

   QString publicKeyFile  = m_configuration->value(ServerConfiguration::PublicKeyFile);
   QString privateKeyFile = m_configuration->value(ServerConfiguration::PrivateKeyFile);
   QString knownHostsFile = m_configuration->value(ServerConfiguration::KnownHostsFile);

   if (publicKeyFile.isEmpty())  publicKeyFile  = Preferences::SSHPublicIdentityFile();
   if (privateKeyFile.isEmpty()) privateKeyFile = Preferences::SSHPrivateIdentityFile();
   if (knownHostsFile.isEmpty()) knownHostsFile = Preferences::SSHKnownHostsFile();

   m_dialog.publicKey->setText(publicKeyFile);
   m_dialog.privateKey->setText(privateKeyFile);
   m_dialog.knownHosts->setText(knownHostsFile);

   connect(this, SIGNAL(accepted()), this, SLOT(updatePreferences()));
}


void SshFileDialog::updatePreferences()
{
   QString publicKeyFile(m_dialog.publicKey->text());
   QString privateKeyFile(m_dialog.privateKey->text());
   QString knownHostsFile(m_dialog.knownHosts->text());

    m_configuration->setValue(ServerConfiguration::PublicKeyFile, publicKeyFile);
    m_configuration->setValue(ServerConfiguration::PrivateKeyFile, privateKeyFile);
    m_configuration->setValue(ServerConfiguration::KnownHostsFile, knownHostsFile);

    Preferences::SSHPublicIdentityFile(publicKeyFile);
    Preferences::SSHPrivateIdentityFile(privateKeyFile);
    Preferences::SSHKnownHostsFile(knownHostsFile);
}


}} // end namespace IQmol::Process
