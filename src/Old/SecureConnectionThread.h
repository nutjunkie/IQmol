#ifndef IQMOL_SECURECONNECTIONTHREAD_H
#define IQMOL_SECURECONNECTIONTHREAD_H
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

#include "Threaded.h"
#include <libssh2.h>


namespace IQmol {
namespace SecureConnection {

   class Connection;

   class Thread : public Threaded {

      Q_OBJECT 

      public:
         Thread(Connection* connection, int const timeout);

      protected:
         void waitsocket();
         LIBSSH2_SESSION* m_session;

      private:
         int m_socket;
   };


   
   /// Executes the given command over the Connection in a separate
   /// thread to avoid hanging the GUI thread during execution.
   class Exec : public Thread {

      Q_OBJECT

      public:
         Exec(Connection* connection, QString const& command, int timeout = 5000) 
          : Thread(connection, timeout), m_command(command) { }
          
		 /// Use this to run the command with blocking, this saves setting up
		 /// the signal/slot connections and is appropriate for when it makes
         /// no sense for the GUI to continue until the request has been resolved.
         /// Note this will hang the GUI thread until run() returns.
         QString block() {
            start();
            wait();
            return m_output;
         }

		 /// Returns the output of the command, or an empty string if
		 /// interrupted of if an error occurs.
         QString output() const { return m_output; }

      protected:
         void run();

      private:
         QString m_command;
         QString m_output;
   };


   /// Similar to SecureConnectionExec, except the return type is expected to
   /// be boolean.
   class Test : public Exec {

      Q_OBJECT

      public:
         Test(Connection* connection, QString const& command, int timeout = 5000) 
          : Exec(connection, command + " && echo success", timeout) { }
            
		 /// Use this to run the command with blocking, this saves setting up
		 /// the signal/slot connections and is appropriate for when it makes
         /// no sense for the GUI to continue until the request has been resolved.
         /// Note: - This will hang the GUI thread until run() returns.
         ///       - This deliberately masks SecureConnectionExec::block()
         bool block() {
            start();
            wait();
            return success();
         }

		 /// Returns if the command was run successfully or not
         bool success() const {
            return output().contains("success");
         }
   };



   class Push : public Thread {

      Q_OBJECT

      public:
         Push(Connection* connection, QString const& sourceFilePath,
            QString const& destinationFilePath, int timeout = 5000) 
             : Thread(connection, timeout), m_sourceFilePath(sourceFilePath),
               m_destinationFilePath(destinationFilePath), m_success(false) 
         { 
            if (m_destinationFilePath.startsWith("~/")) m_destinationFilePath.remove(0,2);
         }
          
         bool success() const { return m_success; }

		 /// Use this to run the copy with blocking, this saves setting up
		 /// the signal/slot connections and is appropriate for when it makes
         /// no sense to continue until the request has been resolved.  Note 
         /// this should not be called from the GUI thread as it will hang until
		 /// run() returns.
         bool block() {
            start();
            wait();
            return m_success;
         }

      Q_SIGNALS:
         void copyProgress();

      protected:
         void run();

      private:
         QString m_sourceFilePath;
         QString m_destinationFilePath;
         bool m_success;
   };



   class Pull : public Thread {

      Q_OBJECT

      public:
         Pull(Connection* connection, QString const& sourceFilePath,
            QString const& destinationFilePath, int timeout = 5000) 
            : Thread(connection, timeout), m_sourceFilePath(sourceFilePath),
              m_destinationFilePath(destinationFilePath), m_success(false) 
            {
               if (m_sourceFilePath.startsWith("~/")) m_sourceFilePath.remove(0,2);
            }

         bool success() const { return m_success; }

		 /// Use this to run the copy with blocking, this saves setting up
		 /// the signal/slot connections and is appropriate for when it makes
         /// no sense to continue until the request has been resolved.  Note 
         /// this should not be called from the GUI thread as it will hang until
		 /// run() returns.
         bool block() {
            start();
            wait();
            return m_success;
         }

      Q_SIGNALS:
         void copyProgress();

      protected:
         void run();

      private:
         QString m_sourceFilePath;
         QString m_destinationFilePath;
         bool m_success;
   };


} } // end namespace IQmol::SecureConnection

#endif
