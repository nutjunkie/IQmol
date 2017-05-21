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

#include "SshConnection.h"
#include "SshReply.h"
#include "QsLog.h"
#include "Network.h"
#include "NetworkException.h"
#include "Preferences.h"
#include <QInputDialog>
#include <QFileInfo>
#include <QEventLoop>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define  in_addr_t u_long
//#define  errno WSAGetLastError()
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

#include <stdio.h>


namespace IQmol {
namespace Network {

// This keeps track of the number of open connections to determine when
// libssh_init and libssh_exit need to be called.
unsigned SshConnection::s_numberOfConnections = 0;


SshConnection::SshConnection(QString const& hostname, int const port) : 
   Connection(hostname, port), m_session(0), m_socket(0), m_agent(0)
{
}



void SshConnection::open()
{
   QLOG_TRACE() << "Opening connection to" << m_hostname;
   if (s_numberOfConnections == 0) init();

   openSocket(m_timeout);
   ++s_numberOfConnections;
   m_status = Connection::Opened;
}


void SshConnection::close()
{
   // No logging as the Logger may not exist on shutdown
   if (m_status == Connection::Closed) return;

   //QLOG_TRACE() << "Closing connection to" << m_hostname;
   killAgent();

   if (m_session) {
      libssh2_session_disconnect(m_session, "Normal Shutdown, Thank you for playing");
      libssh2_session_free(m_session);
      m_session = 0;
      //QLOG_TRACE() << "  Session closed";
   }

   if (m_socket) {
#ifdef WIN32
      closesocket(m_socket);
#else
      ::close(m_socket);
#endif
      m_socket = 0;
   }

   --s_numberOfConnections;
   if (s_numberOfConnections == 0) {
      libssh2_exit();
#ifdef WIN32
      WSACleanup();
#endif
   }

   m_status = Connection::Closed;
}


void SshConnection::killAgent()
{
   if (m_agent) {
      libssh2_agent_disconnect(m_agent);
      libssh2_agent_free(m_agent);
      m_agent = 0;
   }
}


void SshConnection::init()
{
#ifdef WIN32
   WSADATA wsadata;
   int res(WSAStartup(MAKEWORD(2,0), &wsadata));
   if (res != NO_ERROR) {
      QString msg("WSAStartup failed to initialize: error ");
      throw Exception(msg + QString::number(res));
   }   
#endif

   QLOG_TRACE() << "Calling libssh2_init()";
   int rc(libssh2_init(0));
   if (rc != 0) throw Exception("Failed to initialize libssh2 connection");
}


void SshConnection::openSocket(unsigned const timeout)
{
   // Create the socket
   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket < 0) throw Exception("Failed to create socket");

   setNonBlocking();

   // Now try and connect with timout
   struct sockaddr_in sin;
   sin.sin_family = AF_INET;
   sin.sin_port = htons(m_port);
   sin.sin_addr.s_addr = HostLookup(m_hostname);

   int rc(::connect(m_socket, (struct sockaddr*)&sin, sizeof(struct sockaddr_in)));

#ifdef WIN32
   if (rc == SOCKET_ERROR) rc = -1;
   int ok(WSAEWOULDBLOCK);
#else
   int ok(EINPROGRESS);
#endif

   if (rc < 0) {

      if (errno != ok) {
         QString msg("Connection failed: ");
         throw Exception(msg + lastError());
      }

      do {
         struct timeval tv;
         tv.tv_sec  = timeout / 1000;
         tv.tv_usec = 1000*(timeout % 1000);

         fd_set myset;
         FD_ZERO(&myset);
         FD_SET(m_socket, &myset);

         rc = select(m_socket+1, NULL, &myset, NULL, &tv);

         if (rc == 0) {
            throw NetworkTimeout();

         }else if (rc < 0 && errno != EINTR) {
            QString msg("Connection failed: ");
            throw Exception(msg + lastError());

         }else if (rc > 0) {
            // Socket selected for write 
            socklen_t lon(sizeof(int));
            int errorStatus;
            
#ifdef WIN32
            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)(&errorStatus), &lon) < 0) {
#else
            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)(&errorStatus), &lon) < 0) {
#endif
               QString msg("Error check on socket: ");
               throw Exception(msg + lastError());
            }

            // Check the value returned... 
            if (errorStatus) {
               QString msg("Connection failed: ");
               throw Exception(msg + lastError());
            }
            break;
         }

      } while (1);
   }

   setBlocking();
}


