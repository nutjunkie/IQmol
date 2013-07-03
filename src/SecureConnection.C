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

#include "SecureConnection.h"
#include "SecureConnectionException.h"
#include "PasswordVault.h"
#include "Preferences.h"
#include "QsLog.h"
#include <QFileInfo>
#include <QMutexLocker>
#include <QInputDialog>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define in_addr_t u_long
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h> 
#endif


namespace IQmol {
namespace SecureConnection {

// this is a hack for when using the KeyboardCallback function.  We need to
// know the server name to prompt the user.
static QString CurrentServerName;
static int NumberOfConnections = 0;

// -----------------------------------------------------------------------------
// used for KeyboardInteractive
// -----------------------------------------------------------------------------
static void KeyboardCallback(const char *name, int name_len, const char *instruction, 
   int instruction_len, int num_prompts, const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
   LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses, void **abstract)
{
   QString password;
   bool ok(true);
 
    for (int i = 0; i < num_prompts; i++) {
        QString msg("Prompt from ");
        msg += CurrentServerName + ": ";
        std::string prompt(prompts[i].text, prompts[i].length);
        msg += QString::fromStdString(prompt);
        password = QInputDialog::getText(0, "IQmol", msg, QLineEdit::Password, QString(), &ok);
        if (!ok) break;
        responses[i].text = strdup(password.toAscii().data());
        responses[i].length = password.length();
    }
}



Connection::Connection(QString const& hostname, QString const& username, int const port) 
 : m_session(0), m_agent(0), m_hostname(hostname), m_username(username), m_port(port), 
   m_connected(false)
{
   if (NumberOfConnections == 0) {
#ifdef WIN32
      WSADATA wsadata;
      int res = WSAStartup(MAKEWORD(2,0), &wsadata);
      if (res != NO_ERROR) {
         QString msg("WSAStartup failed to initialize: error ");
         msg += QString::number(res);
         throw Exception(msg);
      }
#endif
      int code(libssh2_init(0));
      if (code != 0) throw Exception("Failed to initialize secure connection");
   }else {
      ++NumberOfConnections;
   }
}


Connection::~Connection() 
{
   disconnect();
   --NumberOfConnections;
   if (NumberOfConnections == 0) {
      libssh2_exit();
#ifdef WIN32
      WSACleanup();
#endif
   }
}


QString Connection::lastError() 
{
   char* errorMessage;
   int length;
   libssh2_session_last_error(m_session, &errorMessage, &length, 0);
   return QString(errorMessage);
}


in_addr_t lookupHost2(QString const& hostname)
{
   // This is the validator for an IPv4 address only
   QString octet("(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])");
   QRegExpValidator ipv4Validator(0);
   ipv4Validator.setRegExp(QRegExp(
      "^" + octet + "\\." + octet + "\\." + octet + "\\." + octet + "$"));

   in_addr_t address;
   int pos;

   QString tmp(hostname);
   if (ipv4Validator.validate(tmp,pos) == QValidator::Acceptable) {
      address = inet_addr(hostname.toAscii().data());
      if (address == INADDR_NONE || address == INADDR_ANY) {
         throw Exception("Invalid IP address");
      }
   }else {
      struct hostent* host;
      host = gethostbyname(hostname.toAscii().data());
      if (!host) throw Exception("Invalid hostname");
      if (host->h_addrtype == AF_INET6) {
         throw Exception("IPv6 addresses not supported");
      }
      address = *(in_addr_t*)host->h_addr;
   }

   return address;
}


QString Connection::lookupHost(QString const& hostname)
{
#ifndef WIN32
  QLOG_DEBUG() << "Looking up hostname" << hostname;
  struct addrinfo hints, *res;
  int errcode;
  char addrstr[100];
  void *ptr;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  errcode = getaddrinfo (hostname.toAscii().data(), NULL, &hints, &res);
  if (errcode != 0) throw Exception("Host not found");

  while (res)
    {
      inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

      switch (res->ai_family) {
         case AF_INET:
            ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
            break;
         case AF_INET6:
            ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
            break;
      }
      inet_ntop (res->ai_family, ptr, addrstr, 100);
      printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
              addrstr, res->ai_canonname);
      return QString(addrstr);
      res = res->ai_next;
    }
#endif

  throw Exception("Host not found");
  return QString();
}


#ifdef WIN32
void Connection::connectSocket()
{
   // Create the socket
   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket < 0) {
      QString msg("Failed to create socket.\n");
      msg += "Error code: " + QString::number(WSAGetLastError());
      throw Exception(msg);
   }

