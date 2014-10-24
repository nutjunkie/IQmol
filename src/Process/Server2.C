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

#include "Server2.h"
#include <QDebug>


namespace IQmol {
namespace Process2 {

Server::Server(ServerConfiguration const& configuration) : m_configuration(configuration)
{
   qDebug() << "Constructing a New Server with configuration";
   m_configuration.dump();
}


QString Server::name() const
{
   return m_configuration.name();
}


QStringList Server::tableFields() const
{
   QStringList fields;
   fields.append(m_configuration.name());
   fields.append(m_configuration.value(ServerConfiguration::HostAddress).toString());
   fields.append(ServerConfiguration::toString(m_configuration.connection()));
   //fields.append(m_configuration.value(ServerConfiguration::UserName).toString());
   fields.append(m_configuration.value(ServerConfiguration::Submit).toString());

   return fields;
}


} } // end namespace IQmol::Process
