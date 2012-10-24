#ifndef IQMOL_LOCALHOST_H
#define IQMOL_LOCALHOST_H
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

#include "HostDelegate.h"


namespace IQmol {

   class Threaded;

   class LocalHost : public HostDelegate {

      Q_OBJECT

      public:
         LocalHost(Server* server) : HostDelegate(server) { }

         bool connectServer() { return true; }
         void disconnectServer() { } 
         bool isConnected() { return true; }
         bool getWorkingDirectoryFromUser(JobInfo*);
         static bool getSaveDirectory(JobInfo*);
         
         Threaded* exec(QString const& command);
         Threaded* mkdir(QString const& dir);
         Threaded* exists(QString const& file, HostDelegate::FileFlags const flags);
         Threaded* push(QString const& sourcePath, QString const& destinationPath);
         Threaded* pull(QString const& sourcePath, QString const& destinationPath);
         Threaded* move(QString const& sourcePath, QString const& destinationPath);
         Threaded* remove(QString const& filePath);
         Threaded* grep(QString const& string, QString const& filePath);
         Threaded* checkOutputForErrors(QString const& filePath);
    

         QString workingDirectory(JobInfo* jobInfo) {
            return jobInfo->get(JobInfo::LocalWorkingDirectory);
         }
   };


} // end namespace IQmol

#endif