// Returns true if the username and password are valid, false otherwise.
// throws on any other error.
void SshConnection::authenticate(AuthenticationT const authentication, QString& username)
{
   m_username = username;

   if (m_socket <= 0) throw Exception("Authentication on invalid socket");

   // Create a session instance
   m_session = libssh2_session_init();
   if (!m_session) throw Exception("Failed to initialize SSH session");

   // This trades welcome banners, exchange keys,
   // and sets up crypto, compression, and MAC layers
   int rc(0);
   while ((rc = libssh2_session_handshake(m_session, m_socket)) == LIBSSH2_ERROR_EAGAIN);

   if (rc) {
      QString msg("Failed to establish a valid SSH session ");
      throw Exception(msg + lastSessionError());
   }

/* Can't get this working at the moment
   const char* fingerprint(libssh2_hostkey_hash(m_session, LIBSSH2_HOSTKEY_HASH_MD5));
   fprintf(stderr, "SSH Fingerprint: ");

   for (int i = 0; i < 20; ++i) {
       fprintf(stderr, "%02X ", (unsigned char)fingerprint[i]);
   }
   fprintf(stderr, "\n");
*/

//!!!

   // Check what authentication methods are available
   char* authenticationMethods =
      libssh2_userauth_list(m_session, username.toLatin1().data(), username.length());

   //publickey,gssapi-with-mic,password,hostbased
   //publickey,keyboard-interactive

   rc = LIBSSH2_ERROR_METHOD_NOT_SUPPORTED;

   switch (authentication) {

      case None:
         break;

      case Agent:
         rc = connectAgent();
         break;

      case HostBased:
         if (strstr(authenticationMethods, "hostbased") != NULL) {
            rc = connectHostBased();
         }
         break;

      case KeyboardInteractive:
         if (strstr(authenticationMethods, "keyboard-interactive") != NULL) {
            rc = connectKeyboardInteractive();
         }
         break;

      case Password:
         if (strstr(authenticationMethods, "password") != NULL) {
            rc = connectPassword();
         }
         break;

      case PublicKey:
         if (strstr(authenticationMethods, "publickey") != NULL) {
            rc = connectPublicKey();
         }
         break;
   }

   QString msg;

   switch (rc) {
      case LIBSSH2_ERROR_NONE:
         m_status = Connection::Authenticated;
         break;

      case LIBSSH2_ERROR_PUBLICKEY_NOT_FOUND:
         msg = "Public key not found for host " + m_hostname;
         throw Exception(msg);
         break;

      case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
         throw AuthenticationError();
         break;

      case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED:
         msg  = toString(authentication) + " authentication not supported\n\n";
         msg += "Supported methods: ";
         msg += QString(authenticationMethods).replace(",",", ");
         throw Exception(msg);
         break;

      case LIBSSH2_ERROR_AUTHENTICATION_CANCELLED:
         throw AuthenticationCancelled();
         break;

      default:
         QString msg("Authentication failed:\n");
         msg += lastSessionError();
         throw Exception(msg);
         break;
   }
}


int SshConnection::connectAgent()
{
   QLOG_TRACE() << "SshConnection::connectAgent()";
   m_agent = libssh2_agent_init(m_session);
   if (!m_agent) {
      QString msg("Failed to initialize ssh-agent support\n");
      throw Exception(msg + lastSessionError());
   }

   if (libssh2_agent_connect(m_agent)) {
      QString msg("Failed to connect to ssh-agent\n");
      msg += lastSessionError();
      killAgent();
      throw Exception(msg);
   }

   if (libssh2_agent_list_identities(m_agent)) {
      QString msg("Failed to request identities from ssh-agent\n");
      msg += lastSessionError();
      killAgent();
      throw Exception(msg);
   }

   int rc;
   struct libssh2_agent_publickey *identity(0), *prev_identity(0);

   while (1) {
      rc = libssh2_agent_get_identity(m_agent, &identity, prev_identity);
      if (rc == 1 || rc < 0) {
         QString msg("Failed to obtain identity from ssh-agent\n"); 
         msg += lastSessionError();
         killAgent();
         throw Exception(msg);
      }

      rc = libssh2_agent_userauth(m_agent, m_username.toLatin1().data(), identity);
      if (rc == LIBSSH2_ERROR_NONE) break;
      prev_identity = identity;
   }

   return rc;
}


int SshConnection::connectHostBased()
{
qDebug() << "WARNING connectHostBased() not wired correctly";
   QString passphrase;

   QLOG_TRACE() << "SshConnection::connectHostBased";
   QString privateKey(getPrivateKeyFile());
   QString publicKey(getPublicKeyFile());

   int rc;
   while ((rc = libssh2_userauth_hostbased_fromfile(m_session, m_username.toLatin1().data(),
      publicKey.toLatin1().data(), privateKey.toLatin1().data(), passphrase.toLatin1().data(), 
      m_hostname.toLatin1().data())) == LIBSSH2_ERROR_EAGAIN);
      
   return rc;
}


// this is a hack for when using the KeyboardCallback function.  We need to 
// know the server name to prompt the user. 
static QString CurrentServerName; 

