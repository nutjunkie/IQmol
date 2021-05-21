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

#include "SshReply.h"
#include "SshConnection.h"
#include "Exception.h"
#include "QsLog.h"
#include <QFileInfo>
#include <unistd.h>
#include <libssh2_sftp.h>


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
#ifdef WIN32
      Sleep(2000);
#else
      sleep(2);
#endif
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
      msg += m_connection->lastSessionError();
      QLOG_ERROR() << msg;
      throw Exception(msg);
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
      //QLOG_TRACE() <<  "Closing channel";
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
      if (!m_interrupt && !error.isEmpty()) {
         QLOG_ERROR() << error;
         throw Exception(error);
      }
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

   //QLOG_TRACE() <<  "Closing send channel";
   fclose(localFileHandle);
   libssh2_channel_send_eof(channel);
   libssh2_channel_wait_eof(channel);
   libssh2_channel_wait_closed(channel);
   libssh2_channel_free(channel);

   if (!m_interrupt && !error.isEmpty()) throw Exception(error);
}


// -------------- SftpPutFile ----------------

void SftpPutFile::runDelegate()
{
   QLOG_TRACE() << "SftPutFile:" << m_sourcePath << "->" << m_destinationPath;

   // Check the local file is there first
   QByteArray source(m_sourcePath.toLocal8Bit());
   FILE* localFileHandle(fopen(source.data(), "rb"));

   if (!localFileHandle) {
      QString msg("File not found: ");
      throw Exception(msg + m_sourcePath);
   }

   LIBSSH2_SESSION* session(m_connection->m_session);
   LIBSSH2_SFTP* sftp_session(0);

   while (!sftp_session) {
      sftp_session = libssh2_sftp_init(session);
      if (!sftp_session && (libssh2_session_last_errno(session) != LIBSSH2_ERROR_EAGAIN)) {
         QString msg("Unable to init SFTP session for transfer: \n");
         throw Exception(msg + m_sourcePath);
      }
   } 

   LIBSSH2_SFTP_HANDLE* sftp_handle(0);
   QByteArray destination(m_destinationPath.toLocal8Bit());

   while (!sftp_handle) {
      sftp_handle = libssh2_sftp_open(sftp_session, destination,
         LIBSSH2_FXF_WRITE    | LIBSSH2_FXF_CREAT    | LIBSSH2_FXF_TRUNC,
         LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR |
         LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH);
 
        if (!sftp_handle && (libssh2_session_last_errno(session) != LIBSSH2_ERROR_EAGAIN)) {
            QString msg("Unable to open remote file handle with SFTP\n");
            throw Exception(msg + m_destinationPath);
        }
   }

   struct stat fileInfo;
   stat(source.data(), &fileInfo);
   QLOG_TRACE() <<  "Preparing to send" << fileInfo.st_size << "bytes via sftp";

   // If the buffer size changes, anything connected to the copyProgress will
   // need updating as it assumes kbyte increments.
   char    buffer[1024];
   char*   ptr;
   int     rc(0);
   size_t  total(0);
   size_t  nread;
   QString error;

   do {
       nread = fread(buffer, 1, sizeof(buffer), localFileHandle);
       if (nread <= 0) break;  // end of file 
       ptr    = buffer;
       total += nread;
          
       do { 
          // write data in a loop until we block  
          while ((rc = libssh2_sftp_write(sftp_handle, ptr, nread)) == LIBSSH2_ERROR_EAGAIN) {
             m_connection->waitSocket();
          }

          if (rc < 0) break;
          ptr   += rc;
          nread -= rc;
 
       } while (nread);

       copyProgress();

   } while (rc > 0 && !m_interrupt);

   if (total == fileInfo.st_size) QLOG_TRACE() << "Transfer complete";

   fclose(localFileHandle);
   libssh2_sftp_close(sftp_handle);
   libssh2_sftp_shutdown(sftp_session);

   if (!m_interrupt && !error.isEmpty()) throw Exception(error);
}



// -------------- SshGetFile ----------------
// HACK, needs cleaning up
void SshGetFile::runDelegate()
{
   bool parentInterrupt(false);
   runDelegate(parentInterrupt);
}


