#ifndef IQMOL_NETWORK_LOCALCONNECTION_H
#define IQMOL_NETWORK_LOCALCONNECTION_H
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

#include "Connection.h"
#include <QThread>


namespace IQmol {
namespace Network {

   class LocalConnection : public Connection {

      Q_OBJECT

      public:
         LocalConnection();
         ~LocalConnection();

         void open();
         void close();

         Reply* execute(QString const& command, unsigned const timeout = TIMEOUT);
         Reply* sendFile(QString const& command, unsigned const timeout = TIMEOUT);
         Reply* receiveFile(QString const& command, unsigned const timeout = TIMEOUT);
   };

} } // end namespace IQmol::Network

#endif
