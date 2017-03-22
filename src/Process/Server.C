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

#include "Server.h"
#include "ServerRegistry.h"
#include "Reply.h"
#include "Connection.h"
#include "LocalConnection.h"
#include "SshConnection.h"
#include "HttpConnection.h"
#include "WriteToTemporaryFile.h"
#include "TextStream.h"
#include "JobMonitor.h"
#include "Preferences.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Process {

Server::Server(ServerConfiguration const& configuration) : m_configuration(configuration),
   m_connection(0)
{
   //qDebug() << "Constructing a New Server with configuration";
   //m_configuration.dump();
   setUpdateInterval(m_configuration.updateInterval());
   connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(queryAllJobs()));
}


Server::~Server()
{
   closeConnection();
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


void Server::closeConnection()
{
   if (m_connection) {
      m_connection->close();
      delete m_connection;
      m_connection = 0;
   }
}


void Server::open()
{
   if (m_connection && 
       m_connection->status() == Network::Connection::Authenticated) return;

   if (!m_connection) {
      QLOG_TRACE() << "Creating connection" 
                   << m_configuration.value(ServerConfiguration::Connection);

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
            m_connection = new Network::HttpConnection(address.toString(), port);
            break;
         case ServerConfiguration::HTTPS:
            m_connection = new Network::HttpConnection(address.toString(), port, true);
            break;
      }
   }

   if (m_connection->status() == Network::Connection::Closed) {
      QLOG_TRACE() << "Opening connection to server" << name();
      m_connection->open();
   }

   if (m_connection->status() == Network::Connection::Opened) {
      Network::Connection::AuthenticationT 
         authentication(m_configuration.authentication());

      if (m_configuration.isWebBased()) {
         QString cookie(m_configuration.value(ServerConfiguration::Cookie));
         m_connection->authenticate(authentication, cookie);
         m_configuration.setValue(ServerConfiguration::Cookie, cookie);
         ServerRegistry::save();
      }else {
         QString userName(m_configuration.value(ServerConfiguration::UserName));
         m_connection->authenticate(authentication, userName);
      }
   }

   if (m_connection->status() == Network::Connection::Authenticated) {
      if (!m_watchedJobs.isEmpty()) {
         queryAllJobs();
         startUpdates();
      }
      return;
   }

   delete m_connection;
   m_connection = 0;

   throw Exception(QString("Failed to connect to server ") + name());
}


bool Server::exists(QString const& filePath)
{
   open();
   return m_connection->exists(filePath);
}


QString Server::queueInfo()
{
   open();
   QString cmd(m_configuration.value(ServerConfiguration::QueueInfo));
   cmd = substituteMacros(cmd);
   QString info;
   m_connection->blockingExecute(cmd, &info);
   return info;
}


bool Server::removeDirectory(QString const& path)
{
   open();
   return m_connection->removeDirectory(path);
}


bool Server::makeDirectory(QString const& path)
{
   open();
   return m_connection->makeDirectory(path);
}


void Server::queryAllJobs()
{
   qDebug() << "QueryAllJobs called()";
   // This is a bit of a hack, update this here in case the user
   // has modified the server while running.
   setUpdateInterval(m_configuration.updateInterval());

   if (m_watchedJobs.isEmpty()) {
      stopUpdates(); 
      return;
   }

   open();

   QList<Job*>::iterator iter;
   for (iter = m_watchedJobs.begin(); iter != m_watchedJobs.end(); ++iter) {
       query(*iter);
   }
}


// ---------- Submit ----------
void Server::submit(Job* job)
{
qDebug() << "QJI - trace Server::submit()";
   QList<Network::Reply*> keys(m_activeRequests.keys(job));

   if (!job) throw Exception("Submit called with null job");
   if (m_watchedJobs.contains(job)) throw Exception("Attempt to submit duplicate job");
   if (!keys.isEmpty()) throw Exception("Attempt to submit busy job");

   open();

   QString contents(job->jobInfo().get(QChemJobInfo::InputString));
   QString fileName(Util::WriteToTemporaryFile(contents));
   QLOG_DEBUG() << "Input file contents written to" << fileName;

   if (isLocal()) job->jobInfo().localFilesExist(true);

   // In the case of an HTTP server, we can simply POST the contents of the
   // input file and we're done.  Other servers need the run file and a 
   // separate submission step.
   if (isWebBased()) {
      QString submit(m_configuration.value(ServerConfiguration::Submit));
      submit = substituteMacros(submit);
      Network::Reply* reply(m_connection->putFile(fileName, submit));
      connect(reply, SIGNAL(finished()), this, SLOT(submitFinished()));
      m_activeRequests.insert(reply, job);
      reply->start();
   }else {
      QString destination(job->jobInfo().getRemoteFilePath(QChemJobInfo::InputFileName));
      Network::Reply* reply(m_connection->putFile(fileName, destination));
      connect(reply, SIGNAL(finished()), this, SLOT(copyRunFile()));
      m_activeRequests.insert(reply, job);
      reply->start();
   }
qDebug() << "QJI - trace Server::submit() finished";
}


