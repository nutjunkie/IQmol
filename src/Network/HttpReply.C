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
   if (m_status != Error) m_status = m_interrupt ? Interrupted : Finished;
   finished();
}


void HttpReply::errorSlot(QNetworkReply::NetworkError /*error*/)
{
   m_message = m_networkReply->errorString();
   m_status  = Error;
   finished();
}


void HttpReply::timeout()
{
   QLOG_TRACE() << "HttpReply timedout";
   m_networkReply->abort();
   m_status = TimedOut;
   finished();
}


// --------- HttpGet ---------

HttpGet::HttpGet(HttpConnection* connection,  QString const& sourcePath) 
   : HttpReply(connection), m_file(0)
{
   setSourceUrl(sourcePath);
}


HttpGet::HttpGet(HttpConnection* connection,  QString const& sourcePath, 
   QString const& destinationPath) : HttpReply(connection), m_file(0)
{
   setSourceUrl(sourcePath);

   m_message = destinationPath;
   m_file = new QFile(m_message);

   // We assume we have already okay'd this with the user
   if (m_file->exists() && m_file->isWritable()) m_file->remove();

   // We assume only text files for the time being
   if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
      m_message = "Failed to open file for write: " + destinationPath;
      m_status = Error;
      finished();
   }
}


void HttpGet::setSourceUrl(QString const& sourcePath)
{
 /*
   m_url.setScheme("http");
   m_url.setHost(m_hostname);
   m_url.setPath(path);
 */

   int port(m_connection->port());

   QString url = m_https ? "https://" : "http://";
   url += m_connection->hostname();
   if (port != 0) url += ":" + QString::number(port);
   url += "/" + sourcePath;

   m_sourceUrl.setUrl(url);
   qDebug() << "URL in HttpGet ctor" <<  m_sourceUrl;
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
   request.setUrl(m_sourceUrl);
   qDebug() << "Retrieving:" << m_sourceUrl;

   m_networkReply = m_connection->m_networkAccessManager->get(request);

   connect(m_networkReply, SIGNAL(readyRead()), &m_timer, SLOT(start()));

   if (m_file) {
      connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readToFile()));
      connect(m_networkReply, SIGNAL(finished()),  this, SLOT(closeFile()) );
   }else {
      connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readToString()));
   }

   connect(m_networkReply, SIGNAL(finished()),  this, SLOT(finishedSlot()) );
   connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(errorSlot(QNetworkReply::NetworkError)));

   m_timer.start();
}


void HttpGet::readToString()
{
   qint64 size(m_networkReply->bytesAvailable());
   m_message += m_networkReply->read(size);
}


void HttpGet::readToFile()
{
   qint64 size(m_networkReply->bytesAvailable());
   m_file->write(m_networkReply->read(size));
}

} } // end namespace IQmol::Network
