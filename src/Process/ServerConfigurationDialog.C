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
#include "QueueOptionsDialog.h"
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
namespace Process2 {

ServerConfigurationDialog::ServerConfigurationDialog(ServerConfiguration& configuration, 
   QWidget* parent) : QDialog(parent), m_tested(false), m_originalConfiguration(configuration)
{
   m_dialog.setupUi(this);
   init();
   m_currentConfiguration = new ServerConfiguration(m_originalConfiguration);
   copyFrom(m_originalConfiguration);

   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(verify()));
}


ServerConfigurationDialog::~ServerConfigurationDialog()
{
   delete m_currentConfiguration;
}


void ServerConfigurationDialog::init()
{
   m_dialog.authentication->clear(); 

   // Not pretty, the ordering of these is linked to the enum in Network/SshConnection.h
   m_dialog.authentication->addItem(
      ServerConfiguration::toString(Network::SshConnection::Agent)); 
   m_dialog.authentication->addItem(
      ServerConfiguration::toString(Network::SshConnection::HostBased)); 
   m_dialog.authentication->addItem(
      ServerConfiguration::toString(Network::SshConnection::KeyboardInteractive)); 
   m_dialog.authentication->addItem(
      ServerConfiguration::toString(Network::SshConnection::Password)); 
   m_dialog.authentication->addItem(
      ServerConfiguration::toString(Network::SshConnection::PublicKey)); 
}


void ServerConfigurationDialog::updateAllowedQueueSystems(bool httpOnly)
{
   m_dialog.queueSystem->clear();

   if (httpOnly) {
      m_dialog.queueSystem->addItem(
         ServerConfiguration::toString(ServerConfiguration::Web));
   }else {
      m_dialog.queueSystem->addItem(
         ServerConfiguration::toString(ServerConfiguration::Basic));
      m_dialog.queueSystem->addItem(
         ServerConfiguration::toString(ServerConfiguration::PBS));
      m_dialog.queueSystem->addItem(
         ServerConfiguration::toString(ServerConfiguration::SGE));
   }
   m_tested = false;
}



void ServerConfigurationDialog::on_localRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_currentConfiguration->setDefaults(ServerConfiguration::Local);
   copyFrom(*m_currentConfiguration);

   m_dialog.remoteHostGroupBox->setEnabled(false);
   m_dialog.configureConnectionButton->setEnabled(false);
   updateAllowedQueueSystems(false);
}


void ServerConfigurationDialog::on_sshRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_currentConfiguration->setDefaults(ServerConfiguration::SSH);
   copyFrom(*m_currentConfiguration);

   m_dialog.remoteHostGroupBox->setEnabled(true);
   m_dialog.configureConnectionButton->setEnabled(true);
   m_dialog.authentication->setEnabled(true);
   m_dialog.userName->setEnabled(true);
   m_dialog.workingDirectory->setEnabled(true);
   updateAllowedQueueSystems(false);
}


void ServerConfigurationDialog::on_httpRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_currentConfiguration->setDefaults(ServerConfiguration::HTTP);
   copyFrom(*m_currentConfiguration);

   m_dialog.remoteHostGroupBox->setEnabled(true);
   m_dialog.configureConnectionButton->setEnabled(false);
   m_dialog.authentication->setEnabled(false);
   m_dialog.userName->setEnabled(false);
   m_dialog.workingDirectory->setEnabled(false);
   updateAllowedQueueSystems(true);
}


void ServerConfigurationDialog::on_httpsRadioButton_toggled(bool tf)
{
   if (!tf) return;

   m_currentConfiguration->setDefaults(ServerConfiguration::HTTPS);
   copyFrom(*m_currentConfiguration);

   m_dialog.remoteHostGroupBox->setEnabled(true);
   m_dialog.configureConnectionButton->setEnabled(false);
   m_dialog.authentication->setEnabled(false);
   m_dialog.userName->setEnabled(false);
   m_dialog.workingDirectory->setEnabled(false);
   updateAllowedQueueSystems(true);
}


void ServerConfigurationDialog::on_configureConnectionButton_clicked(bool)
{
   SshFileDialog dialog(this); 
   dialog.exec();
}


void ServerConfigurationDialog::on_configureQueueButton_clicked(bool)
{
   copyTo(m_currentConfiguration);
   QueueOptionsDialog dialog(m_currentConfiguration, this);
   dialog.exec();
}


void ServerConfigurationDialog::on_queueSystem_currentIndexChanged(QString const& queue)
{
  m_currentConfiguration->setDefaults(ServerConfiguration::toQueueSystemT(queue));
}


void ServerConfigurationDialog::copyFrom(ServerConfiguration const& config)
{
   switch (config.connection()) {
      case ServerConfiguration::Local:
         m_dialog.localRadioButton->setChecked(true);
         m_dialog.queueSystem->setCurrentIndex(config.queueSystem());
         break;
      case ServerConfiguration::SSH:
         m_dialog.sshRadioButton->setChecked(true);
         m_dialog.queueSystem->setCurrentIndex(config.queueSystem());
         break;
      case ServerConfiguration::HTTP:
         m_dialog.httpRadioButton->setChecked(true);
         break;
      case ServerConfiguration::HTTPS:
         m_dialog.httpsRadioButton->setChecked(true);
         break;
   }

qDebug() << "in copyfrom";
config.dump();
   m_dialog.serverName->setText(config.name());
   m_dialog.hostAddress->setText(config.value(ServerConfiguration::HostAddress).toString());
   m_dialog.port->setValue(config.value(ServerConfiguration::Port).toInt());
   m_dialog.authentication->setCurrentIndex(config.authentication());
   m_dialog.userName->setText(config.value(ServerConfiguration::UserName).toString());

   m_dialog.workingDirectory->setText(
      config.value(ServerConfiguration::WorkingDirectory).toString());

   m_tested = false;
}


