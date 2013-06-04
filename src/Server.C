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

#include "Server.h"
#include "ServerRegistry.h"
#include "BasicServer.h"
#include "PBSServer.h"
#include "SGEServer.h"
#include "HttpServer.h"
#include "System.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include <QStringList>


namespace IQmol {


QString Server::toString(Server::Type const& type)
{
   QString s;
   switch (type) {
      case Server::Basic:  s = "Basic";  break;
      case Server::PBS:    s = "PBS";    break;
      case Server::SGE:    s = "SGE";    break;
      case Server::HTTP:   s = "HTTP";   break;
   }
   return s;
}


QString Server::toString(Server::Host const& host)
{
   QString s;
   switch (host) {
      case Server::Local:   s = "Local";   break;
      case Server::Remote:  s = "Remote";  break;
      case Server::Web:     s = "Web";     break;
   }
   return s;
}


QString Server::toString(Server::Authentication const& auth)
{
   QString s;
   switch (auth) {
      case Server::None:                 s = "None";                 break;
      case Server::Agent:                s = "SSH Agent";            break;
      case Server::PublicKey:            s = "SSH Public Key";       break;
      case Server::HostBased:            s = "SSH Host Based";       break;
      case Server::KeyboardInteractive:  s = "KeyboardInteractive";  break;
      case Server::Vault:                s = "Vault";                break;
      case Server::Prompt:               s = "Prompt";               break;
   }
   return s;
}


Server::Server() : m_serverDelegate(0), m_testedAndValid(false)
{
   connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateProcesses()));
}


Server::~Server()
{
   if (m_serverDelegate) delete m_serverDelegate;
}


void Server::setDelegate(Host const host, Type const type)
{
   if (m_serverDelegate) delete m_serverDelegate;

   switch (type) {
      case Basic:
         m_serverDelegate = new BasicServer(this, m_delegateDefaults);
         break;
      case PBS:
         m_serverDelegate = new PBSServer(this, m_delegateDefaults);
         break;
      case SGE:
         m_serverDelegate = new SGEServer(this, m_delegateDefaults);
         break;
      case HTTP:
         m_serverDelegate = new HttpServer(this, m_delegateDefaults);
         break;
   }

   m_serverDelegate->setHostDelegate(host);
}


QStringList Server::tableFields() const
{
   QStringList fields;
   fields << m_name
          << m_hostAddress
          << m_userName
          << toString(m_authentication)
          << toString(m_host) + " - " + toString(m_type);
   return fields;
}


int Server::updateInterval() const
{
   return m_updateTimer.interval()/1000;
}


void Server::setUpdateInterval(int const seconds)
{
   m_updateTimer.setInterval(seconds*1000); 
}


void Server::setDefaults(Host const host, Type const type)
{
   switch (host) {
      case Local:   setLocalDefaults();   break;
      case Remote:  setRemoteDefaults();  break;
      case Web:     setWebDefaults();     break;
   }

   switch (type) {
      case Basic:  setBasicDefaults(host);  break;
      case PBS:    setPBSDefaults(host);    break;
      case SGE:    setSGEDefaults(host);    break;
      case HTTP:   /* nowt to do */         break;
   }
}


/// Ideally these defaults should be moved to their respectiv Host or Delegate,
/// however, when creating new Servers, the Hosts and Delegates are not created
/// until they are actually needed, but they still need to be configured in the 
/// dialog.


void Server::setLocalDefaults()
{
   m_host             = Local;
   m_type             = Basic;
   m_qchemEnvironment = QString(qgetenv("QC"));
   m_hostAddress      = "localhost";
   m_userName         = QString(qgetenv("USER"));
   m_authentication   = None;
   m_port             = 0;
   m_workingDirectory = QString(qgetenv("HOME"));
   m_executableName   = "qcprog.exe";
   setUpdateInterval(10);
}


void Server::setRemoteDefaults()
{
   m_host             = Remote;
   m_type             = Basic;
   m_qchemEnvironment = "";
   m_hostAddress      = "";
   m_userName         = QString(qgetenv("USER"));
   m_authentication   = Agent;
   m_port             = 22;
   m_workingDirectory =  "~/";
   m_executableName   = "qcprog.exe";
   setBasicDefaults(m_host);
   setUpdateInterval(20);
}