   // Set non-blocking
   unsigned long arg(0);
   int ret;
   if ( (ret = ioctlsocket(m_socket, FIONBIO, &arg)) == SOCKET_ERROR) { 
      QString msg("Failed to set non-blocking on socket.\n");
      msg += "error code: " + QString::number(WSAGetLastError());
      throw Exception(msg);
   } 

   // Now try and connect with timout
   struct sockaddr_in sin;
   sin.sin_family = AF_INET;
   sin.sin_port = htons(m_port);
   //sin.sin_addr.s_addr = inet_addr(lookupHost(m_hostname).toAscii().data());
   sin.sin_addr.s_addr = lookupHost2(m_hostname);

   int res(::connect(m_socket, (struct sockaddr*)&sin, sizeof(struct sockaddr_in))); 

   if (res == SOCKET_ERROR) { 

      int err(WSAGetLastError());

      if (err != WSAEWOULDBLOCK) { 
         QString msg("Failed to connect:\n");
         msg += "Error code: " + QString::number(err);
         throw Exception(msg);
      }

      do { 
         struct timeval tv; 
         tv.tv_sec = 6; 
         tv.tv_usec = 0; 

         fd_set myset; 
         FD_ZERO(&myset); 
         FD_SET(m_socket, &myset); 

         res = select(m_socket+1, NULL, &myset, NULL, &tv); 
         err = WSAGetLastError();
         if (res < 0 && err != EINTR) { 
            QString msg("Connection error:\n");
            msg += "Error code: " + QString::number(err);
            throw Exception(msg);
         }else if (res > 0) { 
            // Socket selected for write 
            socklen_t lon(sizeof(int));
            int errorStatus; 

            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)(&errorStatus), &lon) < 0) { 
               QString msg("Error checking socket option:\n");
               msg += "error code: " + QString::number(WSAGetLastError());
               throw Exception(msg);
            } 

            break; 
         }else {
            throw Exception("Connection to server timed out");
         } 

      } while (1); 
   }
 
   // Set to blocking mode again... 
   arg = 1;
   if ( (ret = ioctlsocket(m_socket, FIONBIO, &arg)) == SOCKET_ERROR) { 
      QString msg("Failed to set blocking on socket.\n");
      msg += "error code: " + QString::number(WSAGetLastError());
      throw Exception(msg);
   } 
}

#else

void Connection::connectSocket()
{
   // Create the socket
   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket < 0) {
      QString msg("Failed to create socket.\n");
      msg += QString(strerror(errno));
      throw Exception(msg);
   }

   // Set non-blocking
   long arg; 
   if ( (arg = fcntl(m_socket, F_GETFL, NULL)) < 0) { 
      QString msg("Failed to set non-blocking:\n");
      msg += QString(strerror(errno));
      throw Exception(msg);
   } 
   arg |= O_NONBLOCK; 
   if ( fcntl(m_socket, F_SETFL, arg) < 0) { 
      QString msg("Failed to set non-blocking:\n");
      msg += QString(strerror(errno));
      throw Exception(msg);
   } 

   // Now try and connect with timout
   struct sockaddr_in sin;
   sin.sin_family = AF_INET;
   sin.sin_port = htons(m_port);
   sin.sin_addr.s_addr = inet_addr(lookupHost(m_hostname).toAscii().data());

   int res(::connect(m_socket, (struct sockaddr*)&sin, sizeof(struct sockaddr_in))); 

   if (res < 0) { 
      if (errno != EINPROGRESS) { 
         QString msg("Connection error:\n");
         msg += QString(strerror(errno));
         throw Exception(msg);
      }

      do { 
         struct timeval tv; 
         tv.tv_sec = 6; 
         tv.tv_usec = 0; 

         fd_set myset; 
         FD_ZERO(&myset); 
         FD_SET(m_socket, &myset); 

         res = select(m_socket+1, NULL, &myset, NULL, &tv); 
         if (res < 0 && errno != EINTR) { 
            QString msg("Connection error:\n");
            msg += QString(strerror(errno));
            throw Exception(msg);
         }else if (res > 0) { 
            // Socket selected for write 
            socklen_t lon(sizeof(int));
            int errorStatus; 

            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)(&errorStatus), &lon) < 0) { 
               QString msg("Check error on socket:\n");
               msg += QString(strerror(errno));
               throw Exception(msg);
            } 

            // Check the value returned... 
            if (errorStatus) { 
               QString msg("Connection error:\n");
               msg += QString(strerror(errorStatus));
               throw Exception(msg);
            } 
            break; 
         }else {
            throw Exception("Connection to server timed out");
         } 

      } while (1); 
   }
 
   // Set to blocking mode again... 
   if ((arg = fcntl(m_socket, F_GETFL, NULL)) < 0) { 
      QString msg("Failed to set blocking:\n");
      msg += QString(strerror(errno));
      throw Exception(msg);
   } 
   arg &= (~O_NONBLOCK); 
   if (fcntl(m_socket, F_SETFL, arg) < 0) { 
      QString msg("Failed to set blocking:\n");
      msg += QString(strerror(errno));
      throw Exception(msg);
   } 
}