static void KeyboardCallback(const char* /*name*/, int /*name_len*/, 
   const char* /*instruction*/, int /*instruction_len*/, int num_prompts, 
   const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts, 
   LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses, void** /*abstract*/) 
{ 
   QString password; 
   bool ok(true);
 
    for (int i = 0; i < num_prompts; i++) {
        QString msg("Prompt from ");
        msg += CurrentServerName + ":\n";
        std::string prompt(prompts[i].text, prompts[i].length);
        msg += QString::fromStdString(prompt);
        password = QInputDialog::getText(0, "IQmol", msg, QLineEdit::Password, QString(), &ok);
        if (!ok) break;
        responses[i].text = strdup(password.toLatin1().data());
        responses[i].length = password.length();
    }
}


int SshConnection::connectKeyboardInteractive()
{
   QLOG_TRACE() << "SshConnection::connectKeyboardInteractive()";

   int rc;
   CurrentServerName = m_hostname;
   for (int count = 0; count < 3; ++count) {
       while ((rc = libssh2_userauth_keyboard_interactive(m_session, 
          m_username.toLatin1().data(), 
          &KeyboardCallback)) == LIBSSH2_ERROR_EAGAIN);
       if (rc != LIBSSH2_ERROR_AUTHENTICATION_FAILED) break;
   }

   return rc;
}


QString SshConnection::getPasswordFromUser(QString const& message)
{
   bool okPushed(true);
   QString password(QInputDialog::getText(0, "IQmol", message, QLineEdit::Password, 
       QString(), &okPushed));

   if (!okPushed) password.clear();
   return password; 
}


int SshConnection::connectPassword()
{
   QLOG_TRACE() << "SshConnection::connectPassword" << m_username;
   QString msg("Password for ");
   msg += m_username;
   msg += "@" + m_hostname;

   int rc(LIBSSH2_ERROR_AUTHENTICATION_FAILED);

   for (int count = 0; count < 3; ++count) {
      QString password(getPasswordFromUser(msg));
      if (password.isEmpty()) return LIBSSH2_ERROR_AUTHENTICATION_CANCELLED;

      while ((rc = libssh2_userauth_password(m_session, m_username.toLatin1().data(), 
         password.toLatin1().data())) == LIBSSH2_ERROR_EAGAIN);

      if (rc != LIBSSH2_ERROR_AUTHENTICATION_FAILED) break;
   }

   return rc;
}


int SshConnection::connectPublicKey() 
{
   QLOG_TRACE() << "SshConnection::connectPublicKey";
   QString privateKey(getPrivateKeyFile());
   QString publicKey(getPublicKeyFile());
qDebug() << "WARNING connectHostBased() not wired correctly";
   QString passphrase;

   int rc;
   while ((rc = libssh2_userauth_publickey_fromfile(m_session, m_username.toLatin1().data(),
      publicKey.toLatin1().data(), privateKey.toLatin1().data(), 
      passphrase.toLatin1().data())) == LIBSSH2_ERROR_EAGAIN);

   if (rc == LIBSSH2_ERROR_AUTHENTICATION_FAILED) rc = LIBSSH2_ERROR_PUBLICKEY_NOT_FOUND;

   return rc;
}


QString SshConnection::getPublicKeyFile()
{
   QString fileName(Preferences::SSHPublicIdentityFile());
   QFileInfo info(fileName);
   if (!info.exists()) {
      throw Exception(QString("Failed to find SSH public identity file:\n") + fileName);
   }
   return fileName;
}


QString SshConnection::getPrivateKeyFile()
{
   QString fileName(Preferences::SSHPrivateIdentityFile());
   QFileInfo info(fileName);
   if (!info.exists()) {
      throw Exception(QString("Failed to find SSH private identity file:\n") + fileName);
   }
   return fileName;
}


QString SshConnection::lastError()
{
#ifdef WIN32
   return QString("Error code: " + QString::number(WSAGetLastError()));
#else
   return QString(strerror(errno));
#endif
}


QString SshConnection::lastSessionError()
{
   QString msg;

   if (m_session) {
      char* errorMessage;
      int length;
      int code(libssh2_session_last_error(m_session, &errorMessage, &length, 0));
      if (!msg.isEmpty()) msg += "\n";
      if (false) msg += "libssh2 error code " + QString::number(code) + "\n";
      msg += QString(errorMessage) + "\n";
   }

   return msg;
}


void SshConnection::setNonBlocking()
{
#ifdef WIN32
   unsigned long arg(0);
   int ret(0);
   if ( (ret = ioctlsocket(m_socket, FIONBIO, &arg)) == SOCKET_ERROR) {
      throw Exception("Failed to set non-blocking on socket.\n");
   }
#else
   long arg(0);
   if ( (arg = fcntl(m_socket, F_GETFL, NULL)) < 0) {
      throw Exception("Failed to set non-blocking on socket.\n");
   }
   arg |= O_NONBLOCK;
   if (fcntl(m_socket, F_SETFL, arg) < 0) {
      throw Exception("Failed to set non-blocking on socket.\n");
   }
#endif
}