void Server::copyRunFile()
{
qDebug() << "QJI - trace Server::copyRunFile()";
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);

      if (!job) {
         QLOG_WARN() << "Invalid Job pointer";
         //reply->deleteLater();
         return;
      }

      if (reply->status() != Network::Reply::Finished) {
         job->setStatus(Job::Error, reply->message());
         JobMonitor::instance().jobSubmissionFailed(job);
         //reply->deleteLater();
         return;
      }

      //reply->deleteLater();

      QString fileContents(m_configuration.value(ServerConfiguration::RunFileTemplate));
      fileContents = substituteMacros(fileContents);
      fileContents = job->substituteMacros(fileContents);
      fileContents += "\n";
      QString fileName(Util::WriteToTemporaryFile(fileContents));

      qDebug() << "Run file contents written to" << fileName;

      QString destination(job->jobInfo().getRemoteFilePath(QChemJobInfo::RunFileName));
#ifdef Q_OS_WIN32
      if (isLocal()) destination = job->jobInfo().getRemoteFilePath(QChemJobInfo::BatchFileName);
#endif
      reply = m_connection->putFile(fileName, destination);
      connect(reply, SIGNAL(finished()), this, SLOT(queueJob()));
      m_activeRequests.insert(reply, job);
      reply->start();

   }else {
      QLOG_ERROR() << "Server Error: invalid reply";
      //if (reply) reply->deleteLater();
   }
}


void Server::queueJob()
{
qDebug() << "QJI - trace Server::queueJob()";
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);
      if (!job) {
         QLOG_WARN() << "Invalid Job pointer";
         //reply->deleteLater();
         return;
      }

      if (reply->status() != Network::Reply::Finished) {
         job->setStatus(Job::Error, reply->message());
         JobMonitor::instance().jobSubmissionFailed(job);
         //reply->deleteLater();
         return;
      }

      //reply->deleteLater();
      QString submit(m_configuration.value(ServerConfiguration::Submit));
      submit = substituteMacros(submit);
      submit = job->substituteMacros(submit);

      QString workingDirectory(job->jobInfo().get(QChemJobInfo::RemoteWorkingDirectory));

      reply = m_connection->execute(submit, workingDirectory);
      connect(reply, SIGNAL(finished()), this, SLOT(submitFinished()));
      m_activeRequests.insert(reply, job);
      reply->start();

   }else {
      QLOG_ERROR() << "Server Error: invalid reply";
      //if (reply) reply->deleteLater();
   }
}


void Server::submitFinished()
{
qDebug() << "QJI - trace Server::submitFinished()";
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);

      if (job) {
qDebug() << "QJI: Server.C job info dump";
job->jobInfo().dump();
         if (reply->status() == Network::Reply::Finished && 
              parseSubmitMessage(job, reply->message())) {
            job->setStatus(Job::Queued);
            JobMonitor::instance().jobSubmissionSuccessful(job);
qDebug() << "About to watch job";
            watchJob(job);
         }else {
            job->setStatus(Job::Error, reply->message());
            JobMonitor::instance().jobSubmissionFailed(job);
         }
      }else {
         QLOG_WARN() << "Invalid Job pointer";
      }

      //reply->deleteLater();

   }else {
      QLOG_ERROR() << "Server Error: invalid reply";
   }
qDebug() << "QJI: Server.C job submission finished";
}


