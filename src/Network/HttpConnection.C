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

#include "HttpConnection.h"
#include "HttpReply.h"
#include "Exception.h"
#include "QsLog.h"
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>


namespace IQmol {
namespace Network {

HttpConnection::HttpConnection(QString const& hostname, int const port) :
   Connection(hostname, port), m_networkAccessManager(0), m_secure(false)
{
}


HttpConnection::~HttpConnection()
{
   close();
}


void HttpConnection::open()
{
   // If m_secure
   m_networkAccessManager = new QNetworkAccessManager(this);
   if (!m_networkAccessManager) throw Exception("HTTP open connection failed");
   m_status = Opened;
}


void HttpConnection::close()
{
   if (m_networkAccessManager) {
      disconnect(m_networkAccessManager);
      delete m_networkAccessManager;
      m_networkAccessManager = 0;
   }
   
   m_status = Connection::Closed;
}


void HttpConnection::authenticate(AuthenticationT const, QString const&)
{
  m_status = Connection::Authenticated;
}


QString HttpConnection::obtainCookie()
{
   QString cookie;
   qDebug() << "Obtaining cookie from HTTP server";
   Reply* reply(execute("register"));
   QEventLoop loop;
   connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
   reply->start();
   loop.exec();

   if (reply->status() == Reply::Finished) {
      QLOG_DEBUG() << "=============================";
      QLOG_DEBUG() << reply->message();
      QLOG_DEBUG() << "=============================";
   }

   reply->deleteLater();
   return cookie;
}


Reply* HttpConnection::execute(QString const& query)
{
   HttpGet* reply(new HttpGet(this, query));
   reply->start();
   return reply;
}


Reply* HttpConnection::putFile(QString const& sourcePath, QString const& destinationPath)
{
   // souce Path is local and may need to be loaded into a buffer
   // destination path will be a URL waiting to accept the text
   QLOG_WARN() << "Returning bogus sendFile HttpGet response";
   HttpGet* reply(new HttpGet(this, sourcePath, destinationPath));
   reply->start();
   return reply;
}
             

Reply* HttpConnection::getFile(QString const& sourcePath, QString const& destinationPath)
{
   HttpGet* reply(new HttpGet(this, sourcePath, destinationPath));
   reply->start();
   return reply;
}


Reply* HttpConnection::post(QString const& path, QStringList const& postData)
{
   HttpPost* reply(new HttpPost(this, path, postData));
   reply->start();
   return reply;
}

} } // end namespace IQmol::Network