void SshConnection::setBlocking()
{
#ifdef WIN32
   unsigned long arg(1);
   int ret(0);
   if ( (ret = ioctlsocket(m_socket, FIONBIO, &arg)) == SOCKET_ERROR) {
      throw Exception("Failed to set blocking on socket.\n");
   }
#else
   long arg(0);
   if ((arg = fcntl(m_socket, F_GETFL, NULL)) < 0) {
      throw Exception("Failed to set blocking on socket.\n");
   }
   arg &= (~O_NONBLOCK);
   if (fcntl(m_socket, F_SETFL, arg) < 0) {
      throw Exception("Failed to set blocking on socket.\n");
   }
#endif
}


bool SshConnection::waitSocket()
{
   //QLOG_TRACE() << "Calling waitSocket";
   if (!m_socket)  throw Exception("Wait on uninitialized socket");
   if (!m_session) throw Exception("Wait on uninitialized SSH session");

   fd_set fd;
   fd_set *writefd = 0;
   fd_set *readfd  = 0;

   struct timeval tv;
   tv.tv_sec  = m_timeout / 1000;
   tv.tv_usec = 1000*(m_timeout % 1000);

   FD_ZERO(&fd);
   FD_SET(m_socket, &fd);

   // Make sure we wait in the correct direction
   int dir(libssh2_session_block_directions(m_session));
   if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)  readfd  = &fd;
   if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND) writefd = &fd;

   int rc(select(m_socket+1, readfd, writefd, 0, &tv));

   if (rc == -1) throw Exception("Error in select()");

   bool timedout(rc == 0);
   if (timedout) { QLOG_TRACE() << "  waitsocket timeout"; }
   return timedout;
}


bool SshConnection::exists(QString const& filePath)
{
   QString cmd("test -e ");
   cmd += filePath;
   cmd += " && echo SUCCESS";

   QString msg;
   bool ok(blockingExecute(cmd, &msg));
   ok = ok && msg.contains("SUCCESS");

   return ok;
}


bool SshConnection::makeDirectory(QString const& path)
{
   QString cmd("mkdir -p ");
   cmd += path;
   cmd += " && echo SUCCESS";

   QString msg;
   bool ok(blockingExecute(cmd, &msg));
   ok = ok && msg.contains("SUCCESS");

   return ok;
}


bool SshConnection::removeDirectory(QString const& path)
{
   QString cmd("rm -fr ");
   cmd += path;
   cmd += " && echo SUCCESS";

   QString msg;
   bool ok(blockingExecute(cmd, &msg));
   ok = ok && msg.contains("SUCCESS");

   return ok;
}


bool SshConnection::blockingExecute(QString const& command, QString* message)
{
   QString cmd(command);

   SshReply* reply(new SshExecute(this, cmd));
   reply->start();

   //QEventLoop loop;
   //connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
   //thread(reply);
   //loop.exec();
             
   bool ok(reply->status() == Reply::Finished);
   if (message) *message = reply->message();
   reply->deleteLater();

   return ok;
}


Reply* SshConnection::execute(QString const& command)
{
   SshReply* reply(new SshExecute(this, command));
   thread(reply);
   return reply;
}


Reply* SshConnection::execute(QString const& command, QString const& workingDirectory)
{
   QString cmd("cd ");
   cmd += workingDirectory + " && " + command;
   SshReply* reply(new SshExecute(this, cmd));
   thread(reply);
   return reply;
}


Reply* SshConnection::putFile(QString const& sourcePath, QString const& destinationPath) 
{
   SshReply* reply(new SshPutFile(this, sourcePath, destinationPath));
   thread(reply);
   return reply;
}


Reply* SshConnection::getFile(QString const& sourcePath, QString const& destinationPath) 
{
   SshReply* reply(new SshGetFile(this, sourcePath, destinationPath));
   thread(reply);
   return reply;
}


Reply* SshConnection::getFiles(QStringList const& fileList, QString const& destinationPath)
{
   SshReply* reply(new SshGetFiles(this, fileList, destinationPath));
   thread(reply);
   return reply;
}


// for debugging
Reply* SshConnection::test(QString const& id)
{
   SshReply* reply(new SshTest(this, id));
   thread(reply);
   return reply;
}


void SshConnection::callRedundant()
{
   QLOG_TRACE() << "Redundant call made";
   //sleep(5);
   QLOG_TRACE() << "Redundant call finished";
}

} } // end namespace IQmol::Network
