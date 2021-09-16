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

HttpConnection::HttpConnection(QString const& hostname, int const port, bool const https)
 : Connection(hostname, port), m_networkAccessManager(0), m_secure(https)
{
}


ConnectionT HttpConnection::type() const
{
    return m_secure ? HTTPS : HTTP;
}


void HttpConnection::open()
{
   m_networkAccessManager = new QNetworkAccessManager(this);
   if (!m_networkAccessManager) {
      m_message = "Failed to open HTTP connection to " + m_hostname;
      m_status = Error;
      QLOG_DEBUG() << m_message;
   }else {
      m_status = Opened;
   }
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


void HttpConnection::authenticate(AuthenticationT const auth, QString& cookie)
{
  if (auth == Network::Password) {
     cookie = getJwt(cookie); // cookie is actually username in this case
  }else {
     if (cookie.isEmpty()) cookie = getCookie();
  }

  if (!cookie.isEmpty()) m_status = Authenticated;
}


QString HttpConnection::getJwt(QString const& userName)
{
   QString jwt;
   QLOG_TRACE() << "Attempting to obtain JWT from server" << userName << "@" <<  m_hostname;;

   QString msg("Password for ");
   msg += userName + "@" + m_hostname;

   QString password = getPasswordFromUser(msg);
   if (password.isEmpty()) return jwt;

   QStringMap headers;
   headers["Qcloud-Client-User"] = userName;
   headers["Qcloud-Client-Password"] = password;

   Reply* reply(execute("token", headers));

   QEventLoop loop;
   connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
   reply->start();
   loop.exec();

   // This is not getting caught (no thread saftey)
   if (reply->status() == Reply::TimedOut) {
      m_message = "Connection timeout to " + m_hostname;
      m_status = Error;
      QLOG_DEBUG() << m_message;
   }

   if (reply->status() == Reply::Finished) {
      QString msg(reply->message());
      QRegExp rx("Qcloud-Token::([0-9a-zA-Z\\-\\._]+)");
      if (msg.contains("Qcloud-Server-Status::OK") && rx.indexIn(msg,0) != -1) {
         jwt = rx.cap(1);
      }
      QLOG_DEBUG() << "Returning JWT:" << jwt;
   }else {
      m_message = reply->message();
   }

   return jwt;
}


QString HttpConnection::getCookie()
{
   QString cookie;
   qDebug() << "Obtaining cookie from HTTP server";
   QEventLoop loop;
   Reply* reply(execute("register"));
   connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
   reply->start();
   loop.exec();

   // This is not getting caught (no thread saftey)
   if (reply->status() == Reply::TimedOut) {
      m_message = "Connection timeout to " + m_hostname;
      m_status = Error;
      QLOG_DEBUG() << m_message;
   }

   if (reply->status() == Reply::Finished) {
      QString msg(reply->message());
      QRegExp rx("Qchemserv-Cookie::([0-9a-zA-Z\\-\\._]+)");
      if (msg.contains("Qchemserv-Status::OK") && rx.indexIn(msg,0) != -1) {
         cookie = rx.cap(1);
      }
      QLOG_DEBUG() << "Returning cookie:" << cookie;
   }

   reply->deleteLater();
   return cookie;
}


Reply* HttpConnection::execute(QString const& query)
{
   HttpGet* reply(new HttpGet(this, query));
   return reply;
}


Reply* HttpConnection::execute(QString const& query, QString const&)
{
   return execute(query);
}


Reply* HttpConnection::execute(QString const& query, QStringMap const& headers)
{
   QStringMap::const_iterator iter;
   HttpGet* reply(new HttpGet(this, query));
   for (iter = headers.begin(); iter != headers.end(); ++iter) {
       reply->setHeader(iter.key(), iter.value());
   }

   return reply;
}



Reply* HttpConnection::putFile(QString const& sourcePath, QString const& destinationPath)
{
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
