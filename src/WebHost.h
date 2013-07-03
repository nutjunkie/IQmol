#ifndef IQMOL_WEBHOST_H
#define IQMOL_WEBHOST_H
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

#include "HostDelegate.h"
#include "JobInfo.h"


namespace IQmol {

   class WebHost : public HostDelegate {

      Q_OBJECT

      public:
         WebHost(Server* server) : HostDelegate(server), m_connected(false) { }
         ~WebHost() { }

         bool connectServer();
         void disconnectServer() { }
         bool isConnected() { return m_connected; } 
         // we don't allow the user to do this.
         bool getWorkingDirectoryFromUser(JobInfo*) { return true; }

         Threaded* exec(QString const& command);
         Threaded* mkdir(QString const& dir);                                       // no-op
         Threaded* exists(QString const& filePath, HostDelegate::FileFlags const);  // no-op
         Threaded* push(QString const& sourcePath, QString const& destinationPath); // no-op
         Threaded* pull(QString const& sourcePath, QString const& destinationPath);
         Threaded* move(QString const& sourcePath, QString const& destinationPath); // no-op
         Threaded* remove(QString const& filePath);                                 // no-op
         Threaded* grep(QString const& string, QString const& filePath);            
         Threaded* checkOutputForErrors(QString const& filePath);

         QString workingDirectory(JobInfo* jobInfo) {
            return jobInfo->get(JobInfo::RemoteWorkingDirectory);
         }

      private:
        bool m_connected;
   };


} // end namespace IQmol

#endif
