#ifndef IQMOL_NETWORK_HTTPREPLY_H
#define IQMOL_NETWORK_HTTPREPLY_H
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

#include "Reply.h"
#include "HttpConnection.h"
#include <QNetworkReply>
#include <QTimer>


class QNetworkReply;
class QFile;

namespace IQmol {
namespace Network {

   class HttpReply : public Reply {

      Q_OBJECT

      public:
         HttpReply(HttpConnection*);
         virtual ~HttpReply();

      protected:
         HttpConnection* m_connection;
         unsigned m_timeout;
         QNetworkReply* m_networkReply;  // We take ownership of this
         QTimer m_timer;
         bool m_https;
       
      protected Q_SLOTS:
         void finishedSlot();
         void errorSlot(QNetworkReply::NetworkError);

      private Q_SLOTS:
         void timeout();
   };


   class HttpGet : public HttpReply {

      Q_OBJECT

      public:
         HttpGet(HttpConnection*, QString const& sourcePath);
         HttpGet(HttpConnection*, QString const& sourcePath, QString const& destinationPath);

      protected Q_SLOTS:
         void run();

      private Q_SLOTS:
          void readToString();
          void readToFile();
          void closeFile();

      private:
         void setSourceUrl(QString const& sourcePath);
         QUrl   m_sourceUrl;
         QFile* m_file;
   };


} } // end namespace IQmol::Network

#endif