void Server::setWebDefaults()
{
   m_host             = Web;
   m_type             = HTTP;
   m_qchemEnvironment = "(unused)";
   m_hostAddress      = "www.iqmol.org";
   m_userName         = "guest";
   m_port             = 80;
   m_workingDirectory = "(unused)";
   m_authentication   = None;
   setUpdateInterval(20);

   m_executableName   = "/cgi/IQmol";
   m_queueInfo        = "limits.cgi user=${USER}";
   m_killCommand      = "kill.cgi jobID=${JOB_ID}";
   m_queryCommand     = "query.cgi jobID=${JOB_ID}";
   m_submitCommand    = "submit.cgi user=${USER} jobID=${JOB_ID}";
   m_runFileTemplate  = "(unused)";
   m_delegateDefaults.insert("JobLimit", 0);
}


void Server::setBasicDefaults(Host const host)
{
   m_type            = Basic;
   m_queueInfo       = "(unused)";
   m_killCommand     = System::KillCommand();
   m_queryCommand    = System::QueryCommand();
   m_runFileTemplate = "#! /bin/csh\nsource ~/.cshrc\nqchem ${JOB_NAME}.inp ${JOB_NAME}.out";

   if (host == Local) {
#ifdef Q_WS_WIN
      m_submitCommand   = "${QC}/qcenv_s.bat ${JOB_NAME}.inp ${JOB_NAME}.out";
     // note we make the textedit read-only in ServerOptionsDialog
      m_runFileTemplate = "(unused)";  
#else
      m_submitCommand   = "./${JOB_NAME}.run";
#endif
   }else {
      m_submitCommand   = "nohup ./${JOB_NAME}.run < /dev/null >& ${JOB_NAME}.err";
      m_runFileTemplate += " &";
   }

   m_delegateDefaults.insert("JobLimit", 0);
}


void Server::setPBSDefaults(Host const)
{
   m_type = PBS;
   m_submitCommand = "qsub ${JOB_NAME}.run";
   m_queryCommand  = "qstat -f ${JOB_ID}";
   m_queueInfo     = "qstat -fQ";
   m_killCommand   = "qdel ${JOB_ID}";

   QStringList runFile;
   runFile << "#! /bin/csh"
           << "#PBS -q ${QUEUE}"
           << "#PBS -l walltime=${WALLTIME}"
           << "#PBS -l vmem=${MEMORY}Mb"
           << "#PBS -l jobfs=${SCRATCH}Mb"
           << "#PBS -l ncpus=${NCPUS}"
           << "#PBS -j oe"
           << "#PBS -o ${JOB_NAME}.err"
           << "#PBS -wd"
           << ""
           << "setenv QCSCRATCH $PBS_JOBFS"
           << "qchem ${JOB_NAME}.inp ${JOB_NAME}.out"
           << "pbs_rusage $PBS_JOBID >> ${JOB_NAME}.out";

   m_runFileTemplate = runFile.join("\n");
}


void Server::setSGEDefaults(Host const)
{
   m_type = SGE;
   m_submitCommand = "qsub ${JOB_NAME}.run";
   // This is annoying, but SGE qstat -j doesn't give us the status.
   m_queryCommand  = "qstat && qstat -j ${JOB_ID}";
   m_queueInfo     = "qstat -g c";
   m_killCommand   = "qdel ${JOB_ID}";

   // this assumes the $QC is set up in .bashrc
   QStringList runFile;
   runFile << "#!/bin/bash"
           << "#$ -S /bin/bash"
           << "#$ -pe mpi 1"
           << "#$ -j yes"            //merge stderr stdout
           << "#$ -l h_rt=${WALLTIME}"
           << "#$ -l h_vmem=${MEMORY}"
           << "#$ -l scr_free=${SCRATCH}"
           << "#$ -cwd"     
           << ""
           << "export QCSCRATCH=/scratch"
           << "qchem ${JOB_NAME}.inp ${JOB_NAME}.out";

   m_runFileTemplate = runFile.join("\n");
}


int Server::jobLimit() const 
{
   int limit(0);
   if (m_delegateDefaults.contains("JobLimit")) {
      bool ok;
      limit = m_delegateDefaults.value("JobLimit").toInt(&ok);
      if (!ok) limit = 0;
   }
   return limit;
}