// This should be delegated
bool Server::parseSubmitMessage(Job* job, QString const& message)
{
   QLOG_TRACE() << "Submit returned message of length:" << message.size();
   QLOG_TRACE() << "Submit returned:\n" << message;

   bool ok(false);

   switch (m_configuration.queueSystem()) {
      case ServerConfiguration::PBS: {
         // A successful submission returns a single token containing the job ID
         QStringList tokens(message.split(QRegExp("\\s+"), QString::SkipEmptyParts));
         if (tokens.size() == 1) {
            job->setJobId(tokens.first());
            QLOG_DEBUG() << "PBS job submitted with id" << job->jobId();
            ok = true;
         } 
      } break;
         
      case ServerConfiguration::Web: {
         QRegExp rx("Qchemserv-Jobid::([0-9a-zA-Z\\-_]+)");
         if (message.contains("Qchemserv-Status::OK") && rx.indexIn(message,0) != -1) {
            job->setJobId(rx.cap(1));
            ok = true;
         }
      } break;

      case ServerConfiguration::SGE: {
         // A successful submission returns a string like:
         //   Your job 2834 ("test.sh") has been submitted
         QStringList tokens(message.split(QRegExp("\\s+"), QString::SkipEmptyParts));
         if (message.contains("has been submitted")) {
            int id(tokens[2].toInt(&ok));
            if (ok) job->setJobId(QString::number(id));
            ok = true;
         }
      } break;

      case ServerConfiguration::Basic: {
         qDebug() << "Need to correctly parse submit message for server type "
                  << ServerConfiguration::toString(m_configuration.queueSystem());
         // A successful submission returns a string like:
         //   [1] 9876 $QC/exe/qcprog.exe .aaaa.inp.9876.qcin.1 $QCSCRATCH/local/qchem9876
         // ...or on Windows we parse for the following 
         //   ProcessId = 1234

         if (message.contains("ProcessId =")) { // Windows
            QRegExp rx("ProcessId =\\s+(\\d+)\\s+=");
            if (rx.indexIn(message) != -1) {
               int id(rx.cap(1).toInt(&ok));
               if (ok) job->setJobId(QString::number(id));
            }else {
               // It is possible that the job has completed before the batch file
               // determines its PID, so we let this slide.
               ok = true;
            }
         }else {
            QStringList tokens(message.split(QRegExp("\\s+"), QString::SkipEmptyParts));
            if (tokens.size() >= 2) {
               int id(tokens[1].toInt(&ok));
               if (ok) job->setJobId(QString::number(id));
            }
         }
      } break;

      default:
         qDebug() << "Need to parse submit message for server type "
                  << ServerConfiguration::toString(m_configuration.queueSystem());
         break;
   }

   return ok;
}


// ---------- Query ----------
void Server::query(Job* job)
{
   if (!job) throw Exception("Query called on invalid job");
   QList<Network::Reply*> keys(m_activeRequests.keys(job));

   if (!keys.isEmpty()) {
      QLOG_DEBUG() << "Query on busy job";
      return;
   }

   open();

   QString query(m_configuration.value(ServerConfiguration::Query));
   query = substituteMacros(query);
   query = job->substituteMacros(query);

   qDebug() << "Query string:" << query;

   Network::Reply* reply(m_connection->execute(query));
   connect(reply, SIGNAL(finished()), this, SLOT(queryFinished()));
   m_activeRequests.insert(reply, job);
   reply->start();
}


void Server::queryFinished()
{
qDebug() << "QJI - trace Server::queryFinsihed()";
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);

      if (job) {
         if (reply->status() != Network::Reply::Finished || 
            !parseQueryMessage(job, reply->message())) {
            job->setStatus(Job::Unknown, reply->message());
          }
      }else {
         QLOG_WARN() << "Invalid Job pointer";
      }

      //reply->deleteLater();

   }else {
      QLOG_ERROR() << "Server Error: invalid query reply";
   }
}


