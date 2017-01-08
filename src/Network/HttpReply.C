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

#include "HttpReply.h"
#include "QsLog.h"
#include <QFile>


namespace IQmol {
namespace Network {

HttpReply::HttpReply(HttpConnection* connection) : m_connection(connection), m_networkReply(0),
   m_https(connection->isSecure())
{ 
   m_timeout = m_connection->timeout();
   m_timer.setInterval(m_timeout);
   //m_timer.setSingleShot(true);

   connect(this, SIGNAL(finished()), &m_timer, SLOT(stop()));
   connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}


HttpReply::~HttpReply()
{
   // The QNetworkAcessManager takes care of this.
   // if (m_networkReply) m_networkReply->deleteLater(); 
}


void HttpReply::finishedSlot()
{
   QLOG_DEBUG() << "HttpReply finished, HEADER:\n" << headerAsString();

   QString status(headerValue("Qchemserv-Status"));

   if (status.contains("OK")) {
      if (m_status != Error) m_status = m_interrupt ? Interrupted : Finished;
      if (m_message.isEmpty()) {
         m_message = headerAsString(); 
      }
   }else if (status.contains("ERROR")) {
      m_status  = Error;
      m_message = headerValue("Qchemserv-Error");
   }else  {
      m_status  = Error;
      m_message = "QChem server temporarily unavailable";
   }

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
   m_message += m_networkReply->read(size);
   qDebug() << "Reading" << size << " bytes to message:" << m_message;
}


void HttpReply::interrupt()
{
   m_interrupt = true;
   QLOG_TRACE() << "HttpReply interrupted" << m_connection->hostname();
   m_networkReply->abort();
   m_status = Interrupted;
   interrupted();
   finished();
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
 /*
   m_url.setScheme("http");
   m_url.setHost(m_hostname);
   m_url.setPath(path);
 */

   int port(m_connection->port());

   QString url(path);
   url.prepend("/");
   if (port != 80) url.prepend(":" + QString::number(port));
   url.prepend(m_connection->hostname());
   url.replace("//", "/");
   url.prepend(m_https ? "https://" : "http://");

   m_url.setUrl(url);
   QLOG_DEBUG() << "Setting URL to" <<  m_url;
}


QString HttpReply::headerValue(QString const& headerName)
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
    m_file->flush();
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
   QLOG_DEBUG() << "Retrieving:" << m_url;

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


void HttpGet::readToFile()
{
   qint64 size(m_networkReply->bytesAvailable());
   //qDebug() << "Reading " << size << " bytes";
   copyProgress();
   m_file->write(m_networkReply->read(size));
}


// --------- HttpGetFiles---------

HttpGetFiles::HttpGetFiles(HttpConnection* connection, QStringList const& fileList, 
   QString const& destinationPath) : HttpReply(connection), m_fileList(fileList), 
   m_destinationPath(destinationPath), m_totalReplies(0), m_allOk(true)
{
}


void HttpGetFiles::run()
{
   QRegExp rx("file=(.*)");
   m_status = Running;

   QStringList::iterator iter;
   for (iter = m_fileList.begin(); iter != m_fileList.end(); ++iter) {
       QString source(*iter);
       if (rx.indexIn(source, 0) != -1) {
          QString destination(m_destinationPath);
          destination += "/" + rx.cap(1);
          HttpGet* reply(new HttpGet(m_connection, source, destination));
          m_replies.append(reply);
          connect(this, SIGNAL(interrupted()), this, SLOT(interrupt()));
          connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
          //connect(reply, SIGNAL(copyProgress()), this, SIGNAL(copyProgress()));
       }
   }

   QList<HttpGet*> replies(m_replies);
   m_totalReplies = replies.size();
   QList<HttpGet*>::iterator reply;
   for (reply = replies.begin(); reply != replies.end(); ++reply) {
       (*reply)->run();
   }
}


void HttpGetFiles::replyFinished()
{
   HttpGet* reply(qobject_cast<HttpGet*>(sender()));
   m_replies.removeAll(reply);
   double progress(m_totalReplies-m_replies.size());
   if (m_totalReplies > 0) copyProgress(progress/m_totalReplies);
   m_allOk = m_allOk && reply->status() == Finished;
   reply->deleteLater();

   if (m_replies.isEmpty()) {

      if (m_allOk) {
         m_status = m_interrupt ? Interrupted : Finished;
      }else {
         m_status = Error;
      }

      finished();
   }
}


// --------- HttpPost ---------

HttpPost::HttpPost(HttpConnection* connection, QString const& path,  
   QString const& postData) : HttpReply(connection), m_postData(postData)
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

   QByteArray data(m_postData.toLatin1());


   m_networkReply = m_connection->m_networkAccessManager->post(request, data);

   QList<QByteArray> headers(m_networkReply->request().rawHeaderList());

   QList<QByteArray>::iterator iter;
   for (iter = headers.begin(); iter != headers.end(); ++iter) {
       qDebug() << "HEADER:" << *iter << m_networkReply->request().rawHeader(*iter);
   }

   qDebug() << "POST:" << data;

   connect(m_networkReply, SIGNAL(readyRead()), &m_timer, SLOT(start()));
   connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(readToString()));
   connect(m_networkReply, SIGNAL(finished()),  this, SLOT(finishedSlot()) );
   connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(errorSlot(QNetworkReply::NetworkError)));

   m_timer.start();
}


} } // end namespace IQmol::Network
