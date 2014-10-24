#ifndef IQMOL_PROCESS_SERVER_H
#define IQMOL_PROCESS_SERVER_H
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

#include "ServerConfiguration.h"

namespace IQmol {
namespace Process2 {

   class Server {

      friend class ServerRegistry;
      friend class ServerConfigurationListDialog;

      public:
         ServerConfiguration const& configuration() const { return m_configuration; }
         QString name() const;

         QStringList tableFields() const;

      protected:
         Server(ServerConfiguration const&);
         Server();
         ~Server() { }

         // this will probably have to be move up, or pass-through functions created
         ServerConfiguration& configuration() { return m_configuration; }

      private:
        ServerConfiguration m_configuration;
   };

} } // end namespace IQmol::Process

#endif
