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

#include "ServerConfigurationDialog.h"
#include "ServerConfiguration.h"
#include "SSHFileConfigurator.h"
#include "ServerRegistry.h"

#include "Server.h"
#include "QMsgBox.h"


namespace IQmol {
namespace Network {

ServerConfigurationDialog::ServerConfigurationDialog(ServerConfiguration* config, QWidget* parent)
   : QDialog(parent), m_originalConfiguration(config), m_tested(false),
   m_task(0), m_http(false)
{
   m_dialog.setupUi(this);
   m_currentConfiguration = new ServerConfiguration(*config);
   copyFrom(m_currentConfiguration);
}


void ServerConfigurationDialog::on_localRadioButton_toggled(bool tf)
{
   m_dialog.remoteLoginGroupBox->setEnabled(false);

   if (tf) {
      m_dialog.remoteLoginGroupBox->setEnabled(false);
      setHttpProtocol(false);
   }
}


void ServerConfigurationDialog::on_sshRadioButton_toggled(bool tf)
{
   m_dialog.configureSshButton->setVisible(tf);

   if (tf) {
      m_dialog.remoteLoginGroupBox->setEnabled(true);
      setHttpProtocol(false);
   }
}


void ServerConfigurationDialog::on_httpRadioButton_toggled(bool tf)
{
   m_dialog.clearCookieButton->setVisible(tf);
   if (tf)  {
      m_dialog.remoteLoginGroupBox->setEnabled(true);
      setHttpProtocol(true);
   }
}


void ServerConfigurationDialog::setHttpProtocol(bool http)
{
   if (http == m_http) return;
   m_http = http;

   // Queue System
   m_dialog.queueCombo->clear();
   if (http) {
      m_dialog.queueCombo->addItem(ToString(Web));
   }else {
      m_dialog.queueCombo->addItem(ToString(Basic));
      m_dialog.queueCombo->addItem(ToString(PBS));
      m_dialog.queueCombo->addItem(ToString(SGE));
   }

   // Authentication
   m_dialog.authenticationCombo->clear();
   if (http) {
      m_dialog.authenticationCombo->addItem(ToString(Cookie));
   }else {
      m_dialog.authenticationCombo->addItem(ToString(Agent));
      m_dialog.authenticationCombo->addItem(ToString(PublicKey));
      m_dialog.authenticationCombo->addItem(ToString(HostBased));
      m_dialog.authenticationCombo->addItem(ToString(KeyboardInteractive));
      m_dialog.authenticationCombo->addItem(ToString(Prompt));
   }

   m_dialog.userName->setEnabled->(!m_http);
   m_dialog.userNameLabel->setEnabled->(!m_http);
   m_dialog.workingDirectory->setEnabled->(!m_http);
   m_dialog.workingDirectoryLabel->setEnabled->(!m_http);
}



void ServerConfigurationDialog::on_configureButton_clicked(bool)
{
}


void ServerConfigurationDialog::copyFrom(ServerConfiguration& config)
{
   // We use this flag to block the signals triggered by the Host
   // radio buttons and Type combo box to avoid recursion.
   m_blockSignals = true;

   m_dialog.name->setText(config.m_name); 

   if (config.m_protocol == Local) {
      m_dialog.localRadioButton->setChecked(true);
      on_localRadioButton_toggled(true);
   }else if (config.m_protocol == SSH) {
      m_dialog.sshRadioButton->setChecked(true);
      on_sshRadioButton_toggled(true);
   }else {
      m_dialog.httpRadioButton->setChecked(true);
      on_httpRadioButton_toggled(true);
   }

   m_dialog.queueCombo->setCurrentText(ToString(config.m_queueSystem));



   //m_dialog.qcEnvironment->setText(server->m_qchemEnvironment);
   m_dialog.hostAddress->setText(config.m_address);
   m_dialog.userName->setText(server->m_userName);
   m_dialog.authentication->setCurrentIndex(server->m_authentication-1);
   m_dialog.port->setValue(server->m_port);
   m_dialog.workingDirectory->setText(server->m_workingDirectory);

   m_blockSignals = false;
}



bool ServerConfigurationDialog::copyTo(ServerConfiguration& config)
{
   if (m_dialog.name->text().trimmed().isEmpty()) {
      QMsgBox::warning(this, "IQmol", "Server name not set");
      return false;
   }

   Protocol protocol(Local);
   if (m_dialog.sshRadioButton->isChecked()) {
      protocol = SSH;
   }else if (m_dialog.httpRadioButton->isChecked()) {
      protocol = HTTP;
   }

   if (protocol == SSH || protocol == HTTP) {
      if (m_dialog.hostAddress->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "Host address not set");
         return false;
      }
   }

