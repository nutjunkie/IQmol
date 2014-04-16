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
namespace Network {

ServerConfiguration::ServerConfiguration()
{
   m_name           = "Server";
   m_userName       = QString(qgetenv("USER"));
   m_executableName = "qcprog.exe";
   m_cgiRoot        = "/cgi/IQmol";

   m_protocol       = Local;
   m_queueSystem    = Basic;
   m_authentication = None;

   m_port           = 0;
   m_jobLimit       = Unlimited
   m_updateInterval = 10; 
}


void ServerConfiguration::setLocalDefaults() 
{
   m_submitCommand   = System::SubmitCommand();
   m_queryCommand    = System::QueryCommand();
   m_killCommand     = System::KillCommand();
   m_queueInfo       = "(unused)";
#ifdef Q_WS_WIN
   m_runFileTemplate = "(unused)";
#else
   m_runFileTemplate = "#! /bin/csh\nsource ~/.cshrc\nqchem ${JOB_NAME}.inp ${JOB_NAME}.out";
#endif

   m_protocol       = Local;
   m_queueSystem    = Basic;
   m_authentication = None;
}


void ServerConfiguration::setSshDefaults()
{
   m_protocol         = SSH;
   m_authentication   = Prompt;
   m_port             = 22;
   m_workingDirectory = "~/";
}


void ServerConfiguration::setHttpDefaults()
{
   m_hostAddress      = "www.iqmol.org";
   m_userName         = "guest";
   m_protocol         = HTTP;
   m_authentication   = Cookie;
   m_port             = 80;
   m_workingDirectory = "(unused)";

   m_submitCommand    = "submit.cgi cookie=${COOKIE} ";
   m_killCommand      = "kill.cgi   cookie=${COOKIE} jobID=${JOB_ID}";
   m_queryCommand     = "query.cgi  cookie=${COOKIE} jobID=${JOB_ID}";
   m_queueInfo        = "limits.cgi user=${USER}";
   m_runFileTemplate  = "(unused)";
}




QVariant ServerConfiguration::toQVariant() const
{
   QVariantMap map;
   map.insert("Name",            m_name);
   map.insert("Protocol",        ToString(m_protocol));
   map.insert("QueueSystem",     ToString(m_queueSystem));

   map.insert("HostAddress",      m_hostAddress);
   map.insert("Port",             m_port);
   map.insert("Authentication",   ToString(m_authentication));
   map.insert("UserName",         m_userName);
   map.insert("WorkingDirectory", m_workingDirectory);

   map.insert("ExecutableName",   m_executableName);
   map.insert("SubmitCommand",    m_submitCommand);
   map.insert("QueryCommand",     m_queryCommand);
   map.insert("QueueInfo",        m_queueInfo);
   map.insert("KillCommand",      m_killCommand);
   map.insert("RunFileTemplate",  m_runFileTemplate);

   map.insert("UpdateInterval",   m_updateInterval);
   map.insert("JobLimit",         m_jobLimit);

   map.insert("Cookie",           m_cookie);
   return map;
}


void ServerConfiguration::fromQVariant(QVariant const& map) 
{
   // Name
   if (map.contains("Name") && !map.value("Name").isEmpty()) {
      m_name = map.value("Name")
   }else {
      
   }
}

} } // end namespace IQmol::Network

#endif