#endif // WIN32



bool Connection::connect(Authentication const authentication, QString const& password)
{
   connectSocket();

   // Create a session instance
   m_session = libssh2_session_init();
   if (!m_session) throw Exception("Session initialization failed");

   // This trades welcome banners, exchange keys,
   // and sets up crypto, compression, and MAC layers
   int rc(0);
   while ((rc = libssh2_session_handshake(m_session, m_socket)) == LIBSSH2_ERROR_EAGAIN);

   if (rc) throw Exception(m_session, "Failed to establish a valid session");

   // We need this if we care about only connecting to known hosts.  Do we care?
   // checkHost();

   // Check what authentication methods are available
   char *authenticationMethods;
   authenticationMethods = 
      libssh2_userauth_list(m_session, m_username.toAscii().data(), m_username.length());

   //publickey,gssapi-with-mic,password,hostbased
   //publickey,keyboard-interactive

   QString errorMessage;

   switch (authentication) {

      case Agent:
         m_connected = connectAgent();
      break;

      case HostBased:
         if (strstr(authenticationMethods, "hostbased") == NULL) {
            errorMessage = "Host based authentication is not supported by server\n";
         }else {
            m_connected = connectHostBased(password);
         }
      break;

      case KeyboardInteractive:
         if (strstr(authenticationMethods, "keyboard-interactive") == NULL) {
            errorMessage = "Keyboard interactive authentication is not supported by server";
         }else {
            m_connected = connectKeyboardInteractive();
         }
      break;

      case Password:
         if (strstr(authenticationMethods, "password") == NULL) {
            errorMessage = "Password authentication is not supported by server";
         }else {
            m_connected = connectPassword(password);
         }
      break;
 
      case PublicKey: 
         if (strstr(authenticationMethods, "publickey") == NULL) {
            errorMessage = "Public key authentication is not supported by server";
         }else {
            m_connected = connectPublicKey(password);
         }
      break;
   }

   if (!errorMessage.isEmpty()) {
      errorMessage += "\nSupported methods: ";
      errorMessage += QString(authenticationMethods);
      throw Exception(errorMessage);
   }

   return m_connected;
}


bool Connection::connectAgent()
{
   m_agent = libssh2_agent_init(m_session);
   if (!m_agent) {
      throw Exception(m_session, "Failed to initialize ssh-agent support");
   }

   if (libssh2_agent_connect(m_agent)) {
      killAgent();
      throw Exception(m_session, "Failed to connect to ssh-agent");
   }

   if (libssh2_agent_list_identities(m_agent)) {
      killAgent();
      throw Exception(m_session, "Failed to request identities from ssh-agent");
   }

   int rc;
   struct libssh2_agent_publickey *identity(0), *prev_identity(0);

   bool connected(false);
   while (1) {
      rc = libssh2_agent_get_identity(m_agent, &identity, prev_identity);
      if (rc == 1 || rc < 0) {
         killAgent();
         throw Exception(m_session, "Failed to obtain identity from ssh-agent");
      }

      if (libssh2_agent_userauth(m_agent, m_username.toAscii().data(), identity)) {
         connected = false;
      }else {
         connected = true;
         break;
      }
      prev_identity = identity;
   }

   return connected;
}


bool Connection::connectHostBased(QString const& passphrase)
{
   QString privateKey(getPrivateKey());
   QString publicKey(getPublicKey());

   int rc;
   while ((rc = libssh2_userauth_hostbased_fromfile(m_session, m_username.toAscii().data(),
      publicKey.toAscii().data(), privateKey.toAscii().data(), passphrase.toAscii().data(), 
      m_hostname.toAscii().data())) == LIBSSH2_ERROR_EAGAIN);
      
   if (rc != 0) {
      QLOG_DEBUG() << "Connect with HostBased authentication failed: " << lastError();
   }
   return (rc == 0);
}