   if (protocol == SSH) {
      if (m_dialog.userName->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "User name must be set");
         return false;
      }
   }

   // end sanity checks

   m_config.m_name        = m_dialog.name->text();
   m_config.m_protocol    = protocol;
   m_config.m_queueSystem = QueueSystemFromString(m_dialog->queueSystem->currentText());
   m_config.m_address     = "localhost";

   m_config.m_userName.clear();
   m_config.m_workingDirectory.clear();

   if (protocol == Local)  return true;

   m_config.m_address          = m_dialog->address->currentText();
   m_config.m_prot             = m_dialog->port->value();
   m_config.m_authentication   = AuthenticationFromString(m_dialog->authentication->currentText());
   m_config.m_userName         = m_dialog->userName->text();
   m_config.m_workingDirectory = m_dialog->workingDirectory->text();

   return true;
}


void ServerConfigurationDialog::on_configureSsh_clicked(bool)
{
  SSHFileConfigurator ssh(this); 
  ssh.exec();
}


void ServerConfigurationDialog::on_testConnection_clicked(bool) 
{ 
   m_closeAfterTest = false;
   testConnection(); 
}


void ServerConfigurationDialog::testConnection()
{
   if (m_task) {
      QMsgBox::information(this, "IQmol", "Connection test in progress");
      return;
   }

   m_tested = false;
   if (!copyToServer(m_tmpServer)) return;
   copyToServer(m_server);

   try {
      //qDebug() << "Is server connected?" << m_server->isConnected();
      if (m_server->connectServer()) {
         m_task = m_server->testConfiguration();
         connect(m_task, SIGNAL(finished()), this, SLOT(configurationTested()));
         qDebug() << "Starting Test thread";
         m_task->start();
      }else {
         throw Server::Exception("Failed to connect to server");
      }
   } catch (std::exception& err) {
      QString msg("Problem connecting to server:\n");
      msg += err.what();
      QMsgBox::warning(this, "IQmol", msg);
   }
}


void ServerConfigurationDialog::configurationTested()
{
   if (m_task) {
      QString errorMessage(m_task->errorMessage());
      QString message;

      if (errorMessage.isEmpty()) {
         message = "Connection successful";
         QMsgBox::information(this, "IQmol", message);
         m_tested = true;
      }else {
         message ="Problem connecting to server:\n"; 
         message += errorMessage;
         m_closeAfterTest = false;
         QMsgBox::warning(this, "IQmol", message);
      }

      m_task->deleteLater();
      m_task = 0;
   }

   if (m_closeAfterTest) {
      m_accepted = true;
      accept();
   }
}


void ServerConfigurationDialog::verify()
{
   if (!copyTo(m_tmpServer)) return;
   copyToServer(m_server);
   m_closeAfterTest = true;

   if (!m_tested && m_dialog.remoteRadioButton->isChecked()) {
      QString msg("Would you like to try connecting to the server?");
      if (QMsgBox::question(this, "IQmol", msg,
         QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes) {
         testConfiguration();
         return;
      }
   }

   m_accepted = true;
   accept();
}


} } // end namespace IQmol::Network
