#ifndef IQMOL_NETWORK_SSHREPLY_H
#define IQMOL_NETWORK_SSHREPLY_H
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

#include "Reply.h"
#include <QStringList>


namespace IQmol {
namespace Network {

   class SshConnection;

   class SshReply : public Reply {

      Q_OBJECT

      public:
         SshReply(SshConnection*);
         virtual ~SshReply() { }

      protected Q_SLOTS:
         void run();

      protected:
         static QString subEnv(QString const& command);
         virtual void runDelegate() = 0;
         SshConnection* m_connection;
   };


   class SshTest : public SshReply {

      Q_OBJECT

      public:
         SshTest(SshConnection* connection, QString const& id) :
            SshReply(connection), m_id(id) { }

      protected:
         void runDelegate();

      private:
         QString m_id;
   };


   class SshExecute : public SshReply {

      Q_OBJECT

      public:
         SshExecute(SshConnection* connection, QString const& command) : 
            SshReply(connection), m_command(subEnv(command)) { }

      protected:
         void setCommand(QString const& command) { m_command = subEnv(command); }
         void runDelegate();

      private:
         QString m_command;
   };


   class SshPutFile : public SshReply {

      Q_OBJECT

      public:
         SshPutFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath) : SshReply(connection), 
            m_sourcePath(subEnv(sourcePath)), m_destinationPath(subEnv(destinationPath)) { }

      protected:
         void runDelegate();

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };


   class SftpPutFile : public SshReply {

      Q_OBJECT

      public:
         SftpPutFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath) : SshReply(connection), 
            m_sourcePath(subEnv(sourcePath)), m_destinationPath(subEnv(destinationPath)) { }

      protected:
         void runDelegate();

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };


   class SshGetFile : public SshReply {

      Q_OBJECT

      friend class SshGetFiles;

      public:
         SshGetFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath) : SshReply(connection), m_getFilesInterrupt(false),
            m_sourcePath(subEnv(sourcePath)), m_destinationPath(subEnv(destinationPath)) { }

      protected:
         void runDelegate();
         void runDelegate(bool& getFilesInterrupt);
         bool m_getFilesInterrupt;

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };


   class SftpGetFile : public SshReply {

      Q_OBJECT

      friend class SshGetFiles;

      public:
         SftpGetFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath) : SshReply(connection), m_getFilesInterrupt(false),
            m_sourcePath(subEnv(sourcePath)), m_destinationPath(subEnv(destinationPath)) { }

      protected:
         void runDelegate();
         void runDelegate(bool& getFilesInterrupt);
         bool m_getFilesInterrupt;

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };


   class SshGetFiles : public SshReply {

      Q_OBJECT

      public:
         SshGetFiles(SshConnection* connection, QStringList const& fileList, 
            QString const& destinationDirectory) : SshReply(connection), 
            m_fileList(fileList), m_destinationDirectory(destinationDirectory) { }

      protected:
         void runDelegate();

      private:
         QStringList m_fileList;
         QString m_destinationDirectory;
   };


   class SftpGetFiles : public SshReply {

      Q_OBJECT

      public:
         SftpGetFiles(SshConnection* connection, QStringList const& fileList, 
            QString const& destinationDirectory) : SshReply(connection), 
            m_fileList(fileList), m_destinationDirectory(destinationDirectory) { }

      protected:
         void runDelegate();

      private:
         QStringList m_fileList;
         QString m_destinationDirectory;
   };


} } // end namespace IQmol::Network

#endif
