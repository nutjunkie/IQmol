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
#include <QFile>
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
   killThread();
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
   
   m_status = Closed;
}


void HttpConnection::authenticate(AuthenticationT const, QString& cookie)
{
  if ( cookie.isEmpty()) cookie = obtainCookie();
  if (!cookie.isEmpty()) m_status = Authenticated;
}


QString HttpConnection::obtainCookie()
{
   QString cookie;
   qDebug() << "Obtaining cookie from HTTP server";
   QEventLoop loop;
   Reply* reply(execute("register"));
   connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
   reply->start();
   loop.exec();

   if (reply->status() == Reply::TimedOut) throw NetworkTimeout();

   if (reply->status() == Reply::Finished) {
      QString msg(reply->message());
      QRegExp rx("Qchemserv-Cookie::([0-9a-zA-Z\\-_]+)");
      if (msg.contains("Qchemserv-Status::OK") && rx.indexIn(msg,0) != -1) {
         cookie = rx.cap(1);
      }
      qDebug() << "Returning cookie:" << cookie;
   }

   reply->deleteLater();
   return cookie;
}


Reply* HttpConnection::execute(QString const& query)
{
   HttpGet* reply(new HttpGet(this, query));
   return reply;
}


Reply* HttpConnection::putFile(QString const& sourcePath, QString const& destinationPath)
{
   qDebug() << "Should be catching exceptions"; 
   QFile file(sourcePath);
   QByteArray buffer;

   if (file.open(QIODevice::ReadOnly)) {
      buffer = file.readAll();
      file.close();
   }
   
   HttpPost* reply(new HttpPost(this, destinationPath, QString(buffer)));
   return reply;
}


Reply* HttpConnection::getFiles(QStringList const& fileList, QString const& destinationPath)
{
   HttpGetFiles* reply(new HttpGetFiles(this, fileList, destinationPath));
   return reply;
}

             
Reply* HttpConnection::getFile(QString const& sourcePath, QString const& destinationPath)
{
   HttpGet* reply(new HttpGet(this, sourcePath, destinationPath));
   return reply;
}


Reply* HttpConnection::post(QString const& path, QString const& postData)
{
   HttpPost* reply(new HttpPost(this, path, postData));
   return reply;
}

} } // end namespace IQmol::Network
