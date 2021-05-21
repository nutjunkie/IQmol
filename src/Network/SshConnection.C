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
#include <QFileInfo>
#include <QEventLoop>
#include <QInputDialog>

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
#include "openssl/sha.h"
#include "openssl/md5.h"


namespace IQmol {
namespace Network {

// This keeps track of the number of open connections to determine when
// libssh_init and libssh_exit need to be called.
unsigned SshConnection::s_numberOfConnections = 0;


SshConnection::SshConnection(QString const& hostname, int const port, 
   QString const& publicKeyFile, QString const& privateKeyFile, 
   QString const& knownHostsFile, bool const useSftp) : 
   Connection(hostname, port), m_session(0), m_socket(0), m_agent(0), 
   m_publicKeyFile(publicKeyFile), m_privateKeyFile(privateKeyFile),
   m_knownHostsFile(knownHostsFile), m_useSftp(useSftp)
{
}



void SshConnection::open()
{
   QLOG_TRACE() << "Opening SSH connection to" << m_hostname;
   try {
      if (s_numberOfConnections == 0) init();

      if (openSocket(m_timeout)) {
         ++s_numberOfConnections;
         m_status = Opened;
      }else {
      }
   }catch (...)  {
      QLOG_ERROR() << "Error opening SSH connection";
   }
}



void SshConnection::close()
{
   // No logging as the Logger may not exist on shutdown
   if (m_status == Closed) return;

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

   m_status = Closed;
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
      m_message = "WSAStartup failed to initialize: error ";
      m_message += QString::number(res);
      m_status = Error;
#endif

   QLOG_TRACE() << "Calling libssh2_init()";
   int rc(libssh2_init(0));
   if (rc != 0) {
      m_message = "Failed to initialize libssh2 connection";
      m_status = Error;
   }
}



bool SshConnection::openSocket(unsigned const timeout)
{
   // Create the socket
   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket < 0)  {
      m_message = "Failed to create SSH socket";
      m_status = Error;
      QLOG_ERROR() << m_message;
      return false;
   }

   if (!setNonBlocking()) {
      m_message = "Failed to set non-blocking on socket";
      m_status = Error;
      QLOG_ERROR() << m_message;
      return false;
   }

   // Now try and connect with timout
   struct sockaddr_in sin;
   sin.sin_family = AF_INET;
   sin.sin_port = htons(m_port);
   sin.sin_addr.s_addr = HostLookup(m_hostname);

   if (sin.sin_addr.s_addr == INADDR_NONE) {
      m_message = "Invalid hostname: " + m_hostname;
      m_status = Error;
      QLOG_ERROR() << m_message;
      return false;
   }

   int rc(::connect(m_socket, (struct sockaddr*)&sin, sizeof(struct sockaddr_in)));

#ifdef WIN32
   if (rc == SOCKET_ERROR) rc = -1;
   int ok(WSAEWOULDBLOCK);
#else
   int ok(EINPROGRESS);
#endif

   if (rc < 0) {

      if (errno != ok) {
         m_message = "SSH connection to " + m_hostname + " failed";
         m_status = Error;
         QLOG_ERROR() << m_message;
         return false;
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
            m_message = "Connection to " + m_hostname + " timed out";
            m_status = Error;
            QLOG_ERROR() << m_message;
            return false;

         }else if (rc < 0 && errno != EINTR) {
            m_message = "Connection to " + m_hostname + " failed: " + lastError();
            m_status = Error;
            QLOG_ERROR() << m_message;
            return false;

         }else if (rc > 0 && errno != EINTR) {
            // Socket selected for write 
            socklen_t lon(sizeof(int));
            int errorStatus;
            
#ifdef WIN32
            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)(&errorStatus), &lon) < 0)
#else
            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)(&errorStatus), &lon) < 0) 
