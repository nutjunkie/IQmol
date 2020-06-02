#ifndef IQMOL_NETWORK_LOCALCONNECTION_H
#define IQMOL_NETWORK_LOCALCONNECTION_H
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

#include "Connection.h"
#include <QThread>


namespace IQmol {
namespace Network {

   class LocalConnection : public Connection {

      Q_OBJECT

      public:
         LocalConnection();
         ~LocalConnection() { close(); }

         ConnectionT type() const { return Local; }

         void open();
         void close();
         void authenticate(AuthenticationT const, QString& username);

         bool blockingExecute(QString const& command, QString* message = 0);
         bool exists(QString const& filePath);
         bool makeDirectory(QString const& path);
         bool removeDirectory(QString const& path);

         Reply* execute(QString const& command);
         Reply* execute(QString const& command, QString const& workingDirectory);
         Reply* getFile(QString const& sourcePath, QString const& destinationPath);
         Reply* putFile(QString const& sourcePath, QString const& destinationPath);
         Reply* getFiles(QStringList const& fileList, QString const& destinationPath);
   };

} } // end namespace IQmol::Network

#endif
