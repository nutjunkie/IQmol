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

#include "JobMonitor.h"
#include "Job.h"
#include "QChemJobInfo.h"
#include "QChemOutputParser.h"
#include "QueueResourcesList.h"
#include "QueueResourcesDialog.h"
#include "Server.h"
#include "ServerRegistry.h"
#include "Preferences.h"
#include "RemoveDirectory.h"
#include "NetworkException.h"
#include "QMsgBox.h"
#include "QsLog.h"
#include "FileLayer.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QShowEvent>
#include <QHeaderView>
#include <QFileDialog>
#include <QProgressBar>
#include <QDir>
#include <QDebug>


namespace IQmol {
namespace Process {


JobMonitor* JobMonitor::s_instance = 0;
QMap<Job*, QTableWidgetItem*> JobMonitor::s_jobMap = QMap<Job*, QTableWidgetItem*>();
JobList JobMonitor::s_deletedJobs = QList<Job*>();
   

JobMonitor& JobMonitor::instance()
{
   if (s_instance == 0) {
      s_instance = new JobMonitor(0);
      //atexit(JobMonitor::destroy);
   }
   return *s_instance;
}


void JobMonitor::destroy()
{
   s_instance->saveJobListToPreferences();
   JobList jobs(s_jobMap.keys());
   JobList::iterator iter;
   for (iter = jobs.begin(); iter != jobs.end(); ++iter) {
       delete (*iter);
   }
   for (iter = s_deletedJobs.begin(); iter != s_deletedJobs.end(); ++iter) {
       delete (*iter);
   }
   delete s_instance;
}


JobMonitor::JobMonitor(QWidget* parent) : QMainWindow(parent)
{
   m_ui.setupUi(this);
   QTableWidget* table(m_ui.processTable);

#if QT_VERSION >= 0x050000
   table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#else
   table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#endif

   table->horizontalHeader()->setStretchLastSection(true);
   setStatusBar(0);

   // Alter table spacings
   table->setColumnWidth(0,140);
   table->setColumnWidth(1, 80);
   table->setColumnWidth(2,100);
   table->setColumnWidth(3,100);
   table->setColumnWidth(4,120);

   initializeMenus();
   table->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing() + 5);
      
   // Set up the context menu handler
   table->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_ui.processTable, SIGNAL(customContextMenuRequested(QPoint const&)),
      this, SLOT(contextMenu(QPoint const&)));

   // ...and the update timer;
   m_updateTimer.setInterval(1000);  // 1000 milliseconds
   connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateTable()));

   table->setSortingEnabled(false);
   //loadJobListFromPreferences();
}


void JobMonitor::initializeMenus() 
{
   QMenuBar* menubar(menuBar());
   QAction*  action;
   QMenu*    menu;

   menubar->clear();
   menu = menubar->addMenu(tr("File"));
      action = menu->addAction(tr("Reconnect Servers"));
      connect(action, SIGNAL(triggered()), this, SLOT(reconnectServers()));

      action = menu->addAction(tr("Remove All Processes"));
      connect(action, SIGNAL(triggered()), this, SLOT(removeAllJobs()));

      action = menu->addAction(tr("Close"));
      connect(action, SIGNAL(triggered()), this, SLOT(close()));
      action->setShortcut(QKeySequence::Close);
}


void JobMonitor::showEvent(QShowEvent* event)
{
   updateTable();
   m_updateTimer.start();
   event->accept();
}


void JobMonitor::closeEvent(QCloseEvent* event)
{
   m_updateTimer.stop();
   event->accept();
}


void JobMonitor::saveJobListToPreferences() const
{
   JobList jobs(s_jobMap.keys());
   JobList::iterator iter;
   QVariantList list;

   for (iter = jobs.begin(); iter != jobs.end(); ++iter) {
       list.append((*iter)->toQVariant());
   }   

   //qDebug() <<"Saving JobMonitorList" << list;
   Preferences::JobMonitorList(list);
}


