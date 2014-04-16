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

#include "ServerDialog.h"
#include "ServerOptionsDialog.h"
#include "Server.h"
#include "ServerTask.h"
#include "SshFileDialog.h"
#include "ServerRegistry.h"
#include "PasswordVault.h"
#include "QMsgBox.h"


namespace IQmol {

ServerDialog::ServerDialog(QWidget* parent, Server* server) : QDialog(parent), 
   m_server(server), m_tested(false), m_accepted(false), m_blockSignals(false),
   m_closeAfterTest(false), m_task(0)
{
   m_dialog.setupUi(this);

   // We use a temporary Server in case the user cancels the edit.
   ServerRegistry& serverRegistry(ServerRegistry::instance());
   m_tmpServer = serverRegistry.newServer();
   if (!m_server) m_server = m_tmpServer;

   m_dialog.name->setText(m_server->m_name);
   copyFromServer(m_server);
   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
}


ServerDialog::~ServerDialog()
{
   if (m_server != m_tmpServer || !m_accepted) {
      ServerRegistry::instance().remove(m_tmpServer);  
   }
   OverwriteString(m_password);
}


void ServerDialog::on_localRadioButton_toggled(bool)
{
   if (m_dialog.localRadioButton->isChecked()) {
      m_dialog.typeCombo->clear();
      m_dialog.typeCombo->addItem("Basic");
      m_dialog.typeCombo->addItem("PBS");
      m_dialog.typeCombo->addItem("SGE");

      m_dialog.remoteLoginGroupBox->setEnabled(false);
      if (m_blockSignals) return;
      setCurrentDefaults(m_tmpServer);
      copyFromServer(m_tmpServer);
   }
}


void ServerDialog::on_remoteRadioButton_toggled(bool)
{
   if (m_dialog.remoteRadioButton->isChecked()) {
      m_dialog.typeCombo->clear();
      m_dialog.typeCombo->addItem("Basic");
      m_dialog.typeCombo->addItem("PBS");
      m_dialog.typeCombo->addItem("SGE");
      //m_dialog.typeCombo->addItem("Web");
      m_dialog.remoteLoginGroupBox->setEnabled(true);
      if (m_blockSignals) return;
      setCurrentDefaults(m_tmpServer);
      copyFromServer(m_tmpServer);
   }
}


void ServerDialog::on_typeCombo_currentIndexChanged(int index)
{
   Server::Type type = (Server::Type)index;
   bool web(type == Server::HTTP);
   //m_dialog.qcLabel->setEnabled(!web);
   //m_dialog.qcEnvironment->setEnabled(!web);
   m_dialog.workingDirectory->setEnabled(!web);
   m_dialog.workingDirectoryLabel->setEnabled(!web);
   m_dialog.authentication->setEnabled(!web);
   m_dialog.authenticationLabel->setEnabled(!web);
   m_dialog.configureSsh->setEnabled(!web);

   if (m_blockSignals) return;
   setCurrentDefaults(m_tmpServer);
   copyFromServer(m_tmpServer);
}


void ServerDialog::on_configureTypeButton_clicked(bool)
{
   ServerOptionsDialog dialog(this, m_server);
   dialog.exec();
}


void ServerDialog::setCurrentDefaults(Server* server)
{
   Server::Type type = (Server::Type)m_dialog.typeCombo->currentIndex();
   Server::Host host;
   if (type == Server::HTTP) {
      host = Server::Web;
   }else {
      host = m_dialog.localRadioButton->isChecked() ? Server::Local : Server::Remote;
   }
qDebug() << "Setting defaults for server:" << server << "host:" << host << " type: " << type;
   server->setDefaults(host, type);
}


void ServerDialog::copyFromServer(Server* server)
{
   // We use this flag to block the signals triggered by the Host
   // radio buttons and Type combo box to avoid recursion.
   m_blockSignals = true;

   if (server->m_host == Server::Local) {
      m_dialog.localRadioButton->setChecked(true);
      m_dialog.remoteLoginGroupBox->setEnabled(false);
   }else {
      m_dialog.remoteRadioButton->setChecked(true);
      m_dialog.remoteLoginGroupBox->setEnabled(true);
   }

   m_dialog.typeCombo->setCurrentIndex(server->m_type);
   //m_dialog.qcEnvironment->setText(server->m_qchemEnvironment);
   m_dialog.hostAddress->setText(server->m_hostAddress);
   m_dialog.userName->setText(server->m_userName);
   m_dialog.authentication->setCurrentIndex(server->m_authentication-1);
   m_dialog.port->setValue(server->m_port);
   m_dialog.workingDirectory->setText(server->m_workingDirectory);

   m_blockSignals = false;
}



bool ServerDialog::copyToServer(Server* server)
{
   if (!server) {
      QMsgBox::warning(this, "IQmol", "Invalid Server");
      return false;
   }

   if (m_dialog.name->text().trimmed().isEmpty()) {
      QMsgBox::warning(this, "IQmol", "Server name must be set");
      return false;
   }

   //if (m_dialog.qcEnvironment->text().trimmed().isEmpty()) {
   //   QMsgBox::warning(this, "IQmol", "QChem environment variable must be set");
   //   return false;
   //}

   switch (m_dialog.typeCombo->currentIndex()) {
      case Server::HTTP:
         server->m_type = Server::HTTP;
         server->m_host = Server::Web;
         break;
     
      default:
         server->m_type = (Server::Type)m_dialog.typeCombo->currentIndex();
         server->m_host = 
            m_dialog.localRadioButton->isChecked() ? Server::Local : Server::Remote;
         break;
   }

   if (server->m_host == Server::Remote) {
      if (m_dialog.hostAddress->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "Host address must be set");
         return false;
      }
      if (m_dialog.userName->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "User name must be set");
         return false;
      }
   }

   server->m_name             = m_dialog.name->text().trimmed();
   //server->m_qchemEnvironment = m_dialog.qcEnvironment->text().trimmed();

   if (server->m_host == Server::Local) return true;

   Server::Authentication authentication = 
      (Server::Authentication)(m_dialog.authentication->currentIndex()+1);

   if (server->m_host == Server::Remote && 
       authentication == Server::Vault && m_password.isEmpty()) {
      m_password = PasswordVault::instance().getServerPassword(server->m_name);
      if (m_password.isEmpty()) {
         QMsgBox::warning(this, "IQmol", "Password must be set for Vault authentication");
         return false;
      }
   }

   // If any of these options change we should make sure the server is
   // disconnected before over-writing them.
   if (server->m_hostAddress    != m_dialog.hostAddress->text().trimmed() ||
       server->m_userName       != m_dialog.userName->text().trimmed()    ||
       server->m_authentication != authentication               ||
       server->m_port           != m_dialog.port->value()      ) server->disconnectServer();

   server->m_hostAddress      = m_dialog.hostAddress->text().trimmed(); 
   server->m_userName         = m_dialog.userName->text().trimmed();
   server->m_authentication   = authentication;
   server->m_port             = m_dialog.port->value();
   server->m_workingDirectory = m_dialog.workingDirectory->text().trimmed();

   if (!m_password.isEmpty() && server->m_authentication == Server::Vault) {
      PasswordVault::instance().storeServerPassword(server->m_name, m_password);
   }
   return true;
}


void ServerDialog::on_authentication_currentIndexChanged(QString const& text)
{
   m_dialog.setPassword->setEnabled((text == "Password Vault") && !m_dialog.name->text().isEmpty());
}


void ServerDialog::on_setPassword_clicked(bool)
{
   if (PasswordVault::instance().unlock()) {
      QString msg("Set password for ");
      msg += m_dialog.name->text();
      SetPasswordDialog dialog(msg);
      if (dialog.exec() == QDialog::Accepted) m_password = dialog.password();
   }
}


void ServerDialog::on_configureSsh_clicked(bool)
{
  SshFileDialog ssh(this); 
  ssh.exec();
}


void ServerDialog::on_testConfiguration_clicked(bool) 
{ 
   m_closeAfterTest = false;
   testConfiguration(); 
}


void ServerDialog::testConfiguration()
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


void ServerDialog::configurationTested()
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


void ServerDialog::verify()
{
   if (!copyToServer(m_tmpServer)) return;
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


} // end namespace IQmol