bool Connection::connectKeyboardInteractive()
{
   int rc;

   CurrentServerName = m_hostname;
   for (int count = 0; count < 3; ++count) {
       while ((rc = libssh2_userauth_keyboard_interactive(m_session, m_username.toAscii().data(), 
          &KeyboardCallback)) == LIBSSH2_ERROR_EAGAIN);
       if (rc != LIBSSH2_ERROR_AUTHENTICATION_FAILED) break;
       //QMsgBox::warning(0, "IQmol", "Invalid username/password");
   }

   if (rc != 0) {
      QLOG_DEBUG() << "Connect with Keyboard Interactive authentication failed: " << lastError();
   }
   return (rc == 0);
}


bool Connection::connectPassword(QString const& password)
{
   int rc;
   while ((rc = libssh2_userauth_password(m_session, m_username.toAscii().data(), 
      password.toAscii().data())) == LIBSSH2_ERROR_EAGAIN);
   if (rc != 0) {
      QLOG_DEBUG() << "Connect with Password authentication failed: " << lastError();
   }
   return (rc == 0);
}


bool Connection::connectPublicKey(QString const& passphrase) 
{
   QString privateKey(getPrivateKey());
   QString publicKey(getPublicKey());
 
   int rc;
   while ((rc = libssh2_userauth_publickey_fromfile(m_session, m_username.toAscii().data(),
      publicKey.toAscii().data(), privateKey.toAscii().data(), 
      passphrase.toAscii().data())) == LIBSSH2_ERROR_EAGAIN);

   if (rc != 0) {
      QLOG_DEBUG() << "Connect with PublicKey authentication failed: " << lastError();
   }
   return (rc == 0);
}


void Connection::checkHost()
{
   int check(LIBSSH2_KNOWNHOST_CHECK_FAILURE);
   LIBSSH2_KNOWNHOSTS* nh(libssh2_knownhost_init(m_session));

   QString knownHostsFile(Preferences::SSHKnownHostsFile());
   QFileInfo info(knownHostsFile);

   if (!info.exists()) {
      QString msg("Known Hosts file ");
      msg += knownHostsFile + " does not exist.";
      throw Exception(msg);
   }
   
   if (nh) {
      libssh2_knownhost_readfile(nh, knownHostsFile.toAscii().data(),
         LIBSSH2_KNOWNHOST_FILE_OPENSSH);

      int type;
      size_t len;
      const char* fingerprint = libssh2_session_hostkey(m_session, &len, &type);

      if (fingerprint) {
         struct libssh2_knownhost *host;
         check = libssh2_knownhost_checkp(nh, m_hostname.toAscii().data(), m_port, fingerprint,
            len, LIBSSH2_KNOWNHOST_TYPE_PLAIN|LIBSSH2_KNOWNHOST_KEYENC_RAW, &host);
      }

      libssh2_knownhost_free(nh);
   }

   QString errorMessage;
   switch (check) {
      case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
         errorMessage = "checkHost failed, this is unfortunate";
         break;
      case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
         errorMessage  = "Failed to find server in known_hosts file: " + m_hostname + "\n";
         errorMessage += "Please connect to server via ssh in a terminal before using IQmol";
         break;
      case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
         errorMessage = "Keys do not match for host " + m_hostname;
         break;
      default:
         break;
    }

    if (!errorMessage.isEmpty()) throw Exception(errorMessage);
}


QString Connection::getPublicKey()
{
   QString fileName(Preferences::SSHPublicIdentityFile());
   QFileInfo info(fileName);
   if (!info.exists()) {
      QString msg("Could not find SSH public identity file\n");
      msg += fileName;
      throw Exception(msg);
   }
   return fileName;
}


QString Connection::getPrivateKey()
{
   QString fileName(Preferences::SSHPrivateIdentityFile());
   QFileInfo info(fileName);
   if (!info.exists()) {
      QString msg("Could not find SSH private identity file\n");
      msg += fileName;
      throw Exception(msg);
   }
   return fileName;
}


void Connection::killAgent()
{
   if (m_agent) {
      libssh2_agent_disconnect(m_agent);
      libssh2_agent_free(m_agent);
      m_agent = 0;
   }
}


void Connection::disconnect()
{
   m_connected = false;
   killAgent();

   if (m_session) {
      libssh2_session_disconnect(m_session, "Normal Shutdown, Thank you for playing");
      libssh2_session_free(m_session);
      m_session = 0;
   }

   if (m_socket) {
#ifdef WIN32
      closesocket(m_socket);
#else
      close(m_socket);
#endif
      m_socket = 0;
   }
}


} } // end namespce IQmol::SecureConnection
