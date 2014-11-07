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

#include "Server2.h"
#include "ServerRegistry2.h"
#include "Reply.h"
#include "Connection.h"
#include "LocalConnection.h"
#include "SshConnection.h"
#include "HttpConnection.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Process2 {

Server::Server(ServerConfiguration const& configuration) : m_configuration(configuration),
   m_connection(0)
{
   qDebug() << "Constructing a New Server with configuration";
   m_configuration.dump();
   setUpdateInterval(m_configuration.updateInterval());
   connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(queryAllJobs()));
}


Server::~Server()
{
   if (m_connection)  delete m_connection;
}


QString Server::name() const
{
   return m_configuration.value(ServerConfiguration::ServerName);
}


QStringList Server::tableFields() const
{
   QStringList fields;
   fields.append(m_configuration.value(ServerConfiguration::ServerName));
   fields.append(m_configuration.value(ServerConfiguration::HostAddress));
   fields.append(m_configuration.value(ServerConfiguration::Connection));
   fields.append(m_configuration.value(ServerConfiguration::UserName));

   return fields;
}


bool Server::open()
{
   if (!m_connection) {
      QVariant address(m_configuration.value(ServerConfiguration::HostAddress));
      int port(m_configuration.port());

      switch (m_configuration.connection()) {
         case ServerConfiguration::Local:
            m_connection = new Network::LocalConnection();
            break;
         case ServerConfiguration::SSH:
            m_connection = new Network::SshConnection(address.toString(), port);
            break;
         case ServerConfiguration::HTTP:
         case ServerConfiguration::HTTPS:
            m_connection = new Network::HttpConnection(address.toString(), port);
            break;
      }
   }

   if (m_connection->status() == Network::Connection::Closed) {
      m_connection->open();
   }

   if (m_connection->status() == Network::Connection::Opened) {
      Network::Connection::AuthenticationT authentication(m_configuration.authentication());
      QVariant userName(m_configuration.value(ServerConfiguration::UserName));
      m_connection->authenticate(authentication, userName.toString());
   }

   if (m_configuration.isWebBased()) {
      QString cookie(m_configuration.value(ServerConfiguration::Cookie));
      if (cookie.isEmpty()) {
         cookie = m_connection->obtainCookie();
         m_configuration.setValue(ServerConfiguration::Cookie, cookie);
         ServerRegistry::save();
      }
   }

   if (m_connection->status() == Network::Connection::Authenticated) {
      if (!m_watchedJobs.isEmpty()) m_updateTimer.start();
      return true;
   }

   QLOG_ERROR() << "Server::open() failed to open connection to server"
               << m_configuration.value(ServerConfiguration::ServerName);
   m_connection->close();
   delete m_connection;
   m_connection = 0;
   return false;
}


void Server::queryAllJobs()
{
   if (!m_connection || !m_connection->isConnected()) {
      m_updateTimer.stop();
      return;
   }

   // This is a bit of a hack, update this here in case the user
   // has modified the server while running.
   setUpdateInterval(m_configuration.updateInterval());

   QList<Job*>::iterator iter;
   for (iter = m_watchedJobs.begin(); iter != m_watchedJobs.end(); ++iter) {
       query(*iter);
   }
}


// ---------- Test ----------
void Server::test()
{
   if (m_configuration.isWebBased()) {
      if (m_configuration.value(ServerConfiguration::Cookie).isEmpty()) {
qDebug() << "Resistering HTTP server";
         Network::Reply* reply(m_connection->execute("register"));
         connect(reply, SIGNAL(finished()), this, SLOT(testFinished()));
         reply->start();
      }
   } 
   // Check qchem set up
}


void Server::testFinished()
{
}


void Server::registerFinished()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));
   if (!reply) {
      QLOG_ERROR() << "Server Error: invalid reply";
      return;
   }

   if (reply->status() != Network::Reply::Finished) {
      QLOG_WARN() << "Reply finshed with status" << reply->status();
      QLOG_WARN() << "                 Message:" << reply->message();
      return;
   }

   // parse registration id
qDebug() << "===========================";
qDebug() << "=== Register finished  ====";
   reply->message();
qDebug() << "===========================";

   QString cookie;

   m_configuration.setValue(ServerConfiguration::Cookie, cookie);
   ServerRegistry::save();

   reply->deleteLater();
}