#endif
            {
               QString msg(": ");
               m_message = "Error check on socket: " + m_hostname + " " + lastError();
               m_status = Error;
               QLOG_ERROR() << m_message;
            }

            // Check the value returned... 
            if (errorStatus) {
               m_message  = "Connection failed ";
               m_message += QString::number(errorStatus) + ": " + lastError();
               m_status   = Error;
               QLOG_ERROR() << m_message;
               return false;
            }
            break;
         }

      } while (1);
   }

   if (!setBlocking()) {
      QLOG_ERROR() << "Failed to set blocking on socket.";
      return false;
   }

   return true;
}


bool SshConnection::checkHost()
{
   bool ok(true);

   LIBSSH2_KNOWNHOSTS* knownHosts = libssh2_knownhost_init(m_session);
   if (!knownHosts) {
      m_message = "Unable to initialize SSH known hosts"; 
      return false;
   }

   libssh2_knownhost_readfile(knownHosts, getKnownHostsFile().toLatin1().data(),  
      LIBSSH2_KNOWNHOST_FILE_OPENSSH);

   size_t length;
   int    type, check;
   const char* fingerprint = libssh2_session_hostkey(m_session, &length, &type);

   if (!fingerprint) {
      m_message = "Failed to obtain host key fingerprint";
      ok = false;
      goto cleanup;
   }

   struct libssh2_knownhost *host;

   check = libssh2_knownhost_checkp(knownHosts, m_hostname.toLatin1().data(), 
       m_port, fingerprint, length, LIBSSH2_KNOWNHOST_TYPE_PLAIN | 
       LIBSSH2_KNOWNHOST_KEYENC_RAW, &host);

   switch (check) {
      case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
         m_message = "Failed SSH known host check";
         ok = false;
         goto cleanup;
         break;
      case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
         m_message = "SSH host key not found in " + getKnownHostsFile();
         ok = false;
         goto cleanup;
         break;
      case LIBSSH2_KNOWNHOST_CHECK_MATCH:
         m_message = "Host key found";
         ok = true;
         break;
      case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
         m_message = "SSH keys do not match with host found in " + getKnownHostsFile();
         ok = false;
         goto cleanup;
         break;
   }


   if (host && false) {
      printf("Host check: %d, key: %s\n", check,
         (check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH) ? host->key : "<none>");

      unsigned char hash[MD5_DIGEST_LENGTH];
      unsigned char* key = reinterpret_cast<unsigned char*>(host->key); 
      unsigned len(strlen(host->key));
      fprintf(stderr, "key length: %d\n",len);
      MD5(key, len-1, hash);

      for (int i = 0; i < 20; ++i) {
         fprintf(stderr, "%02X:", hash[i]);
     }

     fprintf(stderr, "\n");
   }
                
   cleanup:
      QLOG_TRACE() << m_message;
      libssh2_knownhost_free(knownHosts);
      return ok;
}


