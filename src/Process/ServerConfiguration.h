#ifndef IQMOL_PROCESS_SERVERCONFIGURATION_H
#define IQMOL_PROCESS_SERVERCONFIGURATION_H
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

#include <QVariant>
#include "YamlNode.h"
#include "SshConnection.h"



namespace IQmol {
namespace Process2 {

   class ServerConfigurationDialog;

   class ServerConfiguration {

      friend class ServerConfigurationDialog;
      typedef Network::Connection::AuthenticationT AuthenticationT;

      public:
         enum FieldT { ServerName = 0, Connection, QueueSystem, HostAddress, Port,
                       Authentication, UserName, WorkingDirectory,
                       Submit, Query, QueueInfo, Kill,
                       UpdateInterval, JobLimit, 
                       RunFileTemplate, Cookie, MaxFieldT };

         enum ConnectionT { Local, SSH, HTTP, HTTPS };

         enum QueueSystemT { Basic, PBS, SGE, Web };

         typedef QMap<FieldT, QVariant> ConfigMap;

         static QString toString(FieldT const);
         static QString toString(ConnectionT const);
         static QString toString(QueueSystemT const);
         static QString toString(AuthenticationT const);

         static FieldT toFieldT(QString const& field);
         static ConnectionT toConnectionT(QString const& connection);
         static QueueSystemT toQueueSystemT(QString const& queueSystem);
         static AuthenticationT toAuthenticationT(QString const& authentication);

         ServerConfiguration(); 
         ServerConfiguration(ServerConfiguration const&);
         explicit ServerConfiguration(YAML::Node const&);
         explicit ServerConfiguration(QVariant const&);

         QString value(FieldT const) const;
         void setValue(FieldT const, QVariant const& value);

         ConnectionT connection() const;
         QueueSystemT queueSystem() const;
         AuthenticationT authentication() const;
         int port() const;
         int updateInterval() const;

         bool isWebBased() const {
            return (connection() == HTTP || connection() == HTTPS);
         }

		 // These are to facilitate saving the configuration in the user Preferences.
         QVariant toQVariant() const;
         void fromQVariant(QVariant const&);
         void dump() const;

         ServerConfiguration& operator=(ServerConfiguration const& that);

      protected:
         void setDefaults(ConnectionT const);
         void setDefaults(QueueSystemT const);
         Data::YamlNode toYamlNode() const;
         
      private:
         ConfigMap m_configuration;
         void copy(ServerConfiguration const&);
         void fromQVariantMapLegacy(QVariantMap const&);
         void fromQVariantMap(QVariantMap const&);
         void fromYamlNode(YAML::Node const&);
   };

} } // end namespace IQmol::Process

#endif
