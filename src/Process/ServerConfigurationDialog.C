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

#include "ServerConfigurationDialog.h"
#include "QueueOptionsDialog.h"
#include "QueueResourcesList.h"
#include "SshConnection.h"
#include "SshReply.h"
#include "HttpConnection.h"
#include "HttpReply.h"
#include "SshFileDialog.h"
#include "QMsgBox.h"
#include "YamlNode.h"
#include "ParseFile.h"
#include <QFileDialog>



namespace IQmol {
namespace Process {

ServerConfigurationDialog::ServerConfigurationDialog(ServerConfiguration& configuration, 
   QWidget* parent) : QDialog(parent), m_tested(false), m_setDefaults(false),
   m_originalConfiguration(configuration)
{
   m_dialog.setupUi(this);
   m_currentConfiguration = m_originalConfiguration;

   copyFrom(m_originalConfiguration);

   // Hide this temporarily as it causes the server to not appear in the list
   m_dialog.testConnectionButton->hide();

   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
}



void ServerConfigurationDialog::updateQueueSystemsCombo(
   Network::ConnectionT const connection)
{
   QComboBox* qs(m_dialog.queueSystem);
   QString currentText(qs->currentText());
   qs->clear();

   switch (connection) {
      case Network::HTTP:
      case Network::HTTPS:
         qs->addItem(ServerConfiguration::toString(ServerConfiguration::Web));
         break;
      case Network::Local:
      case Network::SSH:
      case Network::SFTP:
         qs->addItem(ServerConfiguration::toString(ServerConfiguration::Basic));
         qs->addItem(ServerConfiguration::toString(ServerConfiguration::PBS));
         qs->addItem(ServerConfiguration::toString(ServerConfiguration::SGE));
         qs->addItem(ServerConfiguration::toString(ServerConfiguration::SLURM));
         break;
   }

   int index(0);
   for (int i = 0; i < qs->count(); ++i) {
       if (qs->itemText(i) == currentText) {
          index = i;  break;
       }
   }
   qDebug()<< "setting current index to " << index;
   qs->setCurrentIndex(index);
   qDebug()<< "current text now" << qs->currentText();
}



void ServerConfigurationDialog::updateAuthenticationCombo(
   Network::ConnectionT const connection)
{
   QComboBox* auth(m_dialog.authentication);
   QString currentText(auth->currentText());
   auth->clear();

   switch (connection) {

      case Network::Local:
      case Network::HTTP:
         auth->addItem(Network::ToString(Network::Anonymous)); 
         // This is insecure and only here for testing
         auth->addItem(Network::ToString(Network::Password)); 
         break;

      case Network::HTTPS:
         auth->addItem(Network::ToString(Network::Anonymous)); 
         auth->addItem(Network::ToString(Network::Password)); 
         break;

      case Network::SSH:
      case Network::SFTP:
         auth->addItem(Network::ToString(Network::Anonymous)); 
         auth->addItem(Network::ToString(Network::Agent)); 
         auth->addItem(Network::ToString(Network::HostBased)); 
         auth->addItem(Network::ToString(Network::KeyboardInteractive));
         auth->addItem(Network::ToString(Network::Password)); 
         auth->addItem(Network::ToString(Network::PublicKey)); 
   }
    
   int index(0);
   for (int i = 0; i < auth->count(); ++i) {
       if (auth->itemText(i) == currentText) {
          index = i;  break;
       }
   }

   auth->setCurrentIndex(index);
}



void ServerConfigurationDialog::on_localRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_dialog.remoteHostGroupBox->setEnabled(false);
   m_dialog.configureSshButton->setEnabled(false);

   updateQueueSystemsCombo(Network::Local);
   updateAuthenticationCombo(Network::Local);

   if (setDefaults()) {
      qDebug() << "Setting defaults for Local";
      m_currentConfiguration.setDefaults(Network::Local);
      copyFrom(m_currentConfiguration);
   }
}


void ServerConfigurationDialog::on_sshRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_dialog.remoteHostGroupBox->setEnabled(true);
   m_dialog.configureSshButton->setEnabled(true);
   m_dialog.authentication->setEnabled(true);
   m_dialog.userName->setEnabled(true);
   m_dialog.workingDirectory->setEnabled(true);
   m_dialog.workingDirectoryLabel->setEnabled(true);

   updateQueueSystemsCombo(Network::SSH);
   updateAuthenticationCombo(Network::SSH);

   if (setDefaults()) {
      qDebug() << "Setting defaults for SSH";
      m_currentConfiguration.setDefaults(Network::SSH);
      copyFrom(m_currentConfiguration);
   }
}


