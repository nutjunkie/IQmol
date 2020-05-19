#ifndef IQMOL_NETWORK_SSHCONNECTION_H
#define IQMOL_NETWORK_SSHCONNECTION_H
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

#define LIBSSH2_ERROR_PUBLICKEY_NOT_FOUND      -100
#define LIBSSH2_ERROR_AUTHENTICATION_CANCELLED -101

#include "Connection.h"
#include <libssh2.h>


namespace IQmol {
namespace Network {

   class SshReply;
   class SshExecute;

   class SshConnection : public Connection {

      Q_OBJECT

      friend class SshExecute;
      friend class SshPutFile;
      friend class SshGetFile;
      friend class SftpPutFile;
      friend class SftpGetFile;

      public:
         SshConnection(QString const& hostname, int const port,
             QString const& publicKeyFile, QString const& privateKeyFile,
             QString const& knownHostsFile, bool const useSftp = false);
             
         ~SshConnection() { close(); }

         void open();
         void close();
         void authenticate(AuthenticationT const, QString& username);

         bool blockingExecute(QString const& command, QString* message = 0);
         bool exists(QString const& filePath);
         bool makeDirectory(QString const& path);
         bool removeDirectory(QString const& path);

         Reply* execute(QString const& command);
         Reply* execute(QString const& command, QString const& workingDirectory);

         Reply* putFile(QString const& sourcePath, QString const& destinationPath);
         Reply* getFile(QString const& sourcePath, QString const& destinationPath);
         Reply* getFiles(QStringList const& fileList, QString const& destinationPath);

/*
         Reply* sftpPutFile(QString const& sourcePath, QString const& destinationPath);
         Reply* sftpGetFile(QString const& sourcePath, QString const& destinationPath);
         Reply* sftpGetFiles(QString const& sourcePath, QString const& destinationPath);
*/

         bool waitSocket();  // returns true on timeout

         void start(Reply*);
            
         // for debugging
         Reply* test(QString const& id);
         void callRedundant();

      protected:
         LIBSSH2_SESSION* m_session;
         QString lastSessionError();
     
      private:
         static unsigned s_numberOfConnections;

         int m_socket;
         LIBSSH2_AGENT* m_agent;
         QString m_username;
         QString m_publicKeyFile;
         QString m_privateKeyFile;
         QString m_knownHostsFile;
         bool m_useSftp;

         void init();
         bool checkHost();
         bool openSocket(unsigned const timeout);
         bool setNonBlocking();
         bool setBlocking();
         void killAgent();

         int connectAgent();
         int connectHostBased();
         int connectKeyboardInteractive();
         int connectPublicKey();
         int connectPassword();

         QString getPasswordFromUser(QString const& message);
         QString lastError();
         QString getPublicKeyFile()  const;
         QString getPrivateKeyFile() const;
         QString getKnownHostsFile() const;
   };

} } // end namespace IQmol::Network

#endif
