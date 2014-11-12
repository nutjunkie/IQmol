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

   class LocalReply : public Reply {

      Q_OBJECT

      public:
         LocalReply(LocalConnection* connection) : m_connection(connection) { }
         virtual ~LocalReply() { }

      protected:
         LocalConnection* m_connection;
   };


   class LocalExecute : public LocalReply {

      Q_OBJECT

      public:
         LocalExecute(LocalConnection* connection, QString const& command) :
            LocalReply(connection), m_command(command) { }

      protected Q_SLOT:
         void run();

      private:
         QString m_command;
   };


   class LocalCopy : public LocalReply {

      Q_OBJECT

      public:
         LocalCopy(LocalConnection* connection, QString const& sourcePath, 
            QString const& destinationPath) : LocalReply(connection), 
            m_sourcePath(sourcePath), m_destinationPath(destinationPath) { }

      protected Q_SLOT:
         void run();

      private:
         QString m_sourcePath;
         QString m_destinationPath;
   };

} } // end namespace IQmol::Network

#endif