void Server::setJobLimit(int const limit) 
{
   m_delegateDefaults.insert("JobLimit", limit);
}


void Server::setCookie(QString const& cookie)
{
   m_cookie = cookie;
   ServerRegistry::saveToPreferences();
}


QVariant Server::toQVariant()
{
   QVariantMap map;
   map.insert("Name", m_name); 
   map.insert("Host", (int)m_host);
   map.insert("Type", (int)m_type);
   map.insert("QChemEnvironment", m_qchemEnvironment);
   map.insert("HostAddress", m_hostAddress);
   map.insert("UserName", m_userName);
   map.insert("Authentication", (int)m_authentication);
   map.insert("Port", m_port);
   map.insert("WorkingDirectory", m_workingDirectory);
   map.insert("ExecutableName", m_executableName);
   map.insert("SubmitCommand", m_submitCommand);
   map.insert("QueryCommand", m_queryCommand);
   map.insert("QueueInfo", m_queueInfo);
   map.insert("KillCommand", m_killCommand);
   map.insert("RunFileTemplate", m_runFileTemplate);
   map.insert("UpdateInterval", updateInterval());
   map.insert("Cookie", m_cookie);
   
   if (m_serverDelegate) m_delegateDefaults = m_serverDelegate->delegateDefaults();
   map.insert("DelegateDefaults", QVariant(m_delegateDefaults));

   return QVariant(map);
}


void Server::fromQVariant(QVariant const& qvar)
{
   QVariantMap map(qvar.toMap());
   setDefaults(Local, Basic);
   bool ok;
    
   // Name
   if (map.contains("Name") && !map.value("Name").toString().isEmpty()) {
      m_name = map.value("Name").toString();
   } else {
      throw Server::Exception("???", "Server name not set");
   }

   // Host
   if (map.contains("Host")) {
      int host(map.value("Host").toInt(&ok));
      if (ok) {
         if (host == Remote) setRemoteDefaults();
         if (host == Web) {
            setWebDefaults();
         }
      }
   }else {
      throw Server::Exception(m_name, "Remote/Local type not set");
   }

   // Type
   if (map.contains("Type")) {
      int type(map.value("Type").toInt(&ok));
      if (ok) {
         switch (type) {
            case PBS:   m_type = PBS;    break;
            case SGE:   m_type = SGE;    break;
            case HTTP:  m_type = HTTP;   break;
            default:  m_type = Basic;    break;
         }
      }
   }else {
      throw Server::Exception(m_name, "Server type not set");
   }

   // Job Limit
   if (map.contains("JobLimit")) {
      int limit(map.value("JobLimit").toInt(&ok));
      if (ok) m_delegateDefaults.insert("JobLimit", limit);
   }

   // Cookie (this should be shifted to the delegate
   if (map.contains("Cookie")) {
      m_cookie = map.value("Cookie").toString();
   }

   // $QC
   if (map.contains("QChemEnvironment")) {
	  m_qchemEnvironment = map.value("QChemEnvironment").toString();
   }

   // Host Address
   if (map.contains("HostAddress")) {
	  m_hostAddress = map.value("HostAddress").toString();
   }

   // User
   if (map.contains("UserName")) {
	  m_userName = map.value("UserName").toString();
   }

   // Authentication
   if (map.contains("Authentication")) {
      int auth(map.value("Authentication").toInt(&ok));
      if (ok) {
         switch (auth) {
            case 0:  m_authentication = None;                 break;
            case 1:  m_authentication = Agent;                break;
            case 2:  m_authentication = PublicKey;            break;
            case 3:  m_authentication = HostBased;            break;
            case 4:  m_authentication = KeyboardInteractive;  break;
            case 5:  m_authentication = Vault;                break;
            case 6:  m_authentication = Prompt;               break;
         }
      }
   }

   // Port
   if (map.contains("Port")) {
      int port(map.value("Port").toInt(&ok));
	  if (ok) m_port = port;
   }

   // Working Directory
   if (map.contains("WorkingDirectory")) {
	  m_workingDirectory = map.value("WorkingDirectory").toString();
   }

   // ${EXE_NAME}
   if (map.contains("ExecutableName")) {
	  m_executableName = map.value("ExecutableName").toString();
   }

   // ${SUBMIT_CMD}
   if (map.contains("SubmitCommand")) {
	  m_submitCommand = map.value("SubmitCommand").toString();
   }

   // ${QUERY_CMD}
   if (map.contains("QueryCommand")) {
	  m_queryCommand = map.value("QueryCommand").toString();
   }

   // ${QUEUE_INFO}
   if (map.contains("QueueInfo")) {
	  m_queueInfo = map.value("QueueInfo").toString();
   }

   // ${KILL_CMD}
   if (map.contains("KillCommand")) {
	  m_killCommand = map.value("KillCommand").toString();
   }

   // Run File Template
   if (map.contains("RunFileTemplate")) {
	  m_runFileTemplate = map.value("RunFileTemplate").toString();
   }

   // UpdateInterval
   if (map.contains("UpdateInterval")) {
      int interval(map.value("UpdateInterval").toInt(&ok));
	  if (ok) setUpdateInterval(interval);
   }

   if (map.contains("DelegateDefaults")) {
      m_delegateDefaults = map.value("DelegateDefaults").toMap();
   }
}