void SshConnection::authenticate(AuthenticationT const authentication, QString& username)
{
   m_username = username;

   if (m_socket <= 0) {
      m_message  = "Authentication attempt on invalid socket";
      m_status   = Error;
      QLOG_ERROR() << m_message;
      return;
   }

   // Create a session instance
   m_session = libssh2_session_init();
   if (!m_session) {
      m_message  = "Failed to initialize SSH session";
      m_status   = Error;
      QLOG_ERROR() << m_message;
      return;
   }

   // This trades welcome banners, exchange keys,
   // and sets up crypto, compression, and MAC layers
   int rc(0);
   while ((rc = libssh2_session_handshake(m_session, m_socket)) == LIBSSH2_ERROR_EAGAIN);

   if (rc) {
      m_message  = "Failed to establish a valid SSH session (";
      m_message += QString::number(rc) + "): " + lastSessionError();
      m_status   = Error;
      QLOG_ERROR() << m_message;
      return;

   }
  
   if (!checkHost()) {
      m_status = Error;
      return;
   }

   if (false) {
      const char* fingerprint_MD5(libssh2_hostkey_hash(m_session, LIBSSH2_HOSTKEY_HASH_MD5));
      fprintf(stderr, "SSH Fingerprint (MD5):    ");
      for (int i = 0; i < 16; ++i) {
          fprintf(stderr, "%02X ", (unsigned char)fingerprint_MD5[i]);
      }
      fprintf(stderr, "\n");

      const char* fingerprint_SHA1(libssh2_hostkey_hash(m_session, LIBSSH2_HOSTKEY_HASH_SHA1));
      fprintf(stderr, "SSH Fingerprint (SHA1):   ");
      for (int i = 0; i < 20; ++i) {
          fprintf(stderr, "%02X ", (unsigned char)fingerprint_SHA1[i]);
      }
      fprintf(stderr, "\n");

      const char* fingerprint_SHA256(
         libssh2_hostkey_hash(m_session, LIBSSH2_HOSTKEY_HASH_SHA256));
      fprintf(stderr, "SSH Fingerprint (SHA256): ");
      for (int i = 0; i < 32; ++i) {
          fprintf(stderr, "%02X ", (unsigned char)fingerprint_SHA256[i]);
      }
      fprintf(stderr, "\n");
   }


   // Check what authentication methods are available
   char* authenticationMethods =
      libssh2_userauth_list(m_session, username.toLatin1().data(), username.length());

   //publickey,gssapi-with-mic,password,hostbased
   //publickey,keyboard-interactive

   rc = LIBSSH2_ERROR_METHOD_NOT_SUPPORTED;

   switch (authentication) {

      case Anonymous:
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

   switch (rc) {
      case LIBSSH2_ERROR_NONE:
         m_status = Authenticated;
         QLOG_INFO() << "SSH Connection established";
         break;

      case LIBSSH2_ERROR_PUBLICKEY_NOT_FOUND:
         m_message = "Public key not found for host " + m_hostname;
         m_status  = Error;
         QLOG_ERROR() << m_message;
         break;

      case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
         m_message  = "SSH authentication error: " + m_hostname;
         m_status   = Error;
         QLOG_ERROR() << m_message;
         break;

      case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED:
         m_message  = ToString(authentication) + " authentication not supported\n\n";
         m_message += "Supported methods: ";
         m_message += QString(authenticationMethods).replace(",",", ");
         m_status   = Error;
         QLOG_ERROR() << m_message;
         break;

      case LIBSSH2_ERROR_AUTHENTICATION_CANCELLED:
         m_message  = "Authentication cancelled";
         QLOG_WARN() << m_message;
         break;

      case LIBSSH2_ERROR_SOCKET_RECV:
         m_message = "Connection interrupted, try again.\n" + lastSessionError();
         QLOG_WARN() << m_message;
         break;

      default:
         m_message = "Authentication failed:\n" + lastSessionError();
         m_status  = Error;
         QLOG_ERROR() << m_message;
         QLOG_ERROR() << "rc code: " << rc;
         break;
   }
}


int SshConnection::connectAgent()
{
   QLOG_TRACE() << "SshConnection::connectAgent()";
   m_agent = libssh2_agent_init(m_session);
   if (!m_agent) {
      QLOG_ERROR() << "Failed to initialize ssh-agent support";
      return -1;  // LIBSSH2_ERROR_SOCKET_NONE - generic error code.
   }

   if (libssh2_agent_connect(m_agent)) {
      killAgent();
      QLOG_ERROR() << "Failed to connect to ssh-agent";
      return -1;  // LIBSSH2_ERROR_SOCKET_NONE - generic error code.
   }

   if (libssh2_agent_list_identities(m_agent)) {
      killAgent();
      QLOG_ERROR() << "Failed to request identities from ssh-agent";
      return -1;  // LIBSSH2_ERROR_SOCKET_NONE - generic error code.
   }

   int rc;
   struct libssh2_agent_publickey *identity(0), *prev_identity(0);

   while (1) {
      rc = libssh2_agent_get_identity(m_agent, &identity, prev_identity);
      if (rc == 1 || rc < 0) {
         killAgent();
         QLOG_ERROR() << "Failed to obtain identity from ssh-agent"; 
         return -1;  // LIBSSH2_ERROR_SOCKET_NONE - generic error code.
      }

      rc = libssh2_agent_userauth(m_agent, m_username.toLatin1().data(), identity);
      if (rc == LIBSSH2_ERROR_NONE) break;
      prev_identity = identity;
   }

   return rc;
}


int SshConnection::connectHostBased()
{
   QLOG_WARN() << "WARNING connectHostBased() not wired correctly";
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
   QString msg("Enter passphrase for key ");
   msg += privateKey;

   int rc(LIBSSH2_ERROR_AUTHENTICATION_FAILED);

   for (int count = 0; count < 3; ++count) {
      QString passphrase(getPasswordFromUser(msg));

      while ((rc = libssh2_userauth_publickey_fromfile(m_session, m_username.toLatin1().data(),
         publicKey.toLatin1().data(), privateKey.toLatin1().data(), 
         passphrase.toLatin1().data())) == LIBSSH2_ERROR_EAGAIN);

      if (rc != LIBSSH2_ERROR_AUTHENTICATION_FAILED) break;
   }

   return rc;
}


QString SshConnection::getPublicKeyFile() const
{
   QFileInfo info(m_publicKeyFile);
   if (!info.exists()) {
      QLOG_ERROR() << "Failed to find SSH public identity file:\n" + m_publicKeyFile;
   }
   return m_publicKeyFile;
}


QString SshConnection::getPrivateKeyFile() const
{
   QFileInfo info(m_privateKeyFile);
   if (!info.exists()) {
      QLOG_ERROR() << "Failed to find SSH private identity file:\n" + m_privateKeyFile;
   }
   return m_privateKeyFile;
}


QString SshConnection::getKnownHostsFile() const
{
   QFileInfo info(m_knownHostsFile);
   if (!info.exists()) {
      QLOG_ERROR() << "Failed to find SSH private identity file:\n" + m_knownHostsFile;
   }
   return m_knownHostsFile;
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


bool SshConnection::setNonBlocking()
{
#ifdef WIN32
   unsigned long arg(0);
   int ret(0);
   if ( (ret = ioctlsocket(m_socket, FIONBIO, &arg)) == SOCKET_ERROR) {
      return false;
   }
#else
   long arg(0);
   if ( (arg = fcntl(m_socket, F_GETFL, NULL)) < 0) {
      return false;
   }
   arg |= O_NONBLOCK;
   if (fcntl(m_socket, F_SETFL, arg) < 0) {
      return false;
   }
#endif
   return true;
}


bool SshConnection::setBlocking()
{
#ifdef WIN32
   unsigned long arg(1);
   int ret(0);
   if ( (ret = ioctlsocket(m_socket, FIONBIO, &arg)) == SOCKET_ERROR) {
      return false;
   }
#else
   long arg(0);
   if ((arg = fcntl(m_socket, F_GETFL, NULL)) < 0) {
      return false;
   }
   arg &= (~O_NONBLOCK);
   if (fcntl(m_socket, F_SETFL, arg) < 0) {
      return false;
   }
#endif
   return true;
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

   if (rc == -1) {
      QLOG_ERROR() << "Error in SshConnection::waitSocket()";
      rc = 0;
   }

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
   SshReply* reply(0);
   if (m_useSftp) {
      reply = new SftpPutFile(this, sourcePath, destinationPath);
   }else {
      reply = new SshPutFile(this, sourcePath, destinationPath);
   }
   thread(reply);
   return reply;
}


Reply* SshConnection::getFile(QString const& sourcePath, QString const& destinationPath) 
{
   SshReply* reply(0);
   if (m_useSftp) {
      reply = new SftpGetFile(this, sourcePath, destinationPath);
   }else {
      reply = new SshGetFile(this, sourcePath, destinationPath);
   }
   thread(reply);
   return reply;
}


Reply* SshConnection::getFiles(QStringList const& fileList, QString const& destinationPath)
{
   SshReply* reply(0);
   if (m_useSftp) {
      reply = new SftpGetFiles(this, fileList, destinationPath);
   }else {
      reply = new SshGetFiles(this, fileList, destinationPath);
   }
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