void JobMonitor::loadJobListFromPreferences()
{
   QLOG_DEBUG() << "Loading jobs from preferences file";
   QVariantList list(Preferences::JobMonitorList());
   if (list.isEmpty()) return;

   bool remoteJobsActive(false);
   Job* job(0);

   try {
      QVariantList::iterator iter;
      qint64 currentJulianDay(QDate::currentDate().toJulianDay());
      qint64 cutOffDay(currentJulianDay - Preferences::DaysToRememberJobs());

      for (iter = list.begin(); iter != list.end(); ++iter) {
          job = new Job();
          if (job->fromQVariant(*iter) && job->julianDay() >= cutOffDay ) {
             if (job->julianDay() != currentJulianDay) {
                QDate date(QDate::fromJulianDay(job->julianDay()));
                job->setSubmitTime(date.toString("d MMM"));
             }
             addToTable(job);

             if (job->isActive()) {
                Server* server = ServerRegistry::instance().find(job->serverName());
                if (server) {
                   server->watchJob(job);
                   if (server->isLocal()) {
                      server->open();
                      server->query(job);
                   }else {
                      remoteJobsActive = true;
                   }
                }else {
                   QLOG_WARN() << "Unable to find server for existing job";
                }
                job->setStatus(Job::Unknown);
             }
          }else {
             delete job;
             job = 0;
          }
      }

      updateTable();

      if (remoteJobsActive) {
         QString msg("IQmol found processes on remote servers that were still active "
           "in the last session. \n\nWould you like to reconnect to the server(s)?");

         if (QMsgBox::question(this, "IQmol", msg,
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) reconnectServers();
      }

   }catch (Exception& ex) {
      if (job) s_deletedJobs.append(job);
      postUpdateMessage("");
      QMsgBox::warning(this, "IQmol", ex.what());
   }
}


Job* JobMonitor::getSelectedJob(QTableWidgetItem* item)
{
   QTableWidget* table(m_ui.processTable);

   if (item == 0) {
      QList<QTableWidgetItem*> items(table->selectedItems());
      if (items.isEmpty()) return 0;
      item = items.first();
   }

   item = table->item(item->row(), 0);
   JobList list(s_jobMap.keys(item));

   return list.isEmpty() ? 0 : list.first();  // and only
}


void JobMonitor::reconnectServers()
{
   QStringList servers;

   JobList list(s_jobMap.keys());
   JobList::iterator iter;

   for (iter = list.begin(); iter != list.end(); ++iter) {
       QString name((*iter)->serverName());
       if (!servers.contains(name)) servers.append(name);
   }

   ServerRegistry::instance().closeAllConnections();
   ServerRegistry::instance().connectServers(servers);
}


// ---------- Submit ----------

void JobMonitor::submitJob(QChemJobInfo& qchemJobInfo)
{
   Job* job(0);

   QString serverName(qchemJobInfo.serverName());
   Server* server(ServerRegistry::instance().find(serverName));

   if (!server) {
      QString msg("Invalid server: ");
      msg += serverName;
      QMsgBox::warning(this, "IQmol", msg);
      return;
   }

   try {
      // stop the update timer while we are doing this
      BlockServerUpdates bs(server);
      postUpdateMessage("Connecting to server...");
      server->open();

      if (!getWorkingDirectory(server, qchemJobInfo)) {
         postUpdateMessage("");
         return;
      }

      if (server->needsResourceLimits()) {
         postUpdateMessage("Obtaining queue information...");
         if (!getQueueResources(server, qchemJobInfo)) {
            postUpdateMessage("");
            return;
         }
      }

      postUpdateMessage("Submitting job");

      job = new Job(qchemJobInfo);
      server->submit(job);
      jobAccepted();  // Closes the QUI window

   }catch (Network::AuthenticationCancelled& ex) {
      if (job) s_deletedJobs.append(job);
      server->closeConnection();

   }catch (Network::AuthenticationError& ex) {
      if (job) s_deletedJobs.append(job);
      server->closeConnection();
      postUpdateMessage("");
      QMsgBox::warning(this, "IQmol", "Invalid username or password");

   }catch (Exception& ex) {
      if (job) s_deletedJobs.append(job);
      postUpdateMessage("");
      QMsgBox::warning(this, "IQmol", ex.what());
   }
}


bool JobMonitor::getWorkingDirectory(Server* server, QChemJobInfo& qchemJobInfo)
{
   QString dirPath;

   postUpdateMessage("Determining working directory...");

   if (server->isLocal()) {
      dirPath = Preferences::LastFileAccessed();
      QFileInfo info(dirPath);
      if (info.isFile()) dirPath = info.path();
#ifndef Q_OS_WIN32
      dirPath += "/" + qchemJobInfo.baseName();
#endif
      bool allowSpace(false);
      if (!getLocalWorkingDirectory(dirPath, allowSpace)) return false;
   }else {
      dirPath = qchemJobInfo.baseName();
      if (!getRemoteWorkingDirectory(server, dirPath)) return false;
   }

   QDir dir(dirPath);

   qchemJobInfo.setBaseName(dir.dirName());
   qchemJobInfo.set(QChemJobInfo::RemoteWorkingDirectory, dirPath);
   if (server->isLocal()) {
      qchemJobInfo.set(QChemJobInfo::LocalWorkingDirectory, dirPath);
   }

   return true;
}


bool JobMonitor::getRemoteWorkingDirectory(Server* server, QString& name)
{
   QString message;
   QString path;

   if (server->isWebBased()) {
      postUpdateMessage("Obtaining job name");
      message = "Job name:";
   }else {
      message = "Working directory on " + server->name() +":";
      path = server->configuration().value(ServerConfiguration::WorkingDirectory);
      path += "/";
   }

   QString pathName(path + name);
   bool exists(false);

   do {
      bool okPushed(false);
      name = QInputDialog::getText(0, "IQmol", message, QLineEdit::Normal, name, &okPushed);

      while (name.endsWith("/"))  { name.chop(1); }
      while (name.endsWith("\\")) { name.chop(1); }

      if (!okPushed || name.isEmpty()) return false;

      pathName = path + name;

      // clean the path
      QDir dir(pathName);
      pathName = dir.path();

      exists = server->exists(pathName);
      QString msg("Directory " + name + " exists.  Overwrite?");
      if (exists && QMsgBox::question(QApplication::activeWindow(), "IQmol", msg) == 
         QMessageBox::Ok) {
         exists = false;
       }
   } while (exists);

   if (!server->makeDirectory(pathName)) {
      QString msg("Failed to create directory on server: ");
      msg += pathName;
      QMsgBox::warning(QApplication::activeWindow(), "IQmol", msg);
      return false;
   }
   
   name = pathName;
   return true;
}


bool JobMonitor::getLocalWorkingDirectory(QString& dirName, bool allowSpace)
{
   QDir dir(dirName);
   dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

   while (1) {
      QString dirName(dir.dirName());
      dir.cdUp();
      QString dirPath(dir.path());

      QFileDialog dialog(this , "Save As", dirPath);
      dialog.selectFile(dirName);

      // The following may need to be switched depending on which version of
      // the Qt libraries are being used.
      dialog.setOption(QFileDialog::ShowDirsOnly, true);
      dialog.setFileMode(QFileDialog::Directory);
      // dialog.setFileMode(QFileDialog::DirectoryOnly);

      if (dialog.exec() == QDialog::Rejected) return false;

      QStringList list(dialog.selectedFiles());
      if (list.isEmpty()) return false; 

      dir.setPath(list.first());
      if (dir.path().isEmpty()) return false;

      if (dir.dirName().contains(" ") && !allowSpace) {
         // This situation should only arise when we are submitting a QChem
         // job locally, in which case this is effectively the job name.
         // We can't have a space in the name because QChem will barf.
         QMsgBox::warning(0, "IQmol", "Directory name cannot contain spaces");
      }else {
         if (dir.count() == 0) break;

         QString msg("Directory ");
         msg += dir.dirName() + " exists, overwrite?";

         if (QMsgBox::question(QApplication::activeWindow(), "IQmol", msg,
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            // We don't actually overwrite the directory, only the files within
            // the directory with the same name as the ones we create.
            break;
         }   
      }   
   }   

   dirName = dir.path();
   Preferences::LastFileAccessed(dirName);
   return true;

/*
   QString dirPath(dir.path());
   while (dirPath.endsWith("/")) { 
      dirPath.chop(1);
   }   
   m_jobInfo->set(JobInfo::LocalWorkingDirectory, dirPath);
*/
}

bool JobMonitor::getQueueResources(Server* server, QChemJobInfo& qchemJobInfo)
{
   ServerConfiguration& configuration(server->configuration());
   QVariantList qvar(configuration.queueResourcesList());
   QueueResourcesList list(qvar);

   if (list.isEmpty()) {
      QLOG_DEBUG() << "QueueResources List is empty, getting queue information";
      QString info(server->queueInfo());
      ServerConfiguration::QueueSystemT queueSystem(configuration.queueSystem());
      if (queueSystem == ServerConfiguration::PBS) {
         list.fromPbsQueueInfoString(info);
      }else if (queueSystem == ServerConfiguration::SGE) {
         list.fromSgeQueueInfoString(info);
      }
   }

   postUpdateMessage("Setting job resources");
   QueueResourcesDialog dialog(&list, QApplication::activeWindow());
   if (dialog.exec() == QDialog::Rejected) return false;
   configuration.setValue(ServerConfiguration::QueueResources,list.toQVariantList());
   ServerRegistry::save();

   qchemJobInfo.setQueueName(dialog.queue());
   qchemJobInfo.setWallTime(dialog.walltime());
   qchemJobInfo.setMemory(dialog.memory());
   qchemJobInfo.setScratch(dialog.scratch());
   qchemJobInfo.setNcpus(dialog.ncpus());

   return true;
}


void JobMonitor::jobSubmissionSuccessful(Job* job)
{
   addToTable(job);
}


void JobMonitor::jobSubmissionFailed(Job* job)
{
   if (!job) return;
   QString msg("Job submission failed");
   if (!(job->message().isEmpty())) msg += ":\n" + job->message();
   QMsgBox::warning(QApplication::activeWindow(), "IQmol", msg);
   s_deletedJobs.append(job);
}


// ---------- Add/Remove Jobs ----------

void JobMonitor::addToTable(Job* job) 
{
   if (!job) return;
   QTableWidget* table(m_ui.processTable);
   int row(table->rowCount());
   table->setRowCount(row+1);

   int ncol(table->columnCount());
   Q_ASSERT(ncol == 5);
   for (int i = 0; i < ncol; ++i) {
       table->setItem(row, i, new QTableWidgetItem());
   }

   table->item(row, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
   table->item(row, 3)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

   table->item(row, 0)->setText(job->jobName());
   table->item(row, 1)->setText(job->serverName());
   table->item(row, 2)->setText(job->submitTime());
   unsigned time(job->runTime());
   if (time) table->item(row, 3)->setText(Util::Timer::formatTime(time));

   QString status(Job::toString(job->status()));
   table->item(row, 4)->setText(status);
   table->item(row, 4)->setToolTip(job->message());
   table->item(row, 4)->setText(job->message());

   s_jobMap.insert(job, table->item(row,0));

   connect(job, SIGNAL(updated()),  this, SLOT(jobUpdated()));
   connect(job, SIGNAL(finished()), this, SLOT(jobFinished()));
   //connect(job, SIGNAL(error()),    this, SLOT(jobError()));
   saveJobListToPreferences();
}


void JobMonitor::removeJob()
{
   removeJob(getSelectedJob());
}


void JobMonitor::removeJob(Job* job)
{
   if (!job) return;

   QTableWidgetItem* item(s_jobMap.value(job));
   if (!item) return;

   m_ui.processTable->removeRow(item->row());
   s_jobMap.remove(job);
   s_deletedJobs.append(job);

   disconnect(job, SIGNAL(updated()),  this, SLOT(jobUpdated()));
   disconnect(job, SIGNAL(finished()), this, SLOT(jobFinished()));
   //disconnect(job, SIGNAL(error()),    this, SLOT(jobError()));

   Server* server = ServerRegistry::instance().find(job->serverName());
   if (server) server->unwatchJob(job);
   saveJobListToPreferences();
}


void JobMonitor::on_clearListButton_clicked(bool)
{
   bool statusCheck(true);
   clearJobTable(statusCheck);
}


void JobMonitor::removeAllJobs()
{
   QString msg("Remove all jobs from the list?\n\n");
   msg += "This will stop monitoring of all jobs, but will not kill running processes";
   if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Cancel) return;

   bool statusCheck(false);
   clearJobTable(statusCheck);
}


void JobMonitor::clearJobTable(bool const finishedOnly)
{
   JobList list(s_jobMap.keys());
   JobList::iterator iter;

   for (iter = list.begin(); iter != list.end(); ++iter) {
       Job* job(*iter);
       if (job->isActive() && finishedOnly) {
          // Do nothing
       }else {
          removeJob(job);
       }
   }
}


// ---------- Job updatess ----------

void JobMonitor::updateTable()
{
   JobList list(s_jobMap.keys());
   JobList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       reloadJob(*iter);
   }
}


void JobMonitor::reloadJob(Job* job)
{
   if (!job) return;
   if (!s_jobMap.contains(job)) {
      QLOG_WARN() << "Update called on unknown Job";
      return;
   }

   QTableWidgetItem* item(s_jobMap.value(job));
   QTableWidget* table(m_ui.processTable);

   // Only update the runtime and status columns. 
   // These come from the cached results in the Job
   // object, the Server is responsible for updating
   // these according to the updateInterval
   unsigned time(job->runTime());
   if (time) table->item(item->row(), 3)->setText(Util::Timer::formatTime(time));
   if (job->status() == Job::Copying) {
      //QProgressBar* bar = new QProgressBar();
      //bar->setValue(50);
      //table->setCellWidget(item->row(), 4, bar);
      table->item(item->row(), 4)->setText(job->copyProgressString());
   }else {
      QString status(Job::toString(job->status()));
      table->item(item->row(), 4)->setText(status);
   }
   table->item(item->row(), 4)->setToolTip(job->message());
}


void JobMonitor::jobUpdated()
{
   Job* job(qobject_cast<Job*>(sender()));
   reloadJob(job);
   saveJobListToPreferences();
}


void JobMonitor::jobError()
{
   Job* job = qobject_cast<Job*>(sender());
   if (!job) return;
   QString msg("Job ");
   msg += job->jobName() + " failed:\n\n";
   msg += job->message();
   QMsgBox::warning(0, "IQmol", msg);
}


void JobMonitor::jobFinished()
{
   Job* job = qobject_cast<Job*>(sender());
   if (!job) return;

   if (job->localFilesExist()) {
      cleanUp(job);
      if (job->status() == Job::Error) {
         QString msg(job->jobName() + " failed:\n");
         msg += job->message();
         QMsgBox::warning(0, "IQmol", msg);
      }else {
         resultsAvailable(job->jobInfo().get(QChemJobInfo::LocalWorkingDirectory),
                          job->jobInfo().baseName(),
                          job->jobInfo().moleculePointer());
      }
     
   }else {
      QString msg("Job " + job->jobName() + " finished.\n");
      msg += "Copy results from server?";
       
      if (QMsgBox::question(0, "IQmol", msg, QMessageBox::Yes | QMessageBox::No,
          QMessageBox::Yes) == QMessageBox::Yes) {
         copyResults(job);
      }

   }
}


// --------------- Context Menu Actions ---------------
void JobMonitor::contextMenu(QPoint const& pos)
{
   QTableWidget* table(m_ui.processTable);
   QTableWidgetItem* item(table->itemAt(pos));
   if (!item) return;

   Job* job(getSelectedJob(item));
   if (!job) return;

   QMenu *menu = new QMenu(this);
   QAction* kill;
   Job::Status status(job->status());

   switch (status) {
      case Job::Queued:   
         kill = menu->addAction("Delete Job From Queue", this, SLOT(killJob()));
         break;
      case Job::Copying:   
         kill = menu->addAction("Cancel Copy", this, SLOT(cancelCopy()));
         break;
      default:
         kill = menu->addAction("Kill Job", this, SLOT(killJob()));
         break;
   }

   QAction* remove = menu->addAction(tr("Remove Job"),               this, SLOT(removeJob()));
   QAction* query  = menu->addAction(tr("Query Job"),                this, SLOT(queryJob()));
   QAction* view   = menu->addAction(tr("View Output File"),         this, SLOT(viewOutput()));
   QAction* open   = menu->addAction(tr("Open Results"),             this, SLOT(openResults()));
   QAction* copy   = menu->addAction(tr("Copy Results From Server"), this, SLOT(copyResults()));

   kill->setEnabled(false);
   query->setEnabled(false);
   remove->setEnabled(false);
   view->setEnabled(false);
   open->setEnabled(false);
   copy->setEnabled(false);

   switch (status) {
      case Job::NotRunning:
         break;

      case Job::Queued:
         kill->setEnabled(true);
         query->setEnabled(true);
         break;

      case Job::Running:
         kill->setEnabled(true);
         query->setEnabled(true);
         break;

      case Job::Suspended:
         kill->setEnabled(true);
         query->setEnabled(true);
         break;

      case Job::Unknown:
         remove->setEnabled(true);
         query->setEnabled(true);
         break;

      case Job::Killed:
         remove->setEnabled(true);
         break;

      case Job::Error:
         remove->setEnabled(true);
         copy->setEnabled(true);
         break;

      case Job::Finished:
         remove->setEnabled(true);
         copy->setEnabled(true);
         break;

      case Job::Copying:
         kill->setEnabled(true);
         break;
   }

   Server* server = ServerRegistry::instance().find(job->serverName());
   if (server && server->isLocal()) {
      copy->setEnabled(false);
   }

   if (job->localFilesExist()) {
      view->setEnabled(true);
      if (status == Job::Finished) open->setEnabled(true);
   }

   menu->exec(table->mapToGlobal(pos));
   delete menu;
}
      

void JobMonitor::on_processTable_cellDoubleClicked(int, int)
{
   Job* job(getSelectedJob());
   if (!job) return;

   bool localFiles(job->jobInfo().localFilesExist());

   switch (job->status()) {
      case Job::Error:
         if (localFiles) {
            viewOutput(job);
         }else {
            QString msg = "Job failed:\n";
            msg += job->message();
            QMsgBox::information(this, "IQmol", msg);
         }
         break;

      case Job::Finished:
         if (localFiles) {
            openResults(job);
         }else {
            if (QMsgBox::question(this, "IQmol", "Copy results from server?",
               QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
               copyResults(job);
            }
          }
         break;

      default:
         queryJob(job);
         break;
   }
}


void JobMonitor::cancelCopy()
{
   Job* job(getSelectedJob());
   if (!job) return;

   Job::Status status(job->status());
   if (status != Job::Copying) {
      QLOG_DEBUG() << "Cancel copy called on non-copy job";
      return;
   }

   try {
      Server* server = ServerRegistry::instance().find(job->serverName());
      if (server) server->cancelCopy(job);
   } catch (Exception& ex) {
      QMsgBox::warning(this, "IQmol", ex.what());
   }
}


void JobMonitor::killJob()
{    
   Job* job(getSelectedJob());
   if (!job) return;
  
   Job::Status status(job->status());

   QString msg;
   if (status == Job::Queued) {
      msg = "Are you sure you want to remove the job ";
      msg += job->jobName() + " from the queue?";
   }else {
      msg = "Are you sure you want to terminate the job ";
      msg += job->jobName() + "?";
   }    

   if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Cancel) return;
    
   try {
      Server* server = ServerRegistry::instance().find(job->serverName());
      if (server) server->kill(job);
   } catch (Exception& ex) {
      QMsgBox::warning(this, "IQmol", ex.what());
   }
}


void JobMonitor::openResults()
{
  openResults(getSelectedJob());
}


void JobMonitor::openResults(Job* job)
{
   if (!job) return;
   resultsAvailable(job->jobInfo().get(QChemJobInfo::LocalWorkingDirectory),
                    job->jobInfo().baseName(),
                    job->jobInfo().moleculePointer());
}


void JobMonitor::viewOutput()
{    
  viewOutput(getSelectedJob());
}


void JobMonitor::viewOutput(Job* job)
{
   if (!job) return;

   QFileInfo output(job->jobInfo().getLocalFilePath(QChemJobInfo::OutputFileName));

   if (!output.exists()) {
      QMsgBox::warning(this,"IQmol", "Output file no longer exists");
      job->jobInfo().localFilesExist(false);
      return;
   }   

   Layer::File* file = new Layer::File(output.filePath());

   if (job->status() == Job::Running) {
      file->tail();
   }else {
      file->configure();
   }   
}


void JobMonitor::copyResults()
{    
  copyResults(getSelectedJob());
}


void JobMonitor::copyResults(Job* job)
{    
   if (!job) return;

   try {
      QChemJobInfo& qchemJobInfo(job->jobInfo());

      QString dirPath(qchemJobInfo.get(QChemJobInfo::LocalWorkingDirectory));
      QFileInfo info(dirPath);

      if (qchemJobInfo.localFilesExist() && info.exists()) {
         QString msg("Results are in the directory:\n\n");
         msg += dirPath;
         msg += "\n\nDownload results again?";
         if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Cancel) return;
      }

      // clean the path
      QDir dir(dirPath);
      dirPath = dir.path();
   
      bool allowSpace(true);
      if (!getLocalWorkingDirectory(dirPath, allowSpace)) return;

      qchemJobInfo.set(QChemJobInfo::LocalWorkingDirectory, dirPath);

      Server* server = ServerRegistry::instance().find(job->serverName());
      if (!server) throw Exception("Invalid server");
      server->copyResults(job);

   } catch (Exception& ex) {
      QMsgBox::warning(this, "IQmol", ex.what());
   }
}