void ServerConfigurationDialog::on_sftpRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_dialog.remoteHostGroupBox->setEnabled(true);
   m_dialog.configureSshButton->setEnabled(true);
   m_dialog.authentication->setEnabled(true);
   m_dialog.userName->setEnabled(true);
   m_dialog.workingDirectory->setEnabled(true);
   m_dialog.workingDirectoryLabel->setEnabled(true);

   updateQueueSystemsCombo(Network::SFTP);
   updateAuthenticationCombo(Network::SFTP);

   if (setDefaults()) {
      qDebug() << "Setting defaults for SFTP";
      m_currentConfiguration.setDefaults(Network::SFTP);
      copyFrom(m_currentConfiguration);
   }
}


void ServerConfigurationDialog::on_httpRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_dialog.remoteHostGroupBox->setEnabled(true);
   m_dialog.configureSshButton->setEnabled(false);
   m_dialog.authentication->setEnabled(true);
   m_dialog.userName->setEnabled(false);
   m_dialog.workingDirectory->setEnabled(false);
   m_dialog.workingDirectoryLabel->setEnabled(false);

   updateQueueSystemsCombo(Network::HTTP);
   updateAuthenticationCombo(Network::HTTP);

   if (setDefaults()) {
      qDebug() << "Setting defaults for HTTP";
      m_currentConfiguration.setDefaults(Network::HTTP);
      copyFrom(m_currentConfiguration);
   }
}


void ServerConfigurationDialog::on_httpsRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_dialog.remoteHostGroupBox->setEnabled(true);
   m_dialog.configureSshButton->setEnabled(false);
   m_dialog.authentication->setEnabled(true);
   m_dialog.userName->setEnabled(true);
   m_dialog.workingDirectory->setEnabled(false);
   m_dialog.workingDirectoryLabel->setEnabled(false);

   updateQueueSystemsCombo(Network::HTTPS);
   updateAuthenticationCombo(Network::HTTPS);

   if (setDefaults()) {
      qDebug() << "Setting defaults for HTTPS";
      m_currentConfiguration.setDefaults(Network::HTTPS);
      copyFrom(m_currentConfiguration);
   }
}



void ServerConfigurationDialog::on_configureSshButton_clicked(bool)
{
   SshFileDialog dialog(&m_currentConfiguration, this); 
   dialog.exec();
}


void ServerConfigurationDialog::on_configureQueueButton_clicked(bool)
{
   copyTo(&m_currentConfiguration);
   QueueOptionsDialog dialog(&m_currentConfiguration, this);
   dialog.exec();
}


void ServerConfigurationDialog::on_queueSystem_currentIndexChanged(QString const& queue)
{
   if (setDefaults()) {
      ServerConfiguration::QueueSystemT queueT(ServerConfiguration::toQueueSystemT(queue));
      m_currentConfiguration.setDefaults(queueT);
   }
}


void ServerConfigurationDialog::on_authentication_currentIndexChanged(QString const& auth)
{
   if (m_dialog.queueSystem->currentText() == 
      ServerConfiguration::toString(ServerConfiguration::Web)) {
      bool pw(auth == Network::ToString(Network::Password));
      m_dialog.authenticationPort->setEnabled(pw);
      m_dialog.authenticationPortLabel->setEnabled(pw);
      m_dialog.userName->setEnabled(pw);
      m_dialog.userNameLabel->setEnabled(pw);
   }else {
      m_dialog.authenticationPort->setEnabled(false);
      m_dialog.authenticationPortLabel->setEnabled(false);
   }
}



void ServerConfigurationDialog::copyFrom(ServerConfiguration const& config)
{
   setDefaults(false);
   switch (config.connection()) {
      case Network::Local:  m_dialog.localRadioButton->setChecked(true);  break;
      case Network::SSH:    m_dialog.sshRadioButton->setChecked(true);    break;
      case Network::SFTP:   m_dialog.sftpRadioButton->setChecked(true);   break;
      case Network::HTTP:   m_dialog.httpRadioButton->setChecked(true);   break;
      case Network::HTTPS:  m_dialog.httpsRadioButton->setChecked(true);  break;
   }

   m_dialog.queueSystem->setCurrentText(
      ServerConfiguration::toString(config.queueSystem()));

   m_dialog.serverName->setText(config.value(ServerConfiguration::ServerName));
   m_dialog.hostAddress->setText(config.value(ServerConfiguration::HostAddress));
   m_dialog.port->setValue(config.port());

   m_dialog.authentication->setCurrentText(
      Network::ToString(config.authentication()));
   m_dialog.authenticationPort->setValue(config.authenticationPort());

   m_dialog.userName->setText(config.value(ServerConfiguration::UserName));

   m_dialog.workingDirectory->setText(
      config.value(ServerConfiguration::WorkingDirectory));

   setDefaults(true);
   m_tested = false;
}


