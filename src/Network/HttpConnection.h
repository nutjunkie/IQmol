#ifndef IQMOL_NETWORK_HTTPCONNECTION_H
#define IQMOL_NETWORK_HTTPCONNECTION_H
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

#include "Connection.h"


class QNetworkAccessManager;

namespace IQmol {
namespace Network {

   class HttpReply;
   class HttpGet;

   class HttpConnection : public Connection {

      Q_OBJECT

      friend class HttpPageRequest;
      friend class HttpGet;
      friend class HttpPost;

      public:
         HttpConnection(QString const& hostAddress, int const port = 80);
         ~HttpConnection();

         void setSecure(bool secure) { m_secure = secure; }
         bool secure() const { return m_secure; }

         void open();
         void close();
         void authenticate(AuthenticationT const, QString const& cooke);
         QString obtainCookie();

         Reply* execute(QString const& query);
         Reply* putFile(QString const& sourcePath, QString const& destinationPath);
         Reply* getFile(QString const& sourcePath, QString const& destinationPath);
         Reply* get(QString const& query) { return execute(query); }
        
         Reply* post(QString const& path, QStringList const&);

      protected:
         QNetworkAccessManager* m_networkAccessManager;
         bool m_secure; 
   };

} } // end namespace IQmol::Network

#endif
