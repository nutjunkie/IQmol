#ifndef IQMOL_NETWORK_LOCALSESSION_H
#define IQMOL_NETWORK_LOCALSESSION_H
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

#include "Session.h"


namespace IQmol {
namespace Network {

   enum Authentication {Agent, PublicKey, HostBased, KeyboardInteractive, Vault,
      Prompt, Cookie }; 

   /// Base class for network sessions.
   class LocalSession : public Session{

      Q_OBJECT

      public:
         // required for serialization of the Server
         enum Type { Local = 0, SSH, HTTP }; 


         LocalSession() : m_session(session) { }
         QStringList authenticationMethods() const { return s_authentication; }

      public Q_SLOTS:

      private:
         QStringList s_authentication;
   };


} } // end namespace IQmol::Network

#endif