bool ServerConfigurationDialog::copyTo(ServerConfiguration* config)
{
   Network::AuthenticationT authentication = 
      Network::ToAuthenticationT(m_dialog.authentication->currentText());

   Network::ConnectionT connection(Network::Local);

   if (m_dialog.sshRadioButton->isChecked()) {
      connection = Network::SSH;
   }else if (m_dialog.sftpRadioButton->isChecked()) {
      connection = Network::SFTP;
   }else if (m_dialog.httpRadioButton->isChecked()) {
      connection = Network::HTTP;
   }else if (m_dialog.httpsRadioButton->isChecked()) {
      connection = Network::HTTPS;
   }

   // Sanity checks
   if (m_dialog.serverName->text().trimmed().isEmpty()) {
      QMsgBox::warning(this, "IQmol", "Server name not set");
      return false;
   }

   if (connection != Network::Local) {
      if (m_dialog.hostAddress->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "Host address not set");
         return false;
      }
   }

   if (connection == Network::SSH || connection == Network::SFTP || 
       authentication == Network::Password) {
 
      if (m_dialog.userName->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "User name must be set");
         return false;
      }

      if (m_dialog.workingDirectory->text().contains("~")) {
         QString msg("Use of ~ shortcut in working directory may not work\n");
         msg += "Suggest using $HOME or full path";
         QMsgBox::warning(this, "IQmol", msg);
         return false;
      }
   }
   // end sanity checks

   config->setValue(ServerConfiguration::ServerName, m_dialog.serverName->text());
   config->setValue(ServerConfiguration::Connection, connection);

   QString queue(m_dialog.queueSystem->currentText());
   config->setValue(ServerConfiguration::QueueSystem, 
      ServerConfiguration::toQueueSystemT(queue));
   config->setValue(ServerConfiguration::HostAddress, "localhost");

   if (connection == Network::Local) return true;

   config->setValue(ServerConfiguration::HostAddress, 
      m_dialog.hostAddress->text());

   config->setValue(ServerConfiguration::Port, 
      m_dialog.port->value());

   config->setValue(ServerConfiguration::Authentication, 
      Network::ToAuthenticationT(m_dialog.authentication->currentText()));

   config->setValue(ServerConfiguration::AuthenticationPort, 
      m_dialog.authenticationPort->value());

   config->setValue(ServerConfiguration::UserName, 
      m_dialog.userName->text());

   QString dirPath(m_dialog.workingDirectory->text());
   while (dirPath.endsWith("/")) { dirPath.chop(1); }
   while (dirPath.endsWith("\\")) { dirPath.chop(1); }
   
   config->setValue(ServerConfiguration::WorkingDirectory, dirPath);

   return true;
}


void ServerConfigurationDialog::on_testConnectionButton_clicked(bool) 
{ 
   testConnection(); 
}


bool ServerConfigurationDialog::testConnection()
{
   bool okay(false);

   if (!copyTo(&m_currentConfiguration)) return okay;

   switch (m_currentConfiguration.connection()) {
      case Network::Local:
         QMsgBox::information(this, "IQmol", "Local connection just fine");
         break;
      case Network::SSH:
      case Network::SFTP:
         okay = testSshConnection(m_currentConfiguration);
         break;
      case Network::HTTP:
      case Network::HTTPS:
         okay = testHttpConnection(m_currentConfiguration);
         break;
   }

   if (okay) {
      QMsgBox::information(0, "IQmol", "Connection successful");
      m_tested = true;
   }

   return okay;
}
   

bool ServerConfigurationDialog::testSshConnection(ServerConfiguration const& configuration)
{
   bool okay(false);

   try {
      QString hostAddress(configuration.value(ServerConfiguration::HostAddress));
      QString userName(configuration.value(ServerConfiguration::UserName));
      Network::AuthenticationT authentication(configuration.authentication());

      QString publicKeyFile(configuration.value(ServerConfiguration::KnownHostsFile));
      QString privateKeyFile(configuration.value(ServerConfiguration::PrivateKeyFile));
      QString knownHostsFile(configuration.value(ServerConfiguration::PublicKeyFile));

      int port(configuration.port());

      Network::SshConnection ssh(hostAddress, port, publicKeyFile, privateKeyFile, 
         knownHostsFile);
      ssh.open();
      ssh.authenticate(authentication, userName);
      QLOG_TRACE() << "Authentication successful";

      QEventLoop loop;
      Network::Reply* reply(ssh.execute("ls"));
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = (reply->status() == Network::Reply::Finished);
      if (okay) {
         QLOG_DEBUG() << "----------------------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "----------------------------";
      }else {
         QString msg("Connection failed:\n");
         msg += reply->message();
         QMsgBox::warning(this, "IQmol", msg);
      }
      delete reply;

   }catch (Network::AuthenticationCancelled& err) {
      // don't do anything

   }catch (Network::AuthenticationError& err) {
      QMsgBox::warning(0, "IQmol", "Invalid username or password");

   }catch (Exception& err) {
      okay = false;
      QMsgBox::warning(0, "IQmol", err.what());
   }

   return okay;
}


