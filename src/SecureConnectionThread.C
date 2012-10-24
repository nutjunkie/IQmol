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

#include "SecureConnection.h"
#include "SecureConnectionThread.h"
#include "SecureConnectionException.h"
#include <sys/time.h>

#include <QDebug>


namespace IQmol {
namespace SecureConnection {

Thread::Thread(Connection* connection, int const timeout) : Threaded(timeout)
{
   m_session = connection->m_session;
   m_socket = connection->m_socket;
   setMutex(&(connection->m_mutex));
}
         

void Thread::waitsocket()
{ 
   fd_set fd;
   fd_set *writefd = 0;
   fd_set *readfd  = 0;
   
   struct timeval timeout;
   timeout.tv_sec  = m_timeout / 1000;
   timeout.tv_usec = 1000*(m_timeout % 1000);
   
   FD_ZERO(&fd);
   FD_SET(m_socket, &fd);

   /* now make sure we wait in the correct direction */
   int dir(libssh2_session_block_directions(m_session));
   if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)  readfd  = &fd;
   if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND) writefd = &fd;
   
   int rv(select(m_socket + 1, readfd, writefd, 0, &timeout));

   if (rv == -1) {
      throw Exception("Error in select()");
   }else if (rv == 0) {
      throw Timeout();
   }
}



void Exec::run()
{
   qDebug() << "SecureConnection::Exec::run " << m_command;
   // Exececute non-blocking on the remote host 
   libssh2_session_set_blocking(m_session, 0);

   LIBSSH2_CHANNEL *channel;
   while ( (channel = libssh2_channel_open_session(m_session)) == 0 &&
           libssh2_session_last_error(m_session, 0, 0, 0) == LIBSSH2_ERROR_EAGAIN) {
        waitsocket();
   }

   if (channel == 0) throw Exception("Failed to open exec channel");

   int rc(0);
   QByteArray cmd(m_command.toLocal8Bit());

   while ( (rc = libssh2_channel_exec(channel, cmd.data())) == LIBSSH2_ERROR_EAGAIN) {
       waitsocket();
   }

   std::string output;
   int bytecount(0);

   if (rc == 0) {

      for ( ; ; ) { // loop until we block
          int bc;
          char buffer[0x400];
          do {
              bc = libssh2_channel_read(channel, buffer, sizeof(buffer));
              if (bc > 0) {
                 output.append(buffer, bc);
                 bytecount += bc;
               }
//qDebug() <<" sleeping 5"; sleep(5);

          } while( bc > 0 && !m_terminate);

          if (m_terminate) break;

          // this is due to blocking that would occur 
          // otherwise so we loop on this condition
          if (bc == LIBSSH2_ERROR_EAGAIN) {
             waitsocket();
          }else {
             break;
          }
      }

   }else {
      m_errorMessage = "Failed to execute command: " + m_command;
   }

   while ( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN ) {
       waitsocket();
   }

   if (!m_terminate) {
      char *exitsignal=(char *)"none";
      if (rc == 0) {
         libssh2_channel_get_exit_signal(channel, &exitsignal, 0, 0, 0, 0, 0);
      }
      if (exitsignal) {
         m_errorMessage += "\nClose on channel received signal " + QString(exitsignal);
      }
   }

   libssh2_channel_free(channel);
   if (!m_errorMessage.isEmpty()) throw Exception(m_errorMessage);
   if (!m_terminate) m_outputMessage = QString::fromStdString(output).trimmed();
      qDebug() << "              return:" << m_outputMessage;
}



