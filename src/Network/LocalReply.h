#ifndef IQMOL_NETWORK_LOCALREPLY_H
#define IQMOL_NETWORK_LOCALREPLY_H
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

   class LocalConnection;

   class SshReply : public Reply {

      Q_OBJECT

      public:
         LocalReply(LocalConnection* connection, unsigned timeout);
         virtual ~SshReply() { }

         void start() { startSignal(); }

      Q_SIGNALS:
         void startSignal();

      protected:
         virtual void run() = 0;
         LocalConnection* m_connection;
         unsigned m_timeout;

      private Q_SLOTS:
         void runSlot();
   };


   class SshExecute : public SshReply {

      Q_OBJECT

      public:
         SshExecute(SshConnection* connection, QString const& command, unsigned const timeout) :
            SshReply(connection, timeout), m_command(command) { }

      protected:
         void run();

      private:
         QString m_command;
   };


   class SshSendFile : public SshReply {

      Q_OBJECT

      public:
         SshSendFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath, unsigned const timeout) : 
            SshReply(connection, timeout), m_sourcePath(sourcePath),    
            m_destinationPath(destinationPath) { }

      Q_SIGNALS:
         void copyProgress();

      protected:
         void run();

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };


   class SshReceiveFile : public SshReply {

      Q_OBJECT

      public:
         SshReceiveFile(SshConnection* connection, QString const& sourcePath, 
            QString const& destinationPath, unsigned const timeout) : 
            SshReply(connection, timeout), m_sourcePath(sourcePath),    
            m_destinationPath(destinationPath) { }

      Q_SIGNALS:
         void copyProgress();

      protected:
         void run();

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };


} } // end namespace IQmol::Network

#endif
