#ifndef IQMOL_SECURECONNECTION_H
#define IQMOL_SECURECONNECTION_H
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

#include <libssh2.h>
#include <QMutex>
#include <QObject>


namespace IQmol {
namespace SecureConnection {

   class Thread;

   enum Authentication { Agent, HostBased, KeyboardInteractive, Password, PublicKey };

   class Connection : public QObject {

      Q_OBJECT

      friend class Thread;

      public:
         Connection(QString const& hostname, QString const& username, int const port);
         ~Connection();

         bool connect(Authentication const, QString const& password = "");
         void disconnect();
         bool isConnected() { return m_connected; }


      protected:
         int m_socket;
         QMutex m_mutex;
         LIBSSH2_SESSION* m_session;
         QString lastError();


      private:
         // converts a hostname to IP address 
         QString lookupHost(QString const& hostname);
         
         void connectSocket();
         bool connectAgent();
         bool connectHostBased(QString const& passphrase);
         bool connectKeyboardInteractive();
         bool connectPublicKey(QString const& passphrase);
         bool connectPassword(QString const& password);

         void killAgent();
         void checkHost();
         QString getPrivateKey();
         QString getPublicKey();

         LIBSSH2_AGENT* m_agent;
         QString m_hostname;
         QString m_username;
         int  m_port;
         bool m_connected;
   };


} } // end namespace IQmol::SecureConnection

#endif
