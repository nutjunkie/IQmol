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

#include "ServerRegistry2.h"
#include "Preferences.h"
#include "Server2.h"
#include "QMsgBox.h"
#include <exception>
#include <cstdlib>
#include <QDebug>


namespace IQmol {
namespace Process2 {

ServerRegistry* ServerRegistry::s_instance = 0;
QList<Server*>  ServerRegistry::s_servers;
QList<Server*>  ServerRegistry::s_deletedServers;


ServerRegistry& ServerRegistry::instance() 
{
   if (s_instance == 0) {
      s_instance = new ServerRegistry();
      loadFromPreferences();
      atexit(ServerRegistry::destroy);
   }
   return *s_instance;
}


QStringList ServerRegistry::availableServers() const
{
   QStringList list;
   QList<Server*>::iterator iter;
   for (iter = s_servers.begin(); iter != s_servers.end(); ++iter) {
       list.append((*iter)->name());
   }
   return list;
}


Server* ServerRegistry::addServer(ServerConfiguration& config)
{
   QString name(config.name());
   int count(0);

   while (find(name)) {
      ++count;
      name = config.name() + "_" + QString(count);
   }

   config.setValue(ServerConfiguration::ServerName, name);

   Server* server(new Server(config));
   s_servers.append(server);
   saveToPreferences();

   return server;
}


Server* ServerRegistry::find(QString const& serverName) const
{
   int index(indexOf(serverName));
   if (index >= 0) return s_servers[index];
   return 0;
}


void ServerRegistry::remove(QString const& serverName)
{
   int index(indexOf(serverName));
   if (index >= 0) s_deletedServers.append(s_servers.takeAt(index));
}


void ServerRegistry::remove(Server* server)
{
   int index(s_servers.indexOf(server));
   if (index >= 0) s_deletedServers.append(s_servers.takeAt(index));
}



void ServerRegistry::moveUp(QString const& serverName)
{
   int index(indexOf(serverName));
   if (index > 0) s_servers.swap(index, index-1);
}


void ServerRegistry::moveDown(QString const& serverName)
{
   int index(indexOf(serverName));
   if (index < s_servers.size() -1) s_servers.swap(index, index+1);
}


int ServerRegistry::indexOf(QString const& serverName) 
{
   for (int i = 0; i < s_servers.size(); ++i) {
      if (s_servers[i]->name() == serverName) return i;
   }
   return -1;
}


void ServerRegistry::destroy() 
{
   QList<Server*>::iterator iter;
   for (iter = s_servers.begin(); iter != s_servers.end(); ++iter) {
       delete (*iter);
   }
   for (iter = s_deletedServers.begin(); iter != s_deletedServers.end(); ++iter) {
       delete (*iter);
   }
}


void ServerRegistry::loadFromPreferences()
{
   Server* server;
   QVariantList list(Preferences::ServerConfigurationList());
   QVariantList::iterator iter;

   try {
      for (iter = list.begin(); iter != list.end(); ++iter) {
          server = new Server(ServerConfiguration(*iter));
          s_servers.append(server);
      }
   } catch (std::exception& err) {
      QString msg("Problem loading servers from Preferences file:\n");
      msg += err.what();
      QMsgBox::warning(0, "IQmol", msg);
      return;
   }
}


void ServerRegistry::saveToPreferences()
{
   QVariantList list;
   QList<Server*>::const_iterator iter;
   for (iter = s_servers.begin(); iter != s_servers.end(); ++iter) {
qDebug() << "Saving configuration";
(*iter)->configuration().dump();
       list.append((*iter)->configuration().toQVariant());
   }
   Preferences::ServerConfigurationList(list);
}

} } // end namespace IQmol::Process