bool Server::connectServer()
{
   if (m_serverDelegate == 0) setDelegate(m_host, m_type);
   bool connected(isConnected());
   if (!connected) connected = m_serverDelegate->connectServer();
   if (connected) startTimer();
   return connected;
}


void Server::disconnectServer() 
{
   if (m_serverDelegate) {
      m_serverDelegate->disconnectServer();
      delete m_serverDelegate;
      m_serverDelegate = 0;
   }
}


bool Server::isConnected() 
{
   bool connected(false);
   if (m_serverDelegate) connected = m_serverDelegate->isConnected();
   return connected;
}


void Server::startTimer() 
{ 
   if (!m_updateTimer.isActive()) m_updateTimer.start();  
}


QString Server::replaceMacros(QString const& input, Process* process)
{
   QString output(input);
qDebug() << "Server::replaceMacros on string:" << output;

   output.replace("${USER}",      m_userName);
   output.replace("${QC}",        m_qchemEnvironment);
   output.replace("${EXE_NAME}",  m_executableName);
   // bit of a hack, we don't need the executable name for an HTTP server, so
   // we store the location of the cgi scripts instead.
   output.replace("${CGI_ROOT}",  m_executableName);  
   output.replace("${SERVER}",    m_name);  

   if (process) {
      JobInfo* jobInfo(process->jobInfo());
      output.replace("${JOB_ID}",   process->id());
      output.replace("${JOB_NAME}", jobInfo->get(JobInfo::BaseName));
      output.replace("${QUEUE}",    jobInfo->get(JobInfo::Queue));
      output.replace("${WALLTIME}", jobInfo->get(JobInfo::Walltime));
      output.replace("${MEMORY}",   jobInfo->get(JobInfo::Memory));
      output.replace("${JOBFS}",    jobInfo->get(JobInfo::Scratch));
      output.replace("${SCRATCH}",  jobInfo->get(JobInfo::Scratch));
      output.replace("${NCPUS}",    jobInfo->get(JobInfo::Ncpus));
   }

   if (output.contains("${")) {
      QLOG_WARN() << "Unmatched macros found in string:";
      QLOG_WARN() << input;
   }

   QLOG_DEBUG() << "Substituted output: " << output;

   return output;
}


bool Server::getWorkingDirectoryFromUser(Process* process)
{
   bool ok(false);
   if (connectServer()) ok = m_serverDelegate->getWorkingDirectoryFromUser(process);
   return ok;
}


bool Server::configureJob(Process* process)
{
   bool ok(false);
   if (connectServer()) ok = m_serverDelegate->configureJob(process);
   return ok;
}


ServerTask::Base* Server::testConfiguration() 
{
   ServerTask::Base* task(0);
   if (connectServer()) task = m_serverDelegate->testConfiguration();
   return task;
}


ServerTask::Base* Server::setup(Process* process) 
{
   ServerTask::Base* task(0); 
   if (connectServer()) task = m_serverDelegate->setup(process);
   return task;
}


ServerTask::Base* Server::submit(Process* process) 
{
   ServerTask::Base* task(0); 
   if (connectServer()) task = m_serverDelegate->submit(process);
   return task;
}


