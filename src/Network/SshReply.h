#ifndef IQMOL_NETWORK_SSHREPLY_H
#define IQMOL_NETWORK_SSHREPLY_H
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

#include "Reply.h"


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
            SshReply(connection), m_command(command) { }

      protected:
         void setCommand(QString const& command) { m_command = command; }
         void runDelegate();

      private:
         QString m_command;
   };


   class SshPutFile : public SshReply {

      Q_OBJECT

      public:
         SshPutFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath) : SshReply(connection), 
            m_sourcePath(sourcePath), m_destinationPath(destinationPath) { }

      Q_SIGNALS:
         void copyProgress();

      protected:
         void runDelegate();

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };


   class SshGetFile : public SshReply {

      Q_OBJECT

      public:
         SshGetFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath) : SshReply(connection), 
            m_sourcePath(sourcePath), m_destinationPath(destinationPath) { }

      Q_SIGNALS:
         void copyProgress();

      protected:
         void runDelegate();

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };

} } // end namespace IQmol::Network

#endif
