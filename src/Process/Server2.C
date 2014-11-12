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
#include "WriteToTemporaryFile.h"
#include "JobMonitor.h"
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
   if (m_connection && 
       m_connection->status() == Network::Connection::Authenticated) return true;

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
         if (cookie.isEmpty()) return false;
         m_configuration.setValue(ServerConfiguration::Cookie, cookie);
         ServerRegistry::save();
      }
   }

   if (m_connection->status() == Network::Connection::Authenticated) {
      connect(this, SIGNAL(jobSubmissionSuccessful(Job*)), 
         &(JobMonitor::instance()), SLOT(jobSubmissionSuccessful(Job*)));

      connect(this, SIGNAL(jobSubmissionFailed(Job*)), 
         &(JobMonitor::instance()), SLOT(jobSubmissionFailed(Job*)));

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


bool Server::exists(QString const& filePath)
{
   if (!m_connection) {
      QLOG_WARN() << "Server::exists() called on invalid connection";
      return false;
   }
   return m_connection->exists(filePath);
}


bool Server::makeDirectory(QString const& filePath)
{
   if (!m_connection) {
      QLOG_WARN() << "Server::makeDirectory() called on invalid connection";
      return false;
   }
   return m_connection->makeDirectory(filePath);
}


void Server::queryAllJobs()
{
qDebug() << "QueryAllJobs called()";
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


// ---------- Submit ----------
void Server::submit(Job* job)
{
   QList<Network::Reply*> keys(m_activeRequests.keys(job));

   if (!job) throw Exception("Submit called with null job");
   if (!open()) throw Exception("Unable to connect to server");
   if (m_watchedJobs.contains(job)) throw Exception("Attempt to submit duplicate job");
   if (!keys.isEmpty()) throw Exception("Attempt to submit busy job");

   qDebug() << "Request to submit job " << job->jobName();

   QString contents(job->jobInfo().get(QChemJobInfo::InputString));
   QString fileName(Util::WriteToTemporaryFile(contents));
   qDebug() << "Input file contents written to" << fileName;

   // In the case of an HTTP server, we can simply POST the contents of the
   // input file and we're done.  Other servers need the run file and a 
   // separate submission step.
   if (isWebBased()) {
      QString submit(m_configuration.value(ServerConfiguration::Submit));
      submit = substituteMacros(submit);
      qDebug() << "Copying http input file to" << submit;
      Network::Reply* reply(m_connection->putFile(fileName, submit));
      connect(reply, SIGNAL(finished()), this, SLOT(submitFinished()));
      m_activeRequests.insert(reply, job);
   }else {
      QString destination(job->jobInfo().getRemoteFilePath(QChemJobInfo::InputFileName));
      Network::Reply* reply(m_connection->putFile(fileName, destination));
      connect(reply, SIGNAL(finished()), this, SLOT(copyRunFile()));
      m_activeRequests.insert(reply, job);
   }
}


void Server::copyRunFile()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);
      reply->deleteLater();

      if (reply->status() != Network::Reply::Finished) {
         job->setStatus(Job::Error);
         job->setMessage(reply->message());
         jobSubmissionFailed(job);
         return;
      }

      QString fileContents(m_configuration.value(ServerConfiguration::RunFileTemplate));
      fileContents = substituteMacros(fileContents);
      fileContents = job->substituteMacros(fileContents);
      QString fileName(Util::WriteToTemporaryFile(fileContents));

      qDebug() << "Run   file contents written to" << fileName;

      QString destination(job->jobInfo().getRemoteFilePath(QChemJobInfo::RunFileName));
      reply = m_connection->putFile(fileName, destination);
      connect(reply, SIGNAL(finished()), this, SLOT(queueJob()));
      m_activeRequests.insert(reply, job);

   }else {
      QLOG_ERROR() << "Server Error: invalid reply";
      if (reply) reply->deleteLater();
      return;
   }
}


void Server::queueJob()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);
      reply->deleteLater();

      if (reply->status() != Network::Reply::Finished) {
         job->setStatus(Job::Error);
         job->setMessage(reply->message());
qDebug() << "Server::queueJob() sending signal";
         jobSubmissionFailed(job);
         return;
      }

      QString submit(m_configuration.value(ServerConfiguration::Submit));
      submit = substituteMacros(submit);
      submit = job->substituteMacros(submit);

      qDebug() << "Executing submit command:     " << submit;

      reply = m_connection->execute(submit);
      connect(reply, SIGNAL(finished()), this, SLOT(submitFinished()));
      m_activeRequests.insert(reply, job);

   }else {
      QLOG_ERROR() << "Server Error: invalid reply";
      if (reply) reply->deleteLater();
      return;
   }
}


void Server::submitFinished()
{
qDebug() << "Server submitFinished SLOT called";
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);
      reply->deleteLater();

      if (reply->status() != Network::Reply::Finished) {
         job->setStatus(Job::Error);
         job->setMessage(reply->message());
qDebug() << "Server::submitFinished() sending signal";
         jobSubmissionFailed(job);
         return;
      }

      // need to get the id
      qDebug() << "-------------------------------------------";
      qDebug() << "Submit returned" << reply->message();
      qDebug() << "-------------------------------------------";
      job->parseQueryOutput(reply->message());

      job->setStatus(Job::Queued);
      watchJob(job);
      jobSubmissionSuccessful(job);

   }else {
      QLOG_ERROR() << "Server Error: invalid reply";
      if (reply) reply->deleteLater();
      return;
   }
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

   qDebug() << "Query string:" << query;
   qDebug() << "taking a pass on the query";
   m_updateTimer.stop();
return;

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
   output.remove("POST");
   output.remove("GET");
   output = output.trimmed();
   
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
      if (!m_watchedJobs.contains(job)) {
         m_watchedJobs.append(job); 
         connect(job, SIGNAL(deleted(Job*)), this, SLOT(unwatchJob(Job*)));
      }
      if (m_connection && m_connection->isConnected()) m_updateTimer.start();
   }
}


} } // end namespace IQmol::Process