bool ServerConfigurationDialog::copyTo(ServerConfiguration* config)
{
   // Sanity checks
   if (m_dialog.serverName->text().trimmed().isEmpty()) {
      QMsgBox::warning(this, "IQmol", "Server name not set");
      return false;
   }

   ServerConfiguration::ConnectionT connection(ServerConfiguration::Local);
   if (m_dialog.sshRadioButton->isChecked()) {
qDebug() << "Setting connection to SSH";
      connection = ServerConfiguration::SSH;
   }else if (m_dialog.httpRadioButton->isChecked()) {
qDebug() << "Setting connection to HTTP";
      connection = ServerConfiguration::HTTP;
   }else if (m_dialog.httpsRadioButton->isChecked()) {
      connection = ServerConfiguration::HTTPS;
   }

   if (connection != ServerConfiguration::Local) {
      if (m_dialog.hostAddress->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "Host address not set");
         return false;
      }
   }

   if (connection == ServerConfiguration::SSH) {
      if (m_dialog.userName->text().trimmed().isEmpty()) {
         QMsgBox::warning(this, "IQmol", "User name must be set");
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

   if (connection == ServerConfiguration::Local) return true;

   config->setValue(ServerConfiguration::HostAddress, 
      m_dialog.hostAddress->text());
   config->setValue(ServerConfiguration::Port, 
      m_dialog.port->value());

   config->setValue(ServerConfiguration::Authentication, 
      ServerConfiguration::toAuthenticationT(m_dialog.authentication->currentText()));

   config->setValue(ServerConfiguration::UserName, 
      m_dialog.userName->text());
   config->setValue(ServerConfiguration::WorkingDirectory,
      m_dialog.workingDirectory->text());

  qDebug() << "---------------------------------------------------";
  config->dump();
  qDebug() << "---------------------------------------------------";

   return true;
}


void ServerConfigurationDialog::on_testConnectionButton_clicked(bool) 
{ 
   testConnection(); 
}


bool ServerConfigurationDialog::testConnection()
{
   bool okay(false);

   if (!copyTo(m_currentConfiguration)) return okay;

   switch (m_currentConfiguration->connection()) {
      case ServerConfiguration::Local:
         QMsgBox::information(this, "IQmol", "Local connection just fine");
         break;
      case ServerConfiguration::SSH:
         okay = testSshConnection(*m_currentConfiguration);
         break;
      case ServerConfiguration::HTTP:
      case ServerConfiguration::HTTPS:
         okay = testHttpConnection(*m_currentConfiguration);
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
      QString hostAddress(configuration.value(ServerConfiguration::HostAddress).toString());
      QString userName(configuration.value(ServerConfiguration::UserName).toString());
      Network::SshConnection::AuthenticationT authentication(configuration.authentication());

      int port(configuration.port());

      Network::SshConnection ssh(hostAddress, port);
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
      QString hostAddress(configuration.value(ServerConfiguration::HostAddress).toString());
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


/*
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
*/


/*
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
*/


void ServerConfigurationDialog::verify()
{
   if (!copyTo(m_currentConfiguration)) return;

   if (!m_tested && !m_dialog.localRadioButton->isChecked()) {
      QString msg("Would you like to try connecting to the server?");
      if (QMsgBox::question(this, "IQmol", msg,
         QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes) {
         if (!testConnection()) return;
      }
   }

   m_originalConfiguration = *m_currentConfiguration;
   //copyTo(&m_originalConfiguration);

   accept();
}


void ServerConfigurationDialog::on_loadButton_clicked(bool)
{
   QString filePath(QDir::homePath()); 
   filePath = QFileDialog::getOpenFileName(this, tr("Open Server Configuration"),
      filePath, tr("Configuration Files (*.cfg)"));

   if (filePath.isEmpty()) return;

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
         ServerConfiguration config(*(yaml.first()));
         copyFrom(config);
      }

   } catch (YAML::Exception& err) {
      QString msg(QString::fromStdString(err.what()));
      QMsgBox::warning(this, "IQmol", msg);
   }
}
       

void ServerConfigurationDialog::on_exportButton_clicked(bool)
{
   if (!copyTo(m_currentConfiguration)) return;

   QString filePath(QDir::homePath()); 
   filePath += "/iqmol_server.cfg";

   filePath = QFileDialog::getSaveFileName(this, tr("Save File"), filePath, 
       tr("Configuration Files (*.cfg)"));

   if (filePath.isEmpty()) return;
   Data::YamlNode node(m_currentConfiguration->toYamlNode());
   if (!node.saveToFile(filePath)) {
      QMsgBox::warning(this, "IQmol", "Failed to export server configuration");
   }
}

} } // end namespace IQmol::Process
