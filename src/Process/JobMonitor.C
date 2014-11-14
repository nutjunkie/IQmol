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

#include "JobMonitor.h"
#include "Job.h"
#include "QChemJobInfo.h"
#include "QueueResources.h"
#include "QueueResourcesDialog.h"
#include "Server2.h"
#include "ServerRegistry2.h"
#include "Preferences.h"
#include "NetworkException.h"
#include "QMsgBox.h"
#include "QsLog.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QShowEvent>
#include <QHeaderView>
#include <QDir>
#include <QDebug>


namespace IQmol {
namespace Process2 {


JobMonitor* JobMonitor::s_instance = 0;
QMap<Job*, QTableWidgetItem*> JobMonitor::s_jobMap = QMap<Job*, QTableWidgetItem*>();
JobList JobMonitor::s_deletedJobs = QList<Job*>();
   

JobMonitor& JobMonitor::instance()
{
   if (s_instance == 0) {
      s_instance = new JobMonitor(0);
      atexit(JobMonitor::destroy);
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
   table->setColumnWidth(0,160);
   table->setColumnWidth(2,100);
   table->setColumnWidth(3,100);

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
   loadJobListFromPreferences();
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
      connect(action, SIGNAL(triggered()), this, SLOT(clearAllJobs()));

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


void JobMonitor::submitJob(QChemJobInfo& qchemJobInfo)
{
   Job* job(0);
   try {
      qDebug() << "***** Job submitted to new JobMonitor *****";
      qchemJobInfo.dump();

      QString serverName(qchemJobInfo.get(QChemJobInfo::ServerName));
      Server* server(ServerRegistry::instance().find(serverName));

      if (!server) {
         QString msg("Invalid server: ");
         msg += serverName;
         QMsgBox::warning(this, "IQmol", msg);
         return;
      }

      postUpdateMessage("Connecting to server...");

      if (!server->open()) {
         QString msg("Failed to open connection to server ");
         msg += serverName;
         QMsgBox::warning(this, "IQmol", msg);
         postUpdateMessage("");
         return;
      }

      if (!server->isWebBased()) {
         postUpdateMessage("Determining working directory...");
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
      }

      postUpdateMessage("Submitting job");

      job = new Job(qchemJobInfo);
      server->submit(job);
      jobAccepted();  // Closes the QUI window

   }catch (Network::AuthenticationCancelled& err) {
      if (job) s_deletedJobs.append(job);

   }catch (Network::AuthenticationError& err) {
      if (job) s_deletedJobs.append(job);
      postUpdateMessage("");
      QMsgBox::warning(this, "IQmol", "Invalid username or password");

   }catch (Exception& err) {
      if (job) s_deletedJobs.append(job);
      postUpdateMessage("");
      QMsgBox::warning(this, "IQmol", err.what());
   }
}


void JobMonitor::jobSubmissionSuccessful(Job* job)
{
   addToTable(job);
}


void JobMonitor::jobSubmissionFailed(Job* job)
{
   if (!job) return;
   QString msg("Job submission failed:\n");
   msg += job->message();
   QMsgBox::warning(this, "IQmol", msg);
   if (job) s_deletedJobs.append(job);
}


QString JobMonitor::getWorkingDirectory(QString const& msg, QString const& suggestion)
{
   bool okPushed(false);
   QString dirName(suggestion);
   dirName = QInputDialog::getText(0, "IQmol", msg, QLineEdit::Normal, dirName, &okPushed);
   if (!okPushed) return QString();

   while (dirName.endsWith("/"))  { dirName.chop(1); }
   while (dirName.endsWith("\\")) { dirName.chop(1); }
   return dirName; 
}


bool JobMonitor::getWorkingDirectory(Server* server, QChemJobInfo& qchemJobInfo)
{
   ServerConfiguration configuration(server->configuration());
   QString dirPath(configuration.value(ServerConfiguration::WorkingDirectory));
   dirPath += "/" + qchemJobInfo.get(QChemJobInfo::BaseName);

   // clean the path
   QDir dir(dirPath);
   dirPath = dir.path();
   
   bool exists(false);
   QString msg("Working directory");
   if (!server->isLocal()) msg += " on " + server->name();

   do {
      dirPath = getWorkingDirectory(msg, dirPath);
qDebug() << "Directory in submitJob:" << dirPath;
      if (dirPath.isEmpty()) return false;
      exists = server->exists(dirPath);
      QString s("Directory " + dirPath + " exists.  Overwrite?");
      if (exists && QMsgBox::question(this, "IQmol", s) == QMessageBox::Ok) exists = false;
   } while (exists);

   if (!server->makeDirectory(dirPath)) {
      QString msg("Failed to create working directory ");
      msg += dirPath;
      QMsgBox::warning(this, "IQmol", msg);
      return false;
   }

   qchemJobInfo.set(QChemJobInfo::RemoteWorkingDirectory, dirPath);
   if (server->isLocal()) {
      qchemJobInfo.set(QChemJobInfo::LocalWorkingDirectory, dirPath);
   }

   return true;
}


bool JobMonitor::getQueueResources(Server* server, QChemJobInfo& qchemJobInfo)
{
   ServerConfiguration configuration(server->configuration());
   QVariant qvar(configuration.value(ServerConfiguration::QueueResources));
   QueueResourcesList list(qvar);

   if (list.isEmpty()) {
      QString info(server->queueInfo());
      ServerConfiguration::QueueSystemT queueSystem(configuration.queueSystem());
      if (queueSystem == ServerConfiguration::PBS) {
         list.fromPbsQueueInfoString(info);
      }else if (queueSystem == ServerConfiguration::SGE) {
          list.fromSgeQueueInfoString(info);
      }
   }

   postUpdateMessage("Setting job resources");
   QueueResourcesDialog dialog(list, this);
   if (dialog.exec() == QDialog::Rejected) return false;
   configuration.setValue(ServerConfiguration::QueueResources,list.toQVariant());
   ServerRegistry::save();

   qchemJobInfo.set(QChemJobInfo::Queue,    dialog.queue());
   qchemJobInfo.set(QChemJobInfo::Walltime, dialog.walltime());
   qchemJobInfo.set(QChemJobInfo::Memory,   dialog.memory());
   qchemJobInfo.set(QChemJobInfo::Scratch,  dialog.scratch());
   qchemJobInfo.set(QChemJobInfo::Ncpus,    dialog.ncpus());

   return true;
}


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
   table->item(row, 3)->setText(Util::Timer::formatTime(job->runTime()));
   table->item(row, 4)->setText(job->message());

   s_jobMap.insert(job, table->item(row,0));

   connect(job, SIGNAL(updated()),  this, SLOT(jobUpdated()));
   saveJobListToPreferences();
}


void JobMonitor::saveJobListToPreferences() const
{
   JobList jobs(s_jobMap.keys());
   JobList::iterator iter;
   QVariantList list;

   for (iter = jobs.begin(); iter != jobs.end(); ++iter) {
       list.append((*iter)->toQVariant());
   }   

   Preferences::JobMonitorList(list);
}


void JobMonitor::loadJobListFromPreferences()
{
   QLOG_DEBUG() << "Loading jobs from preferences file";
   QVariantList list(Preferences::JobMonitorList());
   if (list.isEmpty()) return;

   bool remoteJobsActive(false);
   QVariantList::iterator iter;

   for (iter = list.begin(); iter != list.end(); ++iter) {
       
       Job* job(new Job());

       if (job->fromQVariant(*iter)) {
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
              if (job->status() == Job::Running || job->status() == Job::Suspended) {
                  qDebug() << "Setting Job status to unknown";
                  job->setStatus(Job::Unknown);
              }
          }

       }else {
          delete job;
       }
   }

   updateTable();

   if (remoteJobsActive) {
      QString msg("IQmol found processes on remote servers that were still active "
        "in the last session. \nWould you like to reconnect to the server(s)?");
      if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Ok) reconnectServers();
   }
}


void JobMonitor::on_clearListButton_clicked(bool)
{
   bool statusCheck(true);
   clearJobTable(statusCheck);
}


void JobMonitor::clearAllJobs()
{
   QString msg("Remove all jobs from the list?\n");
   msg += "This will stop the monitoring of the jobs, but not stop running processes";
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
          QTableWidgetItem* item(s_jobMap.value(job));
          m_ui.processTable->removeRow(item->row());
          s_jobMap.remove(job);
          Server* server(ServerRegistry::instance().find(job->serverName()));
          if (server) server->unwatchJob(job);
       }
   }

   saveJobListToPreferences();
}


