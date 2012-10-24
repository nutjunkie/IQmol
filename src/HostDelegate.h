#ifndef IQMOL_HOSTDELEGATE_H
#define IQMOL_HOSTDELEGATE_H
/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include "Server.h"
#include <QObject>
#include <QString>


namespace IQmol {

   class Threaded;
   class JobInfo;

   /// Interface class for the delegates that handle low-level operations such
   /// as executing a command or copying files.  
   class HostDelegate : public QObject {

      Q_OBJECT

      public:
         enum FileFlag {
            Directory  = 0x001,
            Readable   = 0x002,
            Writable   = 0x004,
            Executable = 0x008,
            Create     = 0x010,
         };

         Q_DECLARE_FLAGS(FileFlags, FileFlag)

         HostDelegate(Server* server) : m_server(server) { }

         virtual bool connectServer() = 0;
         virtual void disconnectServer() = 0;
         virtual bool isConnected() = 0;

		 /// This prompts the user for a candidate for the working directory.
		 /// Note that if the directory exists, we still need to check if we
		 /// can over write it. Returns false if the user cancels the action.
         virtual bool getWorkingDirectoryFromUser(JobInfo*) = 0;

		 /// Executes a command on the server, the result is returned in the
		 /// Threaded::m_outputMessage member variable.
         virtual Threaded* exec(QString const& command) = 0;

         virtual Threaded* mkdir(QString const& dir) = 0;
         
         /// Wrapper which looks for a file or directory
         virtual Threaded* exists(QString const& filePath, FileFlags const) = 0;

         /// Copies a file from the local machine to the server
         virtual Threaded* push(QString const& sourcePath, QString const& destinationPath) = 0;

         /// Copies a file from the server to the local machine
         virtual Threaded* pull(QString const& sourcePath, QString const& destinationPath) = 0;

         /// Renames a file on the server
         virtual Threaded* move(QString const& sourcePath, QString const& destinationPath) = 0;

         /// Removes a file on the server
         virtual Threaded* remove(QString const& filePath) = 0;


         virtual Threaded* grep(QString const& string, QString const& filePath) = 0;
         virtual Threaded* checkOutputForErrors(QString const& filePath) = 0;

         /// Returns the working directory appropriate for the host type
         virtual QString workingDirectory(JobInfo*) = 0;


      protected:
		 // These allow us to access the protected member without having to
		 // make too many friends.
         QString name() const { return m_server->m_name; }
         QString userName() const { return m_server->m_userName; }
         QString hostAddress() const { return m_server->m_hostAddress; }
         Server::Authentication authentication() const { return m_server->m_authentication; }
         int port() const { return m_server->m_port; }

         Server* m_server; 
   };

   Q_DECLARE_OPERATORS_FOR_FLAGS(HostDelegate::FileFlags)


} // end namespace IQmol

#endif