// ---------- Setup ----------
void Server::setup(Job* job)
{
   QString query(m_configuration.value(ServerConfiguration::Query));
   query = substituteMacros(query);
   query = job->substituteMacros(query);

   Network::Reply* reply(m_connection->execute(query));
   connect(reply, SIGNAL(finished()), this, SLOT(queryFinished()));
   m_activeRequests.insert(reply, job);
   reply->start();


// make directories for ssh 
}


void Server::setupFinished()
{
}


// ---------- Submit ----------
void Server::submit(Job* job)
{
   if (!open()) {
      QLOG_ERROR() << "Server Error: submit called on disconnected server - "
         << m_configuration.value(ServerConfiguration::ServerName);
      return;
   }
   if (m_watchedJobs.contains(job)) {
      QLOG_ERROR() << "Server Error: submit called existing job";
      return;
   }

   QList<Network::Reply*> keys(m_activeRequests.keys(job));
   if (!keys.isEmpty()) {
      QLOG_WARN() << "Server Error: submit called on busy job";
      return;  
   }


   QString query(m_configuration.value(ServerConfiguration::Query));
   query = substituteMacros(query);
   query = job->substituteMacros(query);

   Network::Reply* reply(m_connection->execute(query));
   connect(reply, SIGNAL(finished()), this, SLOT(queryFinished()));
   m_activeRequests.insert(reply, job);
   reply->start();
}


void Server::submitFinished()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));
   if (!reply) {
      QLOG_ERROR() << "Server Error: invalid reply";
      return;
   }

   if (!m_activeRequests.contains(reply)) {
      QLOG_WARN() << "Server Error: submit called on busy job";
      if (reply) reply->deleteLater();
      return;
   }

   if (reply->status() == Network::Reply::Finished) {
      Job* job(m_activeRequests.value(reply));

      if (m_watchedJobs.contains(job)) {
         job->parseQueryOutput(reply->message());
         job->updated();
      }
   watchJob(job);

   }else {
      QLOG_WARN() << "Reply finshed with status" << reply->status();
      QLOG_WARN() << "  Message:" << reply->message();
   }

   m_activeRequests.remove(reply);
   reply->deleteLater();

}


// ---------- Query ----------
void Server::query(Job* job)
{
   //if (!check(job, "Query")) return;

   QList<Network::Reply*> keys(m_activeRequests.keys(job));
   if (!keys.isEmpty()) {
      QLOG_WARN() << "Prior request exists for job";
      return;  
   }

   QString query(m_configuration.value(ServerConfiguration::Query));
   query = substituteMacros(query);
   query = job->substituteMacros(query);

   Network::Reply* reply(m_connection->execute(query));
   connect(reply, SIGNAL(finished()), this, SLOT(queryFinished()));
   m_activeRequests.insert(reply, job);
   reply->start();
}


void Server::queryFinished()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));
   if (!m_activeRequests.contains(reply)) {
      if (reply) reply->deleteLater();
      return;
   }

   if (reply->status() == Network::Reply::Finished) {
      Job* job(m_activeRequests.value(reply));

      if (m_watchedJobs.contains(job)) {
         job->parseQueryOutput(reply->message());
         job->updated();
      }

   }else {
      QLOG_WARN() << "Reply finshed with status" << reply->status();
      QLOG_WARN() << "  Message:" << reply->message();
   }

   m_activeRequests.remove(reply);
   reply->deleteLater();
}


// ---------- kill ----------
void Server::kill(Job*)
{
}


void Server::killFinished()
{
}


// ---------- Copy ----------
void Server::copy(Job*)
{
}


void Server::copyFinished()
{
}

// --------------------------


// This will be a null operation for all but HTTP(S) servers
QString Server::substituteMacros(QString const& input)
{
   QString output(input);
   output.replace("${COOKIE}", m_configuration.value(ServerConfiguration::Cookie));
   return output;
}


void Server::setUpdateInterval(int const seconds)
{
   m_updateTimer.setInterval(seconds*1000);
}


void Server::unwatchJob(Job* job)
{
   if (!m_watchedJobs.contains(job)) m_watchedJobs.removeAll(job); 
   if (m_watchedJobs.isEmpty()) m_updateTimer.stop();
}


void Server::watchJob(Job* job)
{
   if (job) {
      if (!m_watchedJobs.contains(job)) m_watchedJobs.append(job); 
      if (m_connection && m_connection->isConnected()) m_updateTimer.start();
   }
}

} } // end namespace IQmol::Process
