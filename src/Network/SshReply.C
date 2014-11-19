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

#include "SshReply.h"
#include "SshConnection.h"
#include "Exception.h"
#include "QsLog.h"
#include <QFileInfo>


namespace IQmol {
namespace Network {


SshReply::SshReply(SshConnection* connection) : m_connection(connection)
{ 
}


void SshReply::run()
{
   // We catch exceptions here as we expect the Reply to be running
   // on a Connection thread.
   try {
      m_status = Running;
      runDelegate();
      m_status = m_interrupt ? Interrupted : Finished;
   }catch (NetworkTimeout& ex) {
      m_status  = TimedOut;
      m_message = ex.what();
   }catch (Exception& ex) {
      m_status  = Error;
      m_message = ex.what();
   }
   finished();
}


QString SshReply::subEnv(QString const& command)
{
   QString cmd(command);
   return cmd.replace("~", "$HOME");
}


// -------------- SshTest ----------------

void SshTest::runDelegate() 
{
   for (int i = 0; i < 10; ++i) {
      QLOG_TRACE() << "Running thread" << m_id << i;
      sleep(2);
      if (m_interrupt) break;
   }
}


// -------------- SshExecute ----------------

void SshExecute::runDelegate()
{
   QLOG_TRACE() << "SshExecute" << m_command;
   QString error;
   std::string output;
   int bytecount(0);

   // Exececute non-blocking on the remote host 
   LIBSSH2_SESSION* session(m_connection->m_session);
   libssh2_session_set_blocking(session, 0);

   LIBSSH2_CHANNEL* channel(0);
   while ( (channel = libssh2_channel_open_session(session)) == 0 &&
           libssh2_session_last_error(session, 0, 0, 0) == LIBSSH2_ERROR_EAGAIN) {
      if (m_interrupt) return;
      if (m_connection->waitSocket()) throw NetworkTimeout();
   }

   if (channel == 0) {
      QString msg("Failed to open execution channel:\n");
      throw Exception(msg + m_connection->lastSessionError());
   }

   int rc(0);
   QByteArray cmd(m_command.toLocal8Bit());

   while ( (rc = libssh2_channel_exec(channel, cmd.data())) == LIBSSH2_ERROR_EAGAIN) {
      if (m_interrupt) goto cleanup;
      m_connection->waitSocket();
   }

   if (rc != 0) {
      error = "Command execution failed: " + m_command;
      goto cleanup;
   }

   for ( ; ; ) { // loop until we block
       int bc;
       char buffer[0x400];
          do {
           bc = libssh2_channel_read(channel, buffer, sizeof(buffer));
           if (bc > 0) {
              output.append(buffer, bc);
              bytecount += bc;
           }
           //qDebug() <<" sleeping 2"; sleep(2);

       } while( bc > 0 && !m_interrupt);

       if (m_interrupt) goto cleanup;

       // this is due to blocking that would occur 
       // otherwise so we loop on this condition
       if (bc == LIBSSH2_ERROR_EAGAIN) {
          m_connection->waitSocket();
       }else {
          break;
       }
   }


   if (m_interrupt) {
      QLOG_TRACE() << "---------- SshExecute Interrupted -----------";
   }else {
      m_message = QString::fromStdString(output).trimmed();
   }

   cleanup:
      QLOG_TRACE() <<  "Closing channel";
      while ( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN ) {
          m_connection->waitSocket();
      }

      if (rc == 0) {
         char* exitsignal = (char *)"none";
         libssh2_channel_get_exit_signal(channel, &exitsignal, 0, 0, 0, 0, 0);
         if (exitsignal) { 
            QLOG_TRACE() << "Channel close recieved signal " + QString(exitsignal); 
         }
      }

      libssh2_channel_free(channel);
      if (!m_interrupt && !error.isEmpty()) throw Exception(error);
}


// -------------- SshPutFile ----------------

void SshPutFile::runDelegate()
{
   QLOG_TRACE() << "SshPutFile:" << m_sourcePath << "->" << m_destinationPath;

   // Check the local file is there first
   QByteArray source(m_sourcePath.toLocal8Bit());
   FILE* localFileHandle(fopen(source.data(), "rb"));

   if (!localFileHandle) {
      QString msg("File not found: ");
      throw Exception(msg + m_sourcePath);
   }

   // Set blocking, which apparently is required.
   LIBSSH2_SESSION* session(m_connection->m_session);
   libssh2_session_set_blocking(session, 1);

   struct stat fileInfo;
   stat(source.data(), &fileInfo);

   QByteArray destination(m_destinationPath.toLocal8Bit());
   LIBSSH2_CHANNEL* channel(0);

   while ( (channel = libssh2_scp_send(session, destination.data(),
      fileInfo.st_mode & 0777, (unsigned long)fileInfo.st_size)) == 0 &&
      libssh2_session_last_error(session, 0, 0, 0) == LIBSSH2_ERROR_EAGAIN) {
      if (m_interrupt) return;
      m_connection->waitSocket();
   }

   if (channel == 0) {
      QString msg("Failed to open send channel:\n");
      throw Exception(msg + m_connection->lastSessionError());
   }
 
   // If the buffer size changes, anything connected to the copyProgress will
   // need updating as it assumes kbyte increments.
   char    buffer[1024];
   char*   ptr;
   int     rc(0);
   size_t  nread;
   QString error;

   QLOG_TRACE() <<  "Prepared to send" << fileInfo.st_size << "bytes";

   while (!m_interrupt) {
       nread = fread(buffer, 1, sizeof(buffer), localFileHandle);
       if (nread <= 0)  break; // end of file
       ptr = buffer;

       // write the same data over and over, until error or completion 
       // rc indicates how many bytes were written this time 
       do {
          rc = libssh2_channel_write(channel, ptr, nread);
          if (rc < 0) {
             error  = "Error writing to channel:\n";
             error += m_connection->lastSessionError();
             break;
          }else {
             ptr   += rc;
             nread -= rc;
          }
       } while (nread && !m_interrupt);

       copyProgress();
   } 

   QLOG_TRACE() <<  "Closing send channel";
   fclose(localFileHandle);
   libssh2_channel_send_eof(channel);
   libssh2_channel_wait_eof(channel);
   libssh2_channel_wait_closed(channel);
   libssh2_channel_free(channel);

   if (!m_interrupt && !error.isEmpty()) throw Exception(error);
}


// -------------- SshGetFile ----------------

void SshGetFile::runDelegate()
{
   QLOG_TRACE() << "SshGetFile " << m_destinationPath << "<-" << m_sourcePath;

   // Check we can write to the local file first
   QByteArray destination(m_destinationPath.toLocal8Bit());
   FILE* localFileHandle(fopen(destination.data(), "wb"));

   if (!localFileHandle) {
      QString msg("Could not open file for writing: ");
      throw Exception(msg + m_destinationPath);
   }

   // Set blocking, which apparently is required.
   LIBSSH2_SESSION* session(m_connection->m_session);
   libssh2_session_set_blocking(session, 1);

   QByteArray source(m_sourcePath.toLocal8Bit());
   LIBSSH2_CHANNEL* channel(0);
   struct stat fileInfo;

   while ( (channel = libssh2_scp_recv(session, source.data(), &fileInfo)) == 0 &&
           libssh2_session_last_error(session, 0, 0, 0) == LIBSSH2_ERROR_EAGAIN) {
      if (m_interrupt) {
         fclose(localFileHandle);
         return;
      }
      m_connection->waitSocket();
   }

   if (channel == 0) {
      QString msg("Failed to open receive channel:\n");
      fclose(localFileHandle);
      throw Exception(msg + m_connection->lastSessionError());
   }

   // If the buffer size changes, anything connected to the copyProgress will
   // need updating as it assumes Kbyte increments.
   char    buffer[1024];
   off_t   got(0);

/*
   QLOG_DEBUG() << "File info" << fileInfo.st_dev     << "   " << sizeof(dev_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_ino     << "   " << sizeof(ino_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_mode    << "   " << sizeof(mode_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_nlink   << "   " << sizeof(nlink_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_uid     << "   " << sizeof(uid_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_gid     << "   " << sizeof(gid_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_rdev    << "   " << sizeof(dev_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_size    << "   " << sizeof(off_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_blksize << "   " << sizeof(blksize_t);
   QLOG_DEBUG() << "File info" << fileInfo.st_blocks  << "   " << sizeof(blkcnt_t);
*/

   QString error;
   unsigned fileSize(fileInfo.st_size);
   QLOG_TRACE() <<  "Preparing to receive" << fileSize << "bytes";

   if (fileSize == 0) {
      error  = "Unable to stat file on server: " + m_sourcePath + "\n";
      error += "Check file exists and firewall permits incoming connections";
      goto cleanup;
   }

   while (got < fileSize && !m_interrupt) {
       int amount(sizeof(buffer));
       if ((fileInfo.st_size - got) < amount) {
          amount = fileInfo.st_size - got;
       }

       int bc(libssh2_channel_read(channel, buffer, amount));

       if (bc > 0) {
          fwrite(buffer, 1, bc, localFileHandle);
       }else if (bc < 0) {
          error  = "Error reading from channel";
          error += m_connection->lastSessionError();
          break;
       }

       got += bc;

qDebug() << "Sending copyProgress()";
       copyProgress();
      //qDebug() << "sleeping 1"; sleep(1);
   }

   cleanup:
      QLOG_TRACE() <<  "Closing receive channel";
      fclose(localFileHandle);
      libssh2_channel_send_eof(channel);
      // This seems to cause a hang sometimes
      // libssh2_channel_wait_eof(channel);
      libssh2_channel_wait_closed(channel);
      libssh2_channel_free(channel);
      QLOG_TRACE() <<  "Channel closed";

      if (!m_interrupt && !error.isEmpty()) throw Exception(error);
}


// -------------- SshGetFiles ----------------

void SshGetFiles::runDelegate()
{
   QStringList::iterator iter;
   for (iter = m_fileList.begin(); iter != m_fileList.end(); ++iter) {
       QString source(*iter);
       QFileInfo info(source);
       QString destination(m_destinationDirectory);
       destination += "/" + info.fileName();

       SshGetFile get(m_connection, source, destination);
       get.runDelegate();
   }
   finished();
}

           
} } // end namespace IQmol::Network