bool ServerConfigurationDialog::testHttpConnection(ServerConfiguration const& configuration)
{
   bool okay(false);

   try {
      QString hostAddress(configuration.value(ServerConfiguration::HostAddress));
      int port(configuration.port());

      Network::HttpConnection http(hostAddress, port);
      http.open();

      Network::Reply* reply(http.get("index.html"));

      QEventLoop loop;
      QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
      loop.exec();

      okay = reply->status() == Network::Reply::Finished;
      if (okay) {
         QLOG_DEBUG() << "----------------------------";
         QLOG_DEBUG() << reply->message();
         QLOG_DEBUG() << "----------------------------";
      }else {
         QString msg("Connection failed:\n");
         msg += reply->message();
         QMsgBox::warning(this, "IQmol", msg);
      }
      delete reply;

   }catch (Network::AuthenticationError& err) {
      QMsgBox::warning(this, "IQmol", "Invalid username or password");

   }catch (Exception& err) {
      QMsgBox::warning(this, "IQmol", err.what());
   }

   return okay;
}


void ServerConfigurationDialog::verify()
{
   if (!copyTo(&m_currentConfiguration)) return;

/* don't bother with the testing at the moment, it is just annoying
   if (!m_tested && !m_dialog.localRadioButton->isChecked()) {
      QString msg("Would you like to try connecting to the server?");
      if (QMsgBox::question(this, "IQmol", msg,
         QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes) {
         if (!testConnection()) return;
      }
   }
*/

   m_originalConfiguration = m_currentConfiguration;

   accept();
}


// This should be refactored to use the ServerRegistry::loadFromFile() code.
void ServerConfigurationDialog::on_loadButton_clicked(bool)
{
   QString filePath(QDir::homePath()); 
   filePath = QFileDialog::getOpenFileName(this, tr("Open Server Configuration"),
      filePath, tr("Configuration Files (*.cfg)"));

   if (filePath.isEmpty()) return;
   
/* -------------------------------------------------------- *\
   QFile file(filePath);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QLOG_ERROR() << "File does not exist";
      return;
   }

   QTextStream in(&file);
   QueueResourcesList qrl;
   qrl.fromPbsQueueInfoString(in.readAll());
   qDebug() << "Server configuration read in from file:";
   qDebug() << qrl.toQVariantList();

   return;
\* -------------------------------------------------------- */

   try {
      Parser::ParseFile parser(filePath);
      parser.start();
      parser.wait();

      QStringList errors(parser.errors());
      if (!errors.isEmpty()) {
         QMsgBox::warning(this, "IQmol", errors.join("\n"));
      }

      Data::Bank& bank(parser.data());
      QList<Data::YamlNode*> yaml(bank.findData<Data::YamlNode>());
      if (yaml.first()) {
         yaml.first()->dump();
         m_currentConfiguration = ServerConfiguration(*(yaml.first()));
         setDefaults(false);
         copyFrom(m_currentConfiguration);
         setDefaults(true);
      }

   } catch (YAML::Exception& err) {
      QString msg(QString::fromStdString(err.what()));
      QMsgBox::warning(this, "IQmol", msg);
   }
}
       

void ServerConfigurationDialog::on_exportButton_clicked(bool)
{
   if (!copyTo(&m_currentConfiguration)) return;

   QString filePath(QDir::homePath()); 
   filePath += "/iqmol_server.cfg";

   filePath = QFileDialog::getSaveFileName(this, tr("Save File"), filePath, 
       tr("Configuration Files (*.cfg)"));

   if (filePath.isEmpty()) return;
   Data::YamlNode node(m_currentConfiguration.toYamlNode());
   if (!node.saveToFile(filePath)) {
      QMsgBox::warning(this, "IQmol", "Failed to export server configuration");
   }
}


void ServerConfigurationDialog::setDefaults(bool const tf)
{
   m_setDefaults = tf;
}


bool ServerConfigurationDialog::setDefaults() const
{
   return m_setDefaults;
}

} } // end namespace IQmol::Process
