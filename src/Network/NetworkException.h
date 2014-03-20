#ifndef IQMOL_NETWORK_EXCEPTION_H
#define IQMOL_NETWORK_EXCEPTION_H
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

#include "Exception.h"
#include <libssh2.h>


namespace IQmol {
namespace Network {

   class AuthenticationError : public IQmol::Exception {
      public:
         InvalidAddress() : Exception("Authentication error") { }
   }

   class InvalidAddress : public IQmol::Exception {
      public:
         InvalidAddress() : Exception("Host not found") { }
   }

   class Timeout : public IQmol::Exception {
      public:
         Timeout() : Exception("Connection timed out") { }
   };

} } // end namespace IQmol::SecureConnection

#endif