void Push::run()
{
   QLOG_TRACE() << "Sending file " << m_sourceFilePath;
   // Check the local file is there first
qDebug() << "SecureConnection::Push " << m_sourceFilePath << "->" << m_destinationFilePath;
   QByteArray source(m_sourceFilePath.toLocal8Bit());
   FILE* localFileHandle(fopen(source.data(), "rb"));
            
   if (!localFileHandle) {
      QString msg("Could not stat file ");
      msg += m_sourceFilePath;
      throw Exception(msg);
   }  
         
   struct stat fileInfo;
   stat(source.data(), &fileInfo);
         
   // Set blocking, which apparently is required.
   libssh2_session_set_blocking(m_session, 1);

   QByteArray destination(m_destinationFilePath.toLocal8Bit());
   LIBSSH2_CHANNEL* channel(libssh2_scp_send(m_session, destination.data(),
      fileInfo.st_mode & 0777, (unsigned long)fileInfo.st_size));
         
   if (channel == 0) {
      QString msg("Unable to open channel for writing to file ");
      msg += m_destinationFilePath;
      throw Exception(msg);
   }
  
   size_t nread;
   char buffer[1024];
   char* ptr;
   int rc;
      
   do {
       nread = fread(buffer, 1, sizeof(buffer), localFileHandle);
       if (nread <= 0)  break; // end of file
       ptr = buffer;
       
       do {
          // write the same data over and over, until error or completion 
          // rc indicates how many bytes were written this time 
          rc = libssh2_channel_write(channel, ptr, nread);
          if (rc < 0) {
             m_errorMessage = "Error writing to channel " + QString::number(rc);
             break;
          }else {
             ptr += rc;
             nread -= rc;
          }
       } while (nread && !m_terminate);
       copyProgress();
   
   } while (!m_terminate);
   
   fclose(localFileHandle);
   
   libssh2_channel_send_eof(channel);
   libssh2_channel_wait_eof(channel);
   libssh2_channel_wait_closed(channel);
   libssh2_channel_free(channel);
   
   if (!m_errorMessage.isEmpty()) throw Exception(m_errorMessage);
   if (!m_terminate) m_success = true;
}



void Pull::run()
{
   QLOG_TRACE() << "Receiving file " << m_sourceFilePath;
   qDebug() << "SecureConnection::Pull " << m_sourceFilePath << "->" << m_destinationFilePath;

   // Set blocking, which apparently is required.
   QByteArray source(m_sourceFilePath.toLocal8Bit());
   libssh2_session_set_blocking(m_session, 1);
   struct stat fileInfo;
   LIBSSH2_CHANNEL* channel(libssh2_scp_recv(m_session, source.data(), &fileInfo));

   if (channel == 0) {
       QString msg("Could not stat file ");
       msg += m_sourceFilePath;
       throw Exception(m_session, msg);
   }

   QByteArray destination(m_destinationFilePath.toLocal8Bit());
   FILE* localFileHandle(fopen(destination.data(), "wb"));

   if (!localFileHandle) {
      QString msg("Could not open file for writing ");
      msg += m_destinationFilePath;
      throw Exception(msg);
   }

   // If the buffer size changes, anything connected to the copyProgress will
   // need updating as it assumes kbyte increments.
   char buffer[1024];
   off_t got(0);
   
   while (got < fileInfo.st_size && !m_terminate) {
       int amount(sizeof(buffer));
       if ((fileInfo.st_size - got) < amount) {
          amount = fileInfo.st_size - got;
       }

       int bc(libssh2_channel_read(channel, buffer, amount));

       if (bc > 0) {
          fwrite(buffer, 1, bc, localFileHandle);
       }else if (bc < 0) {
          m_success = false;
          m_errorMessage = "Error reading from channel";
          break;
       }
       got += bc;
       copyProgress();
//qDebug() << "sleeping 1"; sleep(1);
   }

   fclose(localFileHandle);

   libssh2_channel_send_eof(channel);
// This seems to cause a hang sometimes
//   libssh2_channel_wait_eof(channel);
   libssh2_channel_wait_closed(channel);
   libssh2_channel_free(channel);

   if (!m_errorMessage.isEmpty()) throw Exception(m_errorMessage);
   if (!m_terminate) m_success = true;
}



} } // end namespce IQmol::SecureConnection
