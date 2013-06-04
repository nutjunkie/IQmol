/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include "HttpThread.h"
#include "QsLog.h"
#include <QHttp>
#include <QFile>
#include <QBuffer>
#include <QFileInfo>
#include <QEventLoop>


namespace IQmol {


HttpThread::HttpThread(QUrl const& url, int const timeout) : Threaded(timeout), 
   m_url(url), m_file(0), m_buffer(0)
{ 
   m_eventLoop = new QEventLoop();
   m_eventLoop->moveToThread(&m_thread);
   connect(this, SIGNAL(killEventLoop()), m_eventLoop, SLOT(quit()));

   m_http = new QHttp();
   m_http->moveToThread(&m_thread);
   connect(m_http, SIGNAL(requestFinished(int, bool)), 
      this, SLOT(httpRequestFinished(int, bool)));
   connect(m_http, SIGNAL(dataReadProgress(int, int)),
      this, SLOT(updateDataReadProgress(int, int)));
   connect(m_http, SIGNAL(responseHeaderReceived(QHttpResponseHeader const&)),
      this, SLOT(readResponseHeader(QHttpResponseHeader const&)));
}


HttpThread::~HttpThread()
{
   delete m_http;
   delete m_eventLoop;
   delete m_buffer;
   delete m_file;
}


void HttpThread::run()
{
   // QHTTP is asynchronous, but we have already branched off the GUI thread so
   // we need to block until QHTTP has finished, otherwise our thread will
   // return before the HTTP request has been satisfied.
   //downloadFile();
getResponse();
   m_eventLoop->exec(QEventLoop::ExcludeUserInputEvents);
}


void HttpThread::downloadFile()
{
   QFileInfo fileInfo(m_url.path());
   QString fileName = fileInfo.fileName();
   if (fileName.isEmpty()) fileName = "index.html";
   if (QFile::exists(fileName))  QFile::remove(fileName);

   m_file = new QFile(fileName);
   if (!m_file->open(QIODevice::WriteOnly)) {
      m_errorMessage = "Unable to save the file " + fileName +": " + m_file->errorString();
      delete m_file;
      m_file = 0;
      return;
   }

   QHttp::ConnectionMode mode = (m_url.scheme().toLower() == "https") ?
      QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;

   m_http->setHost(m_url.host(), mode, m_url.port() == -1 ? 0 : m_url.port());
    
   QByteArray path = QUrl::toPercentEncoding(m_url.path(), "!$?&()*+,;=:@/");
   if (path.isEmpty()) path = "/";
    path = "/" + path;
        
qDebug() << "Attempting to get" << path;
   m_httpGetId = m_http->get(path, m_file);
}

void HttpThread::getResponse()
{
   QHttp::ConnectionMode mode = (m_url.scheme().toLower() == "https") ?
      QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;

   m_http->setHost(m_url.host(), mode, m_url.port() == -1 ? 0 : m_url.port());
    
   QByteArray path = QUrl::toPercentEncoding(m_url.path(), "/?&=");
qDebug() << "Attempting to get" << path;
   if (path.isEmpty()) path = "/";
   if (!path.startsWith("/")) path.prepend("/");
        
qDebug() << "Attempting to get" << path;
   m_buffer = new QBuffer();
   m_buffer->open(QBuffer::ReadWrite);
   m_httpGetId = m_http->get(path, m_buffer);
}



void HttpThread::readResponseHeader(QHttpResponseHeader const& responseHeader)
{
qDebug() << "httpResponseHeader" << responseHeader.statusCode();
    switch (responseHeader.statusCode()) {
       case 200:   // Ok
       case 301:   // Moved Permanently
       case 302:   // Found
       case 303:   // See Other
       case 307:   // Temporary Redirect
           break;  // these are not error conditions

       default:
          m_errorMessage = "HTTP request failed: " + responseHeader.reasonPhrase();
qDebug() << "Error message:" << m_errorMessage;
          m_terminate = true;
          m_httpActive = false;
          m_http->abort();
    }
}


void HttpThread::updateDataReadProgress(int bytesRead, int totalBytes)
{
qDebug() << "updateDataReadProgress" << bytesRead << totalBytes;
   if (m_terminate) return;
   copyProgress(double(bytesRead)/totalBytes); 
}


void HttpThread::stopWhatYouAreDoing()
{
    m_terminate = true;
    m_http->abort();
}


void HttpThread::httpRequestFinished(int requestId, bool error)
{

   if (requestId != m_httpGetId) return;
   killEventLoop();

   if (m_file) {
      m_file->close();
      if (error) {
         m_file->remove();
         m_errorMessage = "Download failed: " + m_http->errorString();
      }
      delete m_file;
      m_file = 0;
   }else {
      if (error) {
         m_errorMessage = "Request failed: " + m_http->errorString();
       }else {
         m_buffer->seek(0);
         m_outputMessage = QString(m_buffer->readAll());
         if (m_outputMessage.contains("ERROR:")) {
            m_errorMessage = m_outputMessage;
            m_outputMessage.clear();
         }
       }
   }

   m_httpActive = false;
   finished();
}

} // end namespace IQmol
