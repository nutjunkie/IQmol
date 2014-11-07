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

#include "HttpReply.h"
#include "QsLog.h"
#include <QFile>


namespace IQmol {
namespace Network {

HttpReply::HttpReply(HttpConnection* connection) : m_connection(connection), m_networkReply(0),
   m_https(connection->secure())
{ 
   m_timeout = m_connection->timeout();
   m_timer.setInterval(m_timeout);
   m_timer.setSingleShot(true);

   connect(this, SIGNAL(finished()), &m_timer, SLOT(stop()));
   connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}


HttpReply::~HttpReply()
{
   QLOG_TRACE() << "Deleting HttpReply";
   if (m_networkReply) m_networkReply->deleteLater(); 
}


void HttpReply::finishedSlot()
{
   if (m_message.isEmpty()) {
      m_message = headerAsString(); 
   }

   if (m_status != Error) m_status = m_interrupt ? Interrupted : Finished;
   finished();
}


void HttpReply::errorSlot(QNetworkReply::NetworkError /*error*/)
{
   m_message = m_networkReply->errorString();
   m_status  = Error;
   finished();
}


void HttpReply::readToString()
{
   qint64 size(m_networkReply->bytesAvailable());
   //m_message += m_networkReply->read(size);
   m_message += QString(m_networkReply->readAll());
   qDebug() << "Reading" << size << " bytes to message:" << m_message;
}


void HttpReply::timeout()
{
   QLOG_TRACE() << "HttpReply timeout" << m_connection->hostname();
   m_networkReply->abort();
   m_status = TimedOut;
   finished();
}


void HttpReply::setUrl(QString const& path)
{
  // this should be moved up to the inheritance tree
 /*
   m_url.setScheme("http");
   m_url.setHost(m_hostname);
   m_url.setPath(path);
 */

   int port(m_connection->port());

   QString url = m_https ? "https://" : "http://";
   url += m_connection->hostname();
   if (port != 80) url += ":" + QString::number(port);
   if (!path.isEmpty()) url += "/" + path;

   m_url.setUrl(url);
   qDebug() << "URL in HttpPost ctor" <<  m_url;
}


QString HttpReply::header(QString const& headerName)
{
   QByteArray data(headerName.toLatin1());
   QString header;
   
   if (m_networkReply->hasRawHeader(data)) {
       header = QString(m_networkReply->rawHeader(data));
   }else {
      QLOG_DEBUG() << "Header not found:" << data;
   }

   return header;
}


QString HttpReply::headerAsString()
{
   QString header;
   QList<QByteArray> headers(m_networkReply->rawHeaderList());
   QList<QByteArray>::iterator iter;
   for (iter = headers.begin(); iter != headers.end(); ++iter) {
       header += QString(*iter) +"::" + QString(m_networkReply->rawHeader(*iter));
       header += "\n";
   }
   return header;
}


void HttpReply::dumpHeader()
{
   qDebug() << headerAsString();
}


// --------- HttpGet ---------

HttpGet::HttpGet(HttpConnection* connection,  QString const& sourcePath) 
 : HttpReply(connection), m_file(0)
{
   setUrl(sourcePath);
}


HttpGet::HttpGet(HttpConnection* connection,  QString const& sourcePath, 
   QString const& destinationPath) : HttpReply(connection), m_file(0)
{
   setUrl(sourcePath);

   m_message = destinationPath;
   m_file = new QFile(destinationPath);

   // We assume we have already okay'd this with the user
   if (m_file->exists() && m_file->isWritable()) m_file->remove();

   // We assume only text files for the time being
   if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
      m_message = "Failed to open file for write: " + destinationPath;
      m_status = Error;
      finished();
   }
}


void HttpGet::closeFile()
{
    m_file->close();
    delete m_file;
}


void HttpGet::run()
{
   if (!m_connection->m_networkAccessManager) {
      m_message = "Invalid Network Access Manager";
      m_status = Error;
      finished();
      return;
   }

   m_status = Running;
   QNetworkRequest request;
   request.setUrl(m_url);
   qDebug() << "Retrieving:" << m_url;

   m_networkReply = m_connection->m_networkAccessManager->get(request);

   connect(m_networkReply, SIGNAL(readyRead()), &m_timer, SLOT(start()));

   if (m_file) {
      connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readToFile()));
      connect(m_networkReply, SIGNAL(finished()),  this, SLOT(closeFile()) );
   }else {
      connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readToString()));
   }

   connect(m_networkReply, SIGNAL(finished()),  this, SLOT(dumpHeader()) );
   connect(m_networkReply, SIGNAL(finished()),  this, SLOT(finishedSlot()) );
   connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(errorSlot(QNetworkReply::NetworkError)));

   m_timer.start();
}


void HttpGet::readToFile()
{
   qint64 size(m_networkReply->bytesAvailable());
   m_file->write(m_networkReply->read(size));
}


// --------- HttpPost ---------

HttpPost::HttpPost(HttpConnection* connection, QString const& path,  
   QStringList const& postData) : HttpReply(connection), m_postData(postData)
{
   setUrl(path);
}


void HttpPost::run()
{
   if (!m_connection->m_networkAccessManager) {
      m_message = "Invalid Network Access Manager";
      m_status = Error;
      finished();
      return;
   }

   m_status = Running;
   QNetworkRequest request;

   request.setUrl(m_url);
   request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain; charset=UTF-8");

   QByteArray data(m_postData.join("\n").toLatin1());

qDebug() << "POST:" << data;

   m_networkReply = m_connection->m_networkAccessManager->post(request, data);

   QList<QByteArray> headers(m_networkReply->request().rawHeaderList());

   QList<QByteArray>::iterator iter;
   for (iter = headers.begin(); iter != headers.end(); ++iter) {
       qDebug() << "HEADER:" << *iter << m_networkReply->request().rawHeader(*iter);
   }

   connect(m_networkReply, SIGNAL(readyRead()), &m_timer, SLOT(start()));
   connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readToString()));
   connect(m_networkReply, SIGNAL(finished()),  this, SLOT(finishedSlot()) );
   connect(m_networkReply, SIGNAL(finished()),  this, SLOT(dumpHeader()) );
   connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(errorSlot(QNetworkReply::NetworkError)));

   m_timer.start();
}


} } // end namespace IQmol::Network
