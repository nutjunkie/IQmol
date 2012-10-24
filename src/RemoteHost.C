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

#include "RemoteHost.h"
#include "SecureConnectionThread.h"
#include "PasswordVault.h"
#include "Process.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include <QFileInfo>
#include <QInputDialog>
#include <QFileInfo>

#include <QDebug>


namespace IQmol {

QString RemoteHost::s_sshPassphrase = "";

RemoteHost::~RemoteHost()
{ 
   disconnectServer();
}


bool RemoteHost::isConnected() 
{
   bool connected(false);
   if (m_connection) connected = m_connection->isConnected();
   return connected; 
}


void RemoteHost::disconnectServer()
{
   if (m_connection) {
      m_connection->disconnect();
      delete m_connection;
      m_connection = 0;
   }
}


bool RemoteHost::connectServer()
{
   if (isConnected()) return true;

   QLOG_TRACE() << "Connecting to Server: " << name() << " at host " << hostAddress();

   m_connection = new SecureConnection::Connection(hostAddress(), userName(), port());

   bool connected;
   switch (authentication()) {

      case Server::None:
         connected = false;
         break;

      case Server::Agent: 
         connected = m_connection->connect(SecureConnection::Agent);
         break;

      case Server::PublicKey: 
         connected = getPassphraseFromUserAndConnect(SecureConnection::PublicKey);
         break;

      case Server::HostBased: 
         connected = getPassphraseFromUserAndConnect(SecureConnection::HostBased);
         break;

      case Server::KeyboardInteractive: 
         connected = m_connection->connect(SecureConnection::KeyboardInteractive);
         break;

      case Server::Vault: 
         connected = getPasswordFromVaultAndConnect();
         break;

      case Server::Prompt: 
         connected = getPasswordFromUserAndConnect();
         break;
   }

   if (!connected) {
      QString msg("Connection to server ");
      msg += name() + " falied";
      QMsgBox::warning(0, "IQmol", msg);
      delete m_connection;
      m_connection = 0;
   }

   return connected;
}


bool RemoteHost::getPassphraseFromUserAndConnect(SecureConnection::Authentication auth)
{
   QString msg("SSH passphrase");
   bool okPushed(true);
   bool connected(false);

   while (!connected) {
      if (s_sshPassphrase.isEmpty()) {
         s_sshPassphrase = 
            QInputDialog::getText(0, "IQmol", msg, QLineEdit::Password, QString(), &okPushed); 
      }
      if (okPushed) {
         connected = m_connection->connect(auth, s_sshPassphrase);
         if (!connected) {
            QMsgBox::warning(0, "IQmol", "Invalid passphrase");
            s_sshPassphrase.clear();
         }
      }else {
         break;
      }
   }

   return connected; 
}


bool RemoteHost::getPasswordFromVaultAndConnect()
{
   PasswordVault& vault = PasswordVault::instance();
   QString password(vault.getServerPassword(name()));

   bool connected(false);
   if (!password.isEmpty()) {
      connected = m_connection->connect(SecureConnection::Password, password);
   }
   return connected;
}


bool RemoteHost::getPasswordFromUserAndConnect()
{
   QString msg("Password for ");
   QString password;
   msg += userName() + "@" + hostAddress();
   bool okPushed(true);
   bool connected(false);

   while (!connected) {
      password = QInputDialog::getText(0, "IQmol", msg, QLineEdit::Password, QString(), &okPushed); 
      if (okPushed) {
         connected = m_connection->connect(SecureConnection::Password, password);
         if (!connected) QMsgBox::warning(0, "IQmol", "Invalid password");
      }else {
         break;
      }
   }

   OverwriteString(password);
   return connected; 
}


bool RemoteHost::getWorkingDirectoryFromUser(JobInfo* jobInfo)
{
   QString dirName(jobInfo->get(JobInfo::RemoteWorkingDirectory));
   if (dirName.isEmpty()) {
      dirName = m_server->workingDirectory();
      if (!dirName.endsWith("/")) dirName += "/";
      dirName += jobInfo->get(JobInfo::BaseName);
   }

   bool okPushed(false);
   QString msg("Working directory on ");
   msg += m_server->name() + "\t\t";

   dirName = QInputDialog::getText(0, "IQmol", msg, QLineEdit::Normal, dirName, &okPushed);
   if (!okPushed || dirName.isEmpty()) return false;

   if (dirName.endsWith("/")) dirName.remove(dirName.length()-1,1);

   QFileInfo info(dirName);
   jobInfo->set(JobInfo::RemoteWorkingDirectory, dirName);
   jobInfo->set(JobInfo::BaseName, info.completeBaseName());

   return true;
}


Threaded* RemoteHost::exec(QString const& command) 
{
   return new SecureConnection::Exec(m_connection, command);
}


Threaded* RemoteHost::mkdir(QString const& dir) 
{
   QString cmd("mkdir ");
   cmd += dir;
   return new SecureConnection::Exec(m_connection, cmd);
}


Threaded* RemoteHost::exists(QString const& filePath, HostDelegate::FileFlags const flags) 
{
   QString cmd;

   if (flags & HostDelegate::Executable) {
      cmd += "/usr/bin/which " + filePath;
   }else {
      if (flags & HostDelegate::Directory) {
         cmd += "test -d " + filePath;
      }else {
         cmd += "test -f " + filePath;
      }

      if (flags & HostDelegate::Readable)   cmd += " && test -r " + filePath;
      if (flags & HostDelegate::Writable)   cmd += " && test -w " + filePath;
      if (flags & HostDelegate::Executable) cmd += " && test -x " + filePath;
 
   }

   cmd += " && echo exists";
   return new SecureConnection::Exec(m_connection, cmd);
}
 

Threaded* RemoteHost::push(QString const& sourcePath, QString const& destinationPath)
{
   return new SecureConnection::Push(m_connection, sourcePath, destinationPath);
}

            
Threaded* RemoteHost::pull(QString const& sourcePath, QString const& destinationPath) 
{
   return new SecureConnection::Pull(m_connection, sourcePath, destinationPath);
}
            

Threaded* RemoteHost::move(QString const& sourcePath, QString const& destinationPath) 
{
   // We assume the remote host is *nix based
   QString cmd("mv ");
   cmd += sourcePath + " " + destinationPath;
   return new SecureConnection::Exec(m_connection, cmd);
}


Threaded* RemoteHost::remove(QString const& filePath) 
{
   // We assume the remote host is *nix based
   QString cmd("rm ");
   cmd += filePath;
   return new SecureConnection::Exec(m_connection, cmd);
}


Threaded* RemoteHost::grep(QString const& string, QString const& filePath) 
{
   // We assume the remote host is *nix based
   QString cmd("grep -i \'");
   cmd += string +"\' " + filePath;
   return new SecureConnection::Exec(m_connection, cmd);
}


Threaded* RemoteHost::checkOutputForErrors(QString const& filePath) 
{
   // We assume the remote host is *nix based
   QString cmd("grep -m 1 -A2 fatal ");
   cmd += filePath + " | tail -1";
   return new SecureConnection::Exec(m_connection, cmd);
}
 
            

} // end namespace IQmol

