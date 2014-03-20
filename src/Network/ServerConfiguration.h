#ifndef IQMOL_NETWORK_SERVERCONFIGURATION_H
#define IQMOL_NETWORK_SERVERCONFIGURATION_H
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

#include "Network.h"
#include "Serialization.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>


namespace IQmol {
namespace Network {

   typedef boost::archive::xml_oarchive OutputArchive;
   typedef boost::archive::xml_iarchive InputArchive;

   typedef QMap<QString, QString> QStringMap;

   class ServerConfigurationDialog;

   /// POD class containing all the configuration for setting up a Server.
   class ServerConfiguration {

      friend class ServerConfigurationDialog;

      public:
         ServerConfiguration(); 

		 // These are to facilitate saving the configuration in the user Preferences.
         QVariant toQVariant() const;
         void fromQVariant(QVariant const&);

         void toFile(QString const& fileName) const;
         void fromFile(QString const& fileName);

         void setLocalDefaults();
         void setSshDefaults();
         void setHttpDefaults();
         
      private:
         QString m_name;
         QString m_hostAddress;
         QString m_userName;
         QString m_workingDirectory;
         QString m_executableName;
         QString m_submitCommand;
         QString m_queryCommand;
         QString m_killCommand;
         QString m_queueInfo;
         QString m_runFileTemplate;
         QString m_cgiRoot;
         QString m_cookie;

         Protocol m_protocol;
         QueueSystem m_queueSystem;
         Authentication m_authentication;

         unsigned m_port;
         unsigned m_jobLimit;
         unsigned m_updateInterval;
   };

} } // end namespace IQmol::Network

#endif
