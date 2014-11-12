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

#include "LocalConnection.h"
#include "LocalReply.h"
#include <QFileInfo>
#include <QDir>


namespace IQmol {
namespace Network {

LocalConnection::LocalConnection() : Connection("localhost", 0)
{
}


LocalConnection::~LocalConnection()
{
}


void LocalConnection::open()
{
   m_status = Connection::Opened;
}


void LocalConnection::close()
{
   m_status = Connection::Closed;
}


void LocalConnection::authenticate(AuthenticationT const, QString const& /*username*/)
{
   m_status = Connection::Authenticated;
}


bool LocalConnection::exists(QString const& filePath)
{
   QFileInfo info(filePath);
   return info.exists();
}


bool LocalConnection::makeDirectory(QString const& path)
{
   QFileInfo info(path);

   if (!info.exists()) {
      QDir dir(QDir::root());
      return dir.mkpath(path);
   }

   if (!info.isDir()) {
      QDir dir(info.dir());
      QString fileName(info.fileName());
      qDebug() << "about to remove file " << fileName << " from directory " << dir.path();
      // remove file
      dir.remove(fileName);
      dir = QDir::root();
      return dir.mkpath(path);
   }

   return info.isWritable();
}

            
Reply* LocalConnection::execute(QString const& command)
{
   return new LocalExecute(this, command);
}


Reply* LocalConnection::getFile(QString const& sourcePath, QString const& destinationPath)
{
   return new LocalCopy(this, sourcePath, destinationPath);
}


Reply* LocalConnection::putFile(QString const& sourcePath, QString const& destinationPath)
{
   return new LocalCopy(this, sourcePath, destinationPath);
}

} } // end namespace IQmol::Network