void JobMonitor::updateTable()
{
   QTableWidget* table(m_ui.processTable);
   bool sortingEnabled(table->isSortingEnabled());
   table->setSortingEnabled(false);

   JobList list(s_jobMap.keys());
   JobList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       reloadJob(*iter);
   }

   table->setSortingEnabled(sortingEnabled);
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

   ServerRegistry::instance().connectServers(servers);
}



// --------------- Context Menu Actions ---------------
void JobMonitor::contextMenu(QPoint const& pos)
{
   QTableWidget* table(m_ui.processTable);
   QTableWidgetItem* item(table->itemAt(pos));
   if (!item) return;

   Job* job(getSelectedJob(item));
   if (!job) return;

   Job::Status status(job->status());
   if (status == Job::Copying) return;

   QMenu *menu = new QMenu(this);
   QAction* kill;

   if (status == Job::Queued) {
      kill = menu->addAction(tr("Delete Job From Queue"), this, SLOT(killJob()));
   }else {
      kill = menu->addAction(tr("Kill Job"),         this, SLOT(killJob()));
   }

   QAction* remove = menu->addAction(tr("Remove Process"),   this, SLOT(removeJob()));
   QAction* query  = menu->addAction(tr("Query Process"),    this, SLOT(queryJob()));
   QAction* view   = menu->addAction(tr("View Output File"), job,  SLOT(viewOutput()));
   QAction* open   = menu->addAction(tr("Visualize Results"),        this, SLOT(openOutput()));
   QAction* copy   = menu->addAction(tr("Copy Results From Server"), this, SLOT(copyResults()));

   kill->setEnabled(false);
   query->setEnabled(false);
   remove->setEnabled(false);
   view->setEnabled(false);
   open->setEnabled(false);
   copy->setEnabled(false);

   switch (status) {
      case Job::NotRunning:
         remove->setEnabled(true);
         break;

      case Job::Queued:
         kill->setEnabled(true);
         break;

      case Job::Running:
         kill->setEnabled(true);
         break;

      case Job::Suspended:
         kill->setEnabled(true);
         break;

      case Job::Unknown:
         remove->setEnabled(true);
         break;

      case Job::Killed:
//         view->setEnabled(true);
         remove->setEnabled(true);
         copy->setEnabled(true);
         break;

      case Job::Error:
//         view->setEnabled(true);
         remove->setEnabled(true);
         copy->setEnabled(true);
         break;

      case Job::Finished:
 //        view->setEnabled(true);
         remove->setEnabled(true);
         copy->setEnabled(true);
         break;

      case Job::Copying:
         break;
   }

   if (status == Job::Finished && job->localFilesExist()) {
       open->setEnabled(true);
   }

   menu->exec(table->mapToGlobal(pos));
   delete menu;
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
    
   Server* server = ServerRegistry::instance().find(job->serverName());
   if (server) server->kill(job);
}