// This should be delegated
bool Server::parseQueryMessage(Job* job, QString const& message)
{  
   bool ok(false);
   Job::Status status(Job::Unknown);
   QString statusMessage;

   switch (m_configuration.queueSystem()) {

      case ServerConfiguration::PBS: {

         if (message.isEmpty()) {
            // assume finished, but need to check for errors
            status = Job::Finished;
            ok = true;
         }else {

            QStringList lines(message.split(QRegExp("\\n"), QString::SkipEmptyParts));
            QStringList tokens;
            QStringList::iterator iter;

            for (iter = lines.begin(); iter != lines.end(); ++iter) {
                //job_state = R
                if ((*iter).contains("job_state =")) {
                   tokens = (*iter).split(QRegExp("\\s+"), QString::SkipEmptyParts);
                   if (tokens.size() >= 3) {
                      if (tokens[2] == "R" || tokens[2] == "E") {
                         status = Job::Running;
                      }else if (tokens[2] == "S" || tokens[2] == "H") {
                         status = Job::Suspended;
                      }else if (tokens[2] == "Q" || tokens[2] == "W") {
                         status = Job::Queued;
                      }else if (tokens[2] == "F") {
                         status = Job::Finished;
                      }
                      ok = true;
                   }
                }

                if ((*iter).contains("resources_used.cput")) {
                   QString time((*iter).split(QRegExp("\\s+"), QString::SkipEmptyParts).last());
                   job->resetTimer(Util::Timer::toSeconds(time));
                }else if ((*iter).contains("comment =")) {
                   statusMessage = (*iter).remove("comment = ").trimmed();
                }
            }
         }
      } break;
         
      case ServerConfiguration::Web: {
         QRegExp rx("Qchemserv-Jobstatus::([A-Z]+)");
         if (message.contains("Qchemserv-Status::OK") && rx.indexIn(message,0) != -1) {
            QString rv(rx.cap(1));
            if (rv == "DONE")    status = Job::Finished;
            if (rv == "RUNNING") status = Job::Running;
            if (rv == "QUEUED")  status = Job::Queued;
            if (rv == "ERROR")   status = Job::Error;
            ok = true;
          }
      } break;

      case ServerConfiguration::SGE: {
         if (message.isEmpty() || message.contains("not exist")) {
            status = Job::Finished;
         }else {
            int nTokens;
            QString input(message);
            QStringList tokens;
            Parser::TextStream textStream(&input);

            while (!textStream.atEnd()) {
               tokens = textStream.nextLineAsTokens();
               nTokens = tokens.size();

               if (nTokens >= 5 && tokens.first().contains(job->jobId())) {
                  QString s(tokens[4]);
                  if (s.contains("q", Qt::CaseSensitive)) {
                     status = Job::Queued;
                  }else if (s.contains("s", Qt::CaseInsensitive)) {
                     status = Job::Suspended;
                  }else if (s.contains("r", Qt::CaseSensitive)) {
                     status = Job::Running;
                  }
                  ok = true;
               }else if (nTokens > 1 && 
                  tokens.first().contains("usage"), Qt::CaseInsensitive) {
                  QRegExp rx("cpu=([\\d:]+)");
                  for (int i = 1; i < tokens.size(); ++i) {
                      if (rx.indexIn(tokens[i]) >= 0) {
                         qDebug() << "RegExp matched:" << rx.cap(1);
                         job->resetTimer(Util::Timer::toSeconds(rx.cap(1)));
                      }
                  }
               }
            }
         }
      } break;

      case ServerConfiguration::Basic: {
         if (message.isEmpty()) {
            status = Job::Finished;
         }else if (message.contains("No tasks are running")) { // Windows
            status = Job::Finished;
         }else if (message.contains(Preferences::ServerQueryJobFinished())) { // Windows
            status = Job::Finished;
         }else {
            status = Job::Running;
         }
         ok = true;
      } break;


      default:
         QLOG_ERROR() << "Need to parse query message for server type "
                      << ServerConfiguration::toString(m_configuration.queueSystem());
         break;
   }

   // Only print message if there has been a change in status
   if (job->status() != status) {
      QLOG_TRACE() << "Query returned:" << message;
      QLOG_TRACE() << "parseQueryMessage setting status to " << Job::toString(status) << ok;
   }

   if (!Job::isActive(status)) unwatchJob(job);
   job->setStatus(status, statusMessage); 

   return ok;
}



// ---------- kill ----------
void Server::kill(Job* job)
{
   QList<Network::Reply*> keys(m_activeRequests.keys(job));

   if (!job) throw Exception("Query called on invalid job");
   if (!keys.isEmpty()) throw Exception("Job busy");

   if (isLocal()) {
      //we currently 
   }

   open();

   QString kill(m_configuration.value(ServerConfiguration::Kill));
   kill = substituteMacros(kill);
   kill = job->substituteMacros(kill);

   qDebug() << "Kill string:" << kill;

   Network::Reply* reply(m_connection->execute(kill));
   connect(reply, SIGNAL(finished()), this, SLOT(killFinished()));
   m_activeRequests.insert(reply, job);
   reply->start();
}


void Server::killFinished()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);
      if (!job) {
         QLOG_WARN() << "Invalid Job pointer";
         //reply->deleteLater();
         return;
      }

      unwatchJob(job);
      job->setStatus(Job::Killed, reply->message());
      //reply->deleteLater();

   }else {
      QLOG_ERROR() << "Server Error: invalid kill reply";
   }
}



// ---------- Copy ----------
void Server::copyResults(Job* job)
{
   if (isLocal()) return;

   QList<Network::Reply*> keys(m_activeRequests.keys(job));

   if (!job) throw Exception("Invalid job");
   if (!keys.isEmpty()) throw  "Job busy";

   open();

   QString listCmd(m_configuration.value(ServerConfiguration::JobFileList));
   listCmd = substituteMacros(listCmd);
   listCmd = job->substituteMacros(listCmd);
   qDebug() << "list File command :" << listCmd;

   job->setStatus(Job::Copying);
   Network::Reply* reply(m_connection->execute(listCmd));
   connect(reply, SIGNAL(finished()), this, SLOT(listFinished()));
   m_activeRequests.insert(reply, job);
   reply->start();
}