ServerTask::Base* Server::kill(Process* process) 
{
   ServerTask::Base* task(0); 
   if (connectServer()) task = m_serverDelegate->kill(process);
   return task;
}


ServerTask::Base* Server::cleanUp(Process* process) 
{
   ServerTask::Base* task(0); 
   if (connectServer()) task = m_serverDelegate->cleanUp(process);
   return task;
}


ServerTask::Base* Server::copyResults(Process* process) 
{
   ServerTask::Base* task(0); 
   if (connectServer()) task = m_serverDelegate->copyResults(process);
   return task;
}


void Server::addToWatchList(Process* process)
{
   if (m_serverDelegate == 0) setDelegate(m_host, m_type);
   m_serverDelegate->addToWatchList(process);
}


void Server::removeFromWatchList(Process* process)
{
   m_watchList.removeAll(process);
   BasicServer* basic = qobject_cast<BasicServer*>(m_serverDelegate);
   if (basic) basic->runQueue();
}


// Servers should only be watching Processes that are Queued, Running,
// Suspended or possibly Unknown.
ServerTask::Base* Server::query(Process* process) 
{
   ServerTask::Base* task(0);
   if (m_watchList.contains(process)) {
      if (isConnected()) {
         task = m_serverDelegate->query(process);
      }else {
         QMsgBox::information(0, "IQmol", "Server not connected");
      }

   }else {
      QLOG_WARN() << "Server::query called on unwatched process";
   }
   return task;
}


void Server::updateProcesses()
{
   if (m_watchList.isEmpty()  || !isConnected() ) return;

   // query may trigger the process to be removed from m_watchList, so we make
   // a copy of the list so we don't iterate past the end.
   QList<Process*> watchList(m_watchList);
   QList<Process*>::iterator iter;
   for (iter = watchList.begin(); iter != watchList.end(); ++iter) {
       ServerTask::Base* task(query(*iter));
       if (task) {
          connect(task, SIGNAL(finished()), this, SLOT(queryFinished()));
          task->start();
       }
   }
}


void Server::queryFinished()
{
   ServerTask::Query* task = qobject_cast<ServerTask::Query*>(sender());
   if (!task) return;

   Process* process(task->process());
   Process::Status oldStatus(process->status());
   Process::Status newStatus(task->newStatus());
   task->deleteLater();

   // Check for any error and bail cos, really, we don't know what's going on
   QString msg(task->errorMessage());
   if (!msg.isEmpty()) {
      process->setComment(msg);
      process->setStatus(Process::Unknown);
      return;
   }

   if (oldStatus == Process::Queued    || 
       oldStatus == Process::Running   || 
       oldStatus == Process::Suspended ||
       oldStatus == Process::Unknown) {

      if (newStatus == Process::Unknown) {
         ServerTask::Base* task(cleanUp(process));
         if (task) {
            connect(task, SIGNAL(finished()), this, SLOT(cleanUpFinished()));
            task->start();
         }
         removeFromWatchList(process);
         return;
      }else if (newStatus == Process::Queued    || 
                newStatus == Process::Running   || 
                newStatus == Process::Suspended) {
         process->setStatus(newStatus);
      }else {
         process->setStatus(newStatus);
         removeFromWatchList(process);
         QLOG_WARN() << "Inactive process found in Server::queryFinished";
      }
      
   }else {
      removeFromWatchList(process);
   }
}


void Server::cleanUpFinished()
{
   ServerTask::CleanUp* task = qobject_cast<ServerTask::CleanUp*>(sender());
   if (!task) return;

   QString err(task->runTimeError());
   Process* process(task->process());
   if (err.isEmpty()) {
      process->setStatus(Process::Finished);
   }else {
      process->setStatus(Process::Error);
      process->setComment(err);
   }

   qDebug() << "Task time set to " << task->time();
   if (task->time() > 0) {
      qDebug() << "resetting timer with " << task->time();
      process->resetTimer(task->time());
   }
   
}


int Server::watchedWithStatus(Process::Status const status)
{
   int count(0);

   QList<Process*>::iterator process;
   for (process = m_watchList.begin(); process != m_watchList.end(); ++process) {
       if ((*process)->status() == status) ++count;
   }

   return count;
}



} // end namespace IQmol