void JobMonitor::copyResults()
{    
   Job* job(getSelectedJob());
   if (!job) return;
}


void JobMonitor::queryJob()
{    
   Job* job(getSelectedJob());
   if (!job) return;
}



void JobMonitor::removeJob()
{
   Job* job(getSelectedJob());
   if (!job) return;

   QTableWidgetItem* item(s_jobMap.value(job));
   m_ui.processTable->removeRow(item->row());
   s_jobMap.remove(job);
   Server* server = ServerRegistry::instance().find(job->serverName());
   if (server) server->unwatchJob(job);
   saveJobListToPreferences();

   // we only delete the jobs when IQmol quits, in case there are still connections
   s_deletedJobs.append(job);
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


void JobMonitor::jobFinished()
{
   Job* job = qobject_cast<Job*>(sender());
   if (job) {
      switch (job->status()) {
         case Job::Finished:
            QMsgBox::question(this, "IQmol", "Job finished, copy results?");
            break;
         case Job::Error:
            QMsgBox::warning(this, "IQmol", "Job finished with error");
            break;
         default:
            QMsgBox::warning(this, "IQmol", "Job finished with funny status");
            break;
      }
   }
}


void JobMonitor::jobUpdated()
{
   Job* job = qobject_cast<Job*>(sender());
   if (job) reloadJob(job);
}


void JobMonitor::reloadJob(Job* job)
{
   if (!s_jobMap.contains(job)) {
      QLOG_WARN() << "Update called on unknown Job";
      return;
   }

   QTableWidgetItem* item(s_jobMap.value(job));
   QTableWidget* table(m_ui.processTable);
   bool sortingEnabled(table->isSortingEnabled());

   // only update the runtime and status columns
   table->setSortingEnabled(false);
   table->item(item->row(), 3)->setText(Util::Timer::formatTime(job->runTime()));
   table->item(item->row(), 4)->setText(job->message());
   table->setSortingEnabled(sortingEnabled);
}

} } // end namespace IQmol::Process