void JobMonitor::cleanUp(Job* job)
{
   if (!job) return;
   if (job->isActive()) {
      QLOG_WARN() << "Active Job passed to CleanUp" << job->jobName();
      return;
   }else if (!job->localFilesExist()) {
      QLOG_WARN() << "Local files DNE in CleanUp" << job->jobName();
      return;
   }
    
   QDir dir (job->jobInfo().get(QChemJobInfo::LocalWorkingDirectory));
   if (!dir.exists()) {
      QMsgBox::warning(this, "IQmol", QString("Unable to find results for") + job->jobName());
      return;
   }

   QChemJobInfo& qchemJobInfo(job->jobInfo());

   // Rename Http files
   QString oldName("input"); 
   QString newName(qchemJobInfo.get(QChemJobInfo::InputFileName));
   if (dir.exists(oldName)) {
      if (dir.exists(newName)) dir.remove(newName);
      dir.rename(oldName, newName);
   }

   oldName = "output";
   newName = qchemJobInfo.get(QChemJobInfo::OutputFileName);
   if (dir.exists(oldName)) {
      if (dir.exists(newName)) dir.remove(newName);
      dir.rename(oldName, newName);
   }

   oldName = "input.FChk";
   newName = qchemJobInfo.get(QChemJobInfo::AuxFileName);
   if (dir.exists(oldName) && oldName != newName) {
      if (dir.exists(newName)) dir.remove(newName);
      dir.rename(oldName, newName);
   }

   oldName = "input.fchk";
   if (dir.exists(oldName) && oldName != newName) {
      if (dir.exists(newName)) dir.remove(newName);
      dir.rename(oldName, newName);
   }

   oldName = qchemJobInfo.get(QChemJobInfo::InputFileName) + ".fchk";
   if (dir.exists(oldName) && oldName != newName) {
      if (dir.exists(newName)) dir.remove(newName);
      dir.rename(oldName, newName);
   }

//   This is a hangover from past versions of QChem that used to write the
//   checkpoint file to Test.FChk.  Now it causes problems if the user calls
//   the job 'test' on OSX (stupid case inconsistencies).
//   oldName = "Test.FChk";
//   if (dir.exists(oldName) && oldName != newName) {
//      if (dir.exists(newName)) dir.remove(newName);
//      dir.rename(oldName, newName);
//   }

   if (dir.exists("pathtable")) dir.remove("pathtable");

   // Check for errors and update the run time
   QString output(qchemJobInfo.getLocalFilePath(QChemJobInfo::OutputFileName));
   QStringList errors(Parser::QChemOutput::parseForErrors(output));

   if (!errors.isEmpty()) {
      QString time(errors.takeLast());
      time.remove("Time: ");
      bool ok(false);
      unsigned t(time.toUInt(&ok));
      if (ok) {
         job->resetTimer(t);
         reloadJob(job);
      }
   }

   QLOG_DEBUG() << "Errors in file:" << errors;

   if (!errors.isEmpty()) {
      job->setStatus(Job::Error, errors.join("\n")); 
      reloadJob(job);
   }
}


void JobMonitor::queryJob()
{    
  queryJob(getSelectedJob());
}


void JobMonitor::queryJob(Job* job)
{    
   if (!job) return;

   try {
      Server* server = ServerRegistry::instance().find(job->serverName());
      if (!server) throw Exception("Invalid server");
      server->query(job);
   } catch (Exception& ex) {
      QMsgBox::warning(this, "IQmol", ex.what());
   }
}


} } // end namespace IQmol::Process
