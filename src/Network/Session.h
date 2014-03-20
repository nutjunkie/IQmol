#ifndef IQMOL_NETWORK_SESSION_H
#define IQMOL_NETWORK_SESSION_H
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

#include "Task.h"
#include <QMutex>


namespace IQmol {
namespace Network {

   class Transaction;

   // Required for serialization of the Server
   enum Authentication { None = 0, Agent, PublicKey, HostBased, KeyboardInteractive, Vault,
      Prompt, Cookie }; 

   /// Base class for network sessions, which essentially characterize the type
   /// of connection.
   class Session : public QObject {

      Q_OBJECT

      public:
         enum Protocol { Local = 0, SSH, HTTP };  // Required for serialization of the Server

         static Session* Factory(Type const);

         Transaction* handshake() = 0;
         Transaction* authenticate() = 0;
         Transaction* command() = 0;
         Transaction* copy() = 0;

         QStringList authenticationMethods() const = 0;

      public Q_SLOTS:

      protected:
         Session() { }

      private:
         QMutex m_mutex;

         // No copying
         Session(Session const&);
         Session& operator=(Session const&);
   };


} } // end namespace IQmol::Network

#endif