void Server::listFinished()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);
      if (!job) {
         QLOG_WARN() << "Invalid Job pointer";
         return;
      }

      if (reply->status() != Network::Reply::Finished) {
         job->setStatus(Job::Error, "Copy failed");
         return;
      }

      QStringList fileList(parseListMessage(job, reply->message()));
      QString destination(job->jobInfo().get(QChemJobInfo::LocalWorkingDirectory));
      //reply->deleteLater();

      reply = m_connection->getFiles(fileList, destination);
      connect(reply, SIGNAL(copyProgress(double)), job, SLOT(copyProgress(double)));
      connect(reply, SIGNAL(finished()), this, SLOT(copyResultsFinished()));
      m_activeRequests.insert(reply, job);
      reply->start();

   }else {
      QLOG_ERROR() << "Server Error: invalid query reply";
      // if (reply) reply->deleteLater();
   }
}


QStringList Server::parseListMessage(Job* job, QString const& message)
{
   QStringList tokens(message.split("\n"));
   QStringList list;

   for (int i = 0; i < tokens.size(); ++i) {
       QString file(tokens[i]);
       if (!file.isEmpty() && file != "pathtable") list << file;
   }

   if (m_configuration.isWebBased()) {
      QString download(m_configuration.value(ServerConfiguration::QueueInfo));
      download = substituteMacros(download);
      download = job->substituteMacros(download);

      for (int i = 0; i < list.size(); ++i) {
          QString tmp(download);
          list[i] = tmp.replace("${FILE_NAME}", list[i]);
      }
   }

   return list;
}


void Server::copyResultsFinished()
{
   Network::Reply* reply(qobject_cast<Network::Reply*>(sender()));

   if (reply && m_activeRequests.contains(reply)) {
      Job* job(m_activeRequests.value(reply));
      m_activeRequests.remove(reply);
      if (!job) {
         QLOG_WARN() << "Invalid Job pointer";
         return;
      }

      if (reply->status() != Network::Reply::Finished) {
         job->setStatus(Job::Error, "Copy failed");
         return;
      }

      QString msg("Results in: ");
      msg += job->jobInfo().get(QChemJobInfo::LocalWorkingDirectory);
      job->jobInfo().localFilesExist(true);
      job->setStatus(Job::Finished, msg);
      //reply->deleteLater();

   }else {
      QLOG_ERROR() << "Server Error: invalid copy reply";
      //if (reply) reply->deleteLater();
   }
}


void Server::cancelCopy(Job* job)
{
   if (!job) {
      QLOG_WARN() << "Invalid Job pointer";
      return;
   }

   Network::Reply* reply(m_activeRequests.key(job));

   if (reply && m_activeRequests.contains(reply)) {
      m_activeRequests.remove(reply);
      job->setStatus(Job::Error, "Copy canceled");
      reply->interrupt();
      //reply->deleteLater();
   }else {
      QLOG_ERROR() << "Server Error: invalid copy reply";
      //if (reply) reply->deleteLater();
   }
}


// --------------------------

QString Server::substituteMacros(QString const& input)
{
   QString output(input);
   output.remove("POST");
   output.remove("GET");
   output = output.trimmed();
   
   output.replace("${COOKIE}",     m_configuration.value(ServerConfiguration::Cookie));
   output.replace("${USERNAME}",   m_configuration.value(ServerConfiguration::UserName));
   output.replace("${SERVERNAME}", m_configuration.value(ServerConfiguration::ServerName));
   return output;
}


void Server::setUpdateInterval(int const seconds)
{
   m_updateTimer.setInterval(seconds*1000);
}


void Server::unwatchJob(Job* job)
{
   if (m_watchedJobs.contains(job)) m_watchedJobs.removeAll(job); 
   if (m_watchedJobs.isEmpty()) stopUpdates();
}


void Server::watchJob(Job* job)
{
   if (job) {
      if (!m_watchedJobs.contains(job)) {
         m_watchedJobs.append(job); 
         connect(job, SIGNAL(deleted(Job*)), this, SLOT(unwatchJob(Job*)));
      }
      if (m_connection && m_connection->isConnected()) startUpdates();
   }
}

} } // end namespace IQmol::Process
