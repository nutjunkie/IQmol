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

#include "WebHost.h"
#include "HttpThread.h"
#include "HttpServer.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include <QHttp>

#include <QDebug>


namespace IQmol {


bool WebHost::connectServer()
{
   QUrl url;
   url.setScheme("http");
   url.setPort(m_server->port());
   url.setHost(m_server->hostAddress());

   QString query("${CGI_ROOT}/handshake.cgi?user=${USER}&cookie=");
   QString oldCookie(m_server->cookie());
   query += oldCookie;
   query  = m_server->replaceMacros(query);
   url.setPath(query);

qDebug() << "Connection attempt to HTTP server, URL= " << url.toString();
   HttpThread thread(url, 5000);

   try {
     qDebug() << thread.block();
   } catch (std::exception const& ex) {
     qDebug() << ex.what();
   }

   QString errorMessage(thread.errorMessage());
   QString outputMessage(thread.outputMessage());
   if (!errorMessage.isEmpty() || !outputMessage.contains("cookie")) {
      QString msg("Connection to server ");
      msg += name() + " falied: "  + errorMessage + outputMessage;
      QMsgBox::warning(0, "IQmol", msg);
      return m_connected;
   }

   QString msg;
   QString newCookie;
   QStringList response(outputMessage.split(QRegExp("\\s+"), QString::SkipEmptyParts));

   QStringList::iterator iter;
   for (iter = response.begin(); iter != response.end(); ++iter) {
       QStringList tokens((*iter).split('='));
       if (tokens.size() == 2) {
          if (tokens[0] == "cookie") {
             newCookie = tokens[1];
             m_connected = true;
          }else if (tokens[1] == "valid") {
             msg += "HTTP Server: account valid ";
          }else if (tokens[1] == "expired") {
             msg = "HTTP Server: account expired, new account token issued ";
          }else {
             msg = "HTTP Server: new account token issued ";
          }
       }
   }

   if (oldCookie != newCookie) {
      msg += newCookie;
      m_server->setCookie(newCookie);
      QMsgBox::information(0, "IQmol", msg);
   }

   return m_connected;
}


// exec works a bit differently for web hosts as we auto-insert the cookie.
// Commands should be passed in the form (note spacing):
//    command.cgi arg1=value1 arg2=value2
// this will get converted to
//    http://host.com:80/cgi_root/command.cgi?cookie=cookie_value&arg1=value1&arg2=value2
Threaded* WebHost::exec(QString const& command) 
{
   if (command.isEmpty()) {
      qDebug() << "Empty command passed to WebHost";
      return 0;
   }

   QUrl url;
   url.setScheme("http");
   url.setPort(m_server->port());
   url.setHost(m_server->hostAddress());

   QStringList args(command.split(QRegExp("\\s+"), QString::SkipEmptyParts));
   QString cmd(args.takeFirst());
   
   QString query("${CGI_ROOT}/");
   query += cmd + "?cookie=";
   query += m_server->cookie();

   QStringList::iterator iter;
   for (iter = args.begin(); iter != args.end(); ++iter) {
       query += "&" + (*iter);
   }

   query = m_server->replaceMacros(query);
   qDebug() << "WebHost::exec query:" << query;
   url.setPath(query);

   HttpThread* thread = new HttpThread(url, 5000);

   return thread;
}


Threaded* WebHost::mkdir(QString const& dir) 
{
   QString msg("WARNING: Null thread pointer returned for WetHost::mkdri");
   qDebug() << msg;
   return new ServerTask::DoNothing(m_server, msg);
}


Threaded* WebHost::exists(QString const& filePath, HostDelegate::FileFlags const flags) 
{
   QString msg("WARNING: Null thread pointer returned for WebHost::exists");
   qDebug() << msg;
   return new ServerTask::DoNothing(m_server, msg);
}
 

Threaded* WebHost::push(QString const& contents, QString const&)
{
   QString cmd("push.cgi content=");
   cmd += QUrl::toPercentEncoding(contents);
   return exec(cmd);
}

            
Threaded* WebHost::pull(QString const& sourcePath, QString const& destinationPath) 
{
   QString msg("WARNING: Null thread pointer returned for WebHost::pull");
   qDebug() << msg;
   return new ServerTask::DoNothing(m_server, msg);
}
            

Threaded* WebHost::move(QString const& sourcePath, QString const& destinationPath) 
{
   QString msg("WARNING: Null thread pointer returned for WebHost::move");
   qDebug() << msg;
   return new ServerTask::DoNothing(m_server, msg);
}


Threaded* WebHost::remove(QString const& filePath) 
{
   QString msg("WARNING: Null thread pointer returned for WebHost::remove");
   qDebug() << msg;
   return new ServerTask::DoNothing(m_server, msg);
}


Threaded* WebHost::grep(QString const& string, QString const& filePath) 
{
   QString msg("WARNING: Null thread pointer returned for WebHost::grep");
   qDebug() << msg;
   return new ServerTask::DoNothing(m_server, msg);
}


Threaded* WebHost::checkOutputForErrors(QString const& filePath) 
{
   QString msg("WARNING: Null thread pointer returned for WebHost::checkOutputForErrors");
   qDebug() << msg;
   return new ServerTask::DoNothing(m_server, msg);
}


} // end namespace IQmol