void SshGetFile::runDelegate(bool& getFilesInterrupt)
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

   // If the buffer size changes, anything connected to the copyProgress will
   // need updating as it assumes Kbyte increments.
   char buffer[1024];
   QString error;
   unsigned fileSize(fileInfo.st_size);
   unsigned got(0);
   QLOG_TRACE() <<  "Preparing to receive" << fileSize << "bytes";

   if (fileSize == 0) {
      error  = "Unable to stat file on server: " + m_sourcePath + "\n";
      error += "Check file exists and firewall permits incoming connections";
      goto cleanup;
   }

   while ((got < fileSize) && !m_interrupt && !getFilesInterrupt) {
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

       //copyProgress();
       double frac(double(got)/double(fileSize));
       copyProgress(frac);
       // qDebug() << "CopyProgress" << m_interrupt << frac;
       // qDebug() << "sleeping 1"; sleep(1);
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


// -------------- SftpGetFile ----------------
// HACK, needs cleaning up
void SftpGetFile::runDelegate()
{
   bool parentInterrupt(false);
   runDelegate(parentInterrupt);
}


void SftpGetFile::runDelegate(bool& getFilesInterrupt)
{
   QLOG_TRACE() << "SftpGetFile " << m_destinationPath << "<-" << m_sourcePath;

   // Check we can write to the local file first
   QByteArray destination(m_destinationPath.toLocal8Bit());
   FILE* localFileHandle(fopen(destination.data(), "wb"));

   if (!localFileHandle) {
      QString msg("Could not open file for writing: ");
      throw Exception(msg + m_destinationPath);
   }

   // initialize session
   QLOG_TRACE() << "Initializing SFTP session for read";
   LIBSSH2_SESSION* session(m_connection->m_session);
   LIBSSH2_SFTP* sftp_session(0);

   while (!sftp_session && !getFilesInterrupt) {
      sftp_session = libssh2_sftp_init(session);

      if (!sftp_session) {
         if (libssh2_session_last_errno(session) == LIBSSH2_ERROR_EAGAIN) {
            m_connection->waitSocket();
         }else {
            QString msg("Unable to init SFTP session for transfer: \n");
            throw Exception(msg + m_sourcePath);
         }
      }
   } 

   QLOG_TRACE() << "Opening SFTP handle for read transfer";
   LIBSSH2_SFTP_HANDLE* sftp_handle(0);
   QByteArray source(m_sourcePath.toLocal8Bit());

   while (!sftp_handle && !getFilesInterrupt) {
      sftp_handle = libssh2_sftp_open(sftp_session, source,  LIBSSH2_FXF_READ, 0); 
 
      if (!sftp_handle) {
         if (libssh2_session_last_errno(session) == LIBSSH2_ERROR_EAGAIN) {
            m_connection->waitSocket();
         }else {
            QString msg("Unable to open remote file handle with SFTP\n");
            throw Exception(msg + m_sourcePath);
         }
      }
   } 

   // stat the filesize
   LIBSSH2_SFTP_ATTRIBUTES attributes;
   while ( libssh2_sftp_fstat(sftp_handle, &attributes) == LIBSSH2_ERROR_EAGAIN) {
      m_connection->waitSocket();
   }

   unsigned fileSize(attributes.filesize);

   QLOG_DEBUG() << "SFTP file size transfer: " << fileSize;

   // If the buffer size changes, anything connected to the copyProgress will
   // need updating as it assumes Kbyte increments.
   char     buffer[1024];
   int      rc(0);
   unsigned got(0);
   QString  error;

   while (got < fileSize && !m_interrupt && !getFilesInterrupt) {
      // read in a loop until we block 
      do {
         rc = libssh2_sftp_read(sftp_handle, buffer, sizeof(buffer));
         if (rc > 0) {
            fwrite(buffer, rc, 1, localFileHandle);
            got += rc;
            double frac(double(got)/double(fileSize));
            copyProgress(frac);
         }
      } while (rc > 0);
 
      if (rc == LIBSSH2_ERROR_EAGAIN) {
         m_connection->waitSocket();

      }else if (rc < 0) {
          error  = "Error reading from sftp handle: ";
          error += m_connection->lastSessionError();
          break;
      }
   }

   cleanup:
      QLOG_TRACE() <<  "Closing sftp read transfer";
      fclose(localFileHandle);
      libssh2_sftp_shutdown(sftp_session);

      if (!m_interrupt && !error.isEmpty()) throw Exception(error);
}

// -------------- SshGetFiles ----------------

void SshGetFiles::runDelegate()
{
   QStringList::iterator iter;
   for (iter = m_fileList.begin(); iter != m_fileList.end(); ++iter) {
       if (m_interrupt) break;
       QString source(*iter);
       QFileInfo info(source);
       QString destination(m_destinationDirectory);
       destination += "/" + info.fileName();

       SshGetFile get(m_connection, source, destination);
       connect(&get, SIGNAL(copyProgress()), this, SIGNAL(copyProgress()));
       connect(&get, SIGNAL(copyProgress(double)), this, SIGNAL(copyProgress(double)));
       get.runDelegate(m_interrupt);
   }
   finished();
}


void SftpGetFiles::runDelegate()
{
   QStringList::iterator iter;
   for (iter = m_fileList.begin(); iter != m_fileList.end(); ++iter) {
       if (m_interrupt) break;
       QString source(*iter);
       QFileInfo info(source);
       QString destination(m_destinationDirectory);
       destination += "/" + info.fileName();

       SftpGetFile get(m_connection, source, destination);
       connect(&get, SIGNAL(copyProgress()), this, SIGNAL(copyProgress()));
       connect(&get, SIGNAL(copyProgress(double)), this, SIGNAL(copyProgress(double)));
       get.runDelegate(m_interrupt);
   }
   finished();
}

          
} } // end namespace IQmol::Network
