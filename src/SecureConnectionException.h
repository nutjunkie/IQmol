#ifndef IQMOL_SECURECONNECTIONEXCEPTION_H
#define IQMOL_SECURECONNECTIONEXCEPTION_H
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

#include "QsLog.h"
#include <libssh2.h>
#include <exception>


namespace IQmol {
namespace SecureConnection {

   /// Exceptions are thrown on non-recoverable errors.  Permission problems
   /// should be covered by return codes
   class Exception : public std::exception { 
      public:
         /// This is a debug version of the exception used for determining what
         /// error codes are returned from libssh2.
         Exception(LIBSSH2_SESSION* session, QString const& message = "") {
            char* errorMessage;
            int length;
            int code = libssh2_session_last_error(session, &errorMessage, &length, 0);

            QString msg;
            if (!message.isEmpty()) msg = message + "\n";
            msg += "libssh2 error code " + QString::number(code) + "\n";
            msg += QString(errorMessage) + "\n";
            setMessage(msg);
         } 

         Exception(QString const& message) { 
            setMessage(message);
         } 

         ~Exception() throw () { }

         virtual const char* what() const throw() {
            return m_message.toAscii().data();
         }

      private:
         void setMessage(QString const& message) {
            m_message = message;
            QLOG_ERROR() << m_message;
         }
         QString m_message;
   };

         

   class Timeout : public Exception {
      public:
         Timeout() : Exception("Connection timed out") { }
   };

} } // end namespace IQmol::SecureConnection

#endif
