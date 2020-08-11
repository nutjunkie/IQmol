#ifndef IQMOL_NETWORK_H
#define IQMOL_NETWORK_H
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

#include <QString>

#ifdef WIN32
#include <winsock2.h>
#define in_addr_t u_long
#else
#include <netinet/in.h>
#endif


namespace IQmol {
namespace Network {

   enum ErrorT { NoError, Initialization, Timeout, Unknown };

   QString ToString(ErrorT const);
   ErrorT ToErrorT(QString const&);

   enum Status { Closed, Opened, Authenticated, Error };

   QString ToString(Status const);
   Status ToStatus(QString const&);

   enum ConnectionT { Local, SSH, SFTP, HTTP, HTTPS };

   QString ToString(ConnectionT const);
   ConnectionT ToConnectionT(QString const &);

   enum AuthenticationT { Anonymous = 0, Agent, HostBased, KeyboardInteractive, 
            Password, PublicKey };

   QString ToString(AuthenticationT const);
   AuthenticationT ToAuthenticationT(QString const&);



   /// Returns INADDR_NONE if there is a problem with the name lookup
   in_addr_t HostLookup(QString const& hostname);

   /// Performs a synchonous test on the network to determine if we are
   /// connected or not.
   bool TestNetworkConnection();


} } // end namespace IQmol::Network

#endif
