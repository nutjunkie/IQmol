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

#include "ProcessMonitor.h"
#include "Process.h"
#include "JobInfo.h"
#include "Server.h"
#include "ServerRegistry.h"
#include "ServerTask.h"
#include "Preferences.h"
#include "QMsgBox.h"
#include <QCloseEvent>
#include <QShowEvent>
#include <QHeaderView>
#include <QSet>

#include <QtDebug>


namespace IQmol {


ProcessMonitor::ProcessMonitor(QWidget* parent) : QMainWindow(parent), m_pendingProcess(0)
{
   m_ui.setupUi(this);

   QTableWidget* table(m_ui.processTable);
   table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
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
   loadProcessList();
}


ProcessMonitor::~ProcessMonitor()
{
   ProcessList processes(m_processMap.keys());
   ProcessList::iterator iter;
   for (iter = processes.begin(); iter != processes.end(); ++iter) {
       delete (*iter);
   }
}


void ProcessMonitor::initializeMenus() {
   QMenuBar* menubar(menuBar());
   QAction*  action;
   QMenu*    menu;

   menubar->clear();
   menu = menubar->addMenu(tr("File"));
      action = menu->addAction(tr("Reconnect Servers"));
      connect(action, SIGNAL(triggered()), this, SLOT(reconnectServers()));
      action->setShortcut(Qt::CTRL + Qt::Key_R);


      action = menu->addAction(tr("Remove All Processes"));
      connect(action, SIGNAL(triggered()), this, SLOT(clearProcessList()));
      action->setShortcut(Qt::CTRL + Qt::Key_Backspace);

      action = menu->addAction(tr("Close"));
      connect(action, SIGNAL(triggered()), this, SLOT(close()));
      action->setShortcut(QKeySequence::Close);
}


void ProcessMonitor::showEvent(QShowEvent* event)
{
   updateTable();
   m_updateTimer.start();
   event->accept();
}


void ProcessMonitor::closeEvent(QCloseEvent* event)
{
   m_updateTimer.stop();
   event->accept();
}


void ProcessMonitor::submitJob(IQmol::JobInfo* jobInfo)
{  
   if (m_pendingProcess) {
      QString msg("Job submission pending, cannot submit additional jobs");
      QMsgBox::warning(this, "IQmol", msg);
      return;
   }

   // Because we need to thread the network stuff, we break the job submission
   // process up into several bits (ServerTasks) and connect these via signals
   // and slots. First we need to make sure the server is connected and tested. 
   // Note that because the connection process is potentially interactive, we
   // don't bother threading it.
   QString serverName(jobInfo->get(JobInfo::ServerName));
   Server* server = ServerRegistry::instance().get(serverName);

   if (!server) {
      QMsgBox::warning(this, "IQmol", "Invalid server");
      return;
   }

qDebug() << "Server is connected:" << server->isConnected();

   if (!server->isConnected()) {
      postStatusMessage("Connecting to server...");
      try {
        qDebug() << "Attempting to connect to server";
         if (!server->connectServer()) throw Server::Exception("Connection failed");
         qDebug() << "Server is connected 2:" << server->isConnected();
        
         postStatusMessage("Testing configuration...");
      } catch (std::exception& err) {
         QString msg("Failed to connect to server ");
         msg += serverName + ":\n";
         msg += err.what();
         QMsgBox::warning(this, "IQmol", msg);
         return;
      }
   }

   // This is a bit untidy, but we need to tell the next slot in the submission
   // chain what job we are dealing with.
   m_pendingProcess = new Process(jobInfo);
   ServerTask::Base* task = server->testConfiguration();
   if (task) {
      connect(task, SIGNAL(finished()), this, SLOT(submitJob1()));
      task->start();
   }
}


// At this point the server is connected and tested, but we still need to check
// if the test was successful.  If it is, then we can go ahead with the setup
// which starts with obtaining the working directory.
void ProcessMonitor::submitJob1()
{
   if (!m_pendingProcess) {
      qDebug() << "ProcessMonitor::submitJob1() called with no pending process";
      return;
   }

   ServerTask::Base* task = qobject_cast<ServerTask::Base*>(sender());
   if (!task) {
      delete m_pendingProcess;
      m_pendingProcess = 0;
      return;
   }

   QString errorMessage(task->errorMessage());
   task->deleteLater();

   if (!errorMessage.isEmpty()) {
      QString msg("Problem submitting job:\n");
      msg += errorMessage;
      QMsgBox::warning(this, "IQmol", msg);
      delete m_pendingProcess;
      m_pendingProcess = 0;
      return;
   }

   postStatusMessage("Determining working directory...");
   Server* server = ServerRegistry::instance().get(m_pendingProcess->serverName());
   if (server) {
      if (server->getWorkingDirectoryFromUser(m_pendingProcess)) {
         task = server->setup(m_pendingProcess);
         connect(task, SIGNAL(finished()), this, SLOT(submitJob2()));
         task->start();
      }else {
         delete m_pendingProcess;
         m_pendingProcess = 0;
      }
   }else {
      qDebug() << "failed to find server" << m_pendingProcess->serverName();
   }
}


// At this point we may or may not have a valid working directory in that the
// selected directory may exist and we haven't confirmed if the user wants to
// overwrite it. 
void ProcessMonitor::submitJob2()
{
   if (!m_pendingProcess) {
      qDebug() << "ProcessMonitor::submitJob2() called with no pending process";
      return;
   }

   ServerTask::Base* task = qobject_cast<ServerTask::Base*>(sender());
   if (!task) {
      delete m_pendingProcess;
      m_pendingProcess = 0;
      return;
   }

   QString errorMessage(task->errorMessage());
   task->deleteLater();

   Server* server = ServerRegistry::instance().get(m_pendingProcess->serverName());
   if (!server) {
      QMsgBox::warning(this, "IQmol", "Failed to find server");
      return;
   }

   if (errorMessage.contains("Working directory exists")) {
      if (QMsgBox::question(0, "IQmol", "Directory exists, overwrite?") == QMessageBox::Ok) {
         m_pendingProcess->jobInfo()->promptOnOverwrite(false);
      }else if (!server->getWorkingDirectoryFromUser(m_pendingProcess)) {
         delete m_pendingProcess;
         m_pendingProcess = 0;
         return;
      }
 
      task = server->setup(m_pendingProcess);
      connect(task, SIGNAL(finished()), this, SLOT(submitJob2()));
      task->start();
      return;
   }

   if (!errorMessage.isEmpty()) {
      QString msg("Problem setting up Job:\n");
      msg += errorMessage;
      QMsgBox::warning(this, "IQmol", msg);
      delete m_pendingProcess;
      m_pendingProcess = 0;
      return;
   }

   // We now have a valid working directory
   postStatusMessage("Configuring options...");
   if (server->configureJob(m_pendingProcess)) {
      postStatusMessage("Submitting job...");
      task = server->submit(m_pendingProcess);
      connect(task, SIGNAL(finished()), this, SLOT(jobSubmitted()));
      task->start();
   }else {
      delete m_pendingProcess;
      m_pendingProcess = 0;
      return;
   }
}


// At this point we have submitted the job, but we need to check if it is still running
// the job should be submitted, we check that this is indeed the
// case and add the Process to our monitor.
void ProcessMonitor::jobSubmitted()
{
   if (!m_pendingProcess) {
      qDebug() << "ProcessMonitor::submitJob3() called with no pending process";
      return;
   }

   ServerTask::Base* task = qobject_cast<ServerTask::Base*>(sender());
   if (!task) {
      delete m_pendingProcess;
      m_pendingProcess = 0;
      return;
   }

   QString errorMessage(task->errorMessage());
qDebug() << "ProcessMonitor::jobSubmitted() error message" << errorMessage;
   task->deleteLater();
// if message = Process not found then we try a clean up

   if (!errorMessage.isEmpty()) {
      QString msg("Problem submitting Job:\n");
      msg += errorMessage;
      QMsgBox::warning(this, "IQmol", msg);
      delete m_pendingProcess;
      m_pendingProcess = 0;
      return;
   }

   jobAccepted(); // tells the QUI window to close
   Server* server = ServerRegistry::instance().get(m_pendingProcess->serverName());
   if (server) server->startTimer();
   addToTable(m_pendingProcess);
   saveProcessList();
   m_pendingProcess = 0;
}



void ProcessMonitor::addToTable(Process* process) 
{
   QTableWidget* table(m_ui.processTable);
   int row(table->rowCount());
   table->setRowCount(row+1);

   int ncol(table->columnCount());
   for (int i = 0; i < ncol; ++i) {
       table->setItem(row, i, new QTableWidgetItem());
   }
   table->item(row, 2)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
   table->item(row, 3)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

   m_processMap.insert(process, table->item(row,0));
   connect(process, SIGNAL(updated()),  this, SLOT(processUpdated()));
   connect(process, SIGNAL(finished()), this, SLOT(processFinished()));
   updateRow(row, process->monitorItems());
}



void ProcessMonitor::reconnectServers()
{
   ProcessList list(m_processMap.keys());
   QSet<Server*> servers;

   ProcessList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
	   // We set the status to Unknown when we load the proceses from file if
	   // the process hasn't finished.
       if ((*iter)->status() == Process::Unknown ||
           (*iter)->status() == Process::Queued) {

          Server* server = ServerRegistry::instance().get((*iter)->serverName());
          if (server) servers.insert(server);
       }
   }

   QSet<Server*>::iterator server;   
   for (server = servers.begin(); server != servers.end(); ++server) {
       try {
          QLOG_INFO() << "Reconnecting to server" << (*server)->name();
          if (!(*server)->connectServer()) throw Server::Exception("Connection failed");
          QLOG_INFO() << "Updating Processes on server" << (*server)->name();
          (*server)->updateProcesses();
       } catch (std::exception& err) {
          QString msg("Failed to reconnect to server: ");
          msg += (*server)->name() +":\n";
          QLOG_WARN() << msg;
          QMsgBox::warning(this, "IQmol", msg);
       }
   }
}


void ProcessMonitor::on_clearListButton_clicked(bool)
{
   bool statusCheck(true);
   bool prompt(false);
   clearProcessList(statusCheck, prompt);
}


void ProcessMonitor::clearProcessList(bool const finishedOnly, bool const prompt)
{
   if (prompt) {
      QString msg("Are you sure you want to remove all processes from the list?  ");
      msg += "This will not stop running processes.";
      if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Cancel) return;
   }

   ProcessList list(m_processMap.keys());
   ProcessList::iterator iter;

   for (iter = list.begin(); iter != list.end(); ++iter) {
       Process* process(*iter);
       Process::Status status(process->status());
       if (!finishedOnly               ||
           status == Process::Killed   || 
           status == Process::Error    ||
           status == Process::Finished) 
       {
           QTableWidgetItem* item(m_processMap[process]);
           m_ui.processTable->removeRow(item->row());
           m_processMap.remove(process);
           Server* server = ServerRegistry::instance().get(process->serverName());
           if (server) server->removeFromWatchList(process);
       }
   }
   saveProcessList();
}


// --------------- Context Menu Actions ---------------

void ProcessMonitor::contextMenu(QPoint const& pos) 
{
   QTableWidget* table(m_ui.processTable);
   QTableWidgetItem* item(table->itemAt(pos));
   if (!item) return;

   Process* process(getSelectedProcess(item));
   if (!process) return;

   if (process->status() == Process::Copying) return;

   QMenu *menu = new QMenu(this);
   QAction* kill   = menu->addAction(tr("Kill Job"), this, SLOT(killProcess()));
   QAction* remove = menu->addAction(tr("Remove Process"), this, SLOT(removeProcess()));
   QAction* query  = menu->addAction(tr("Query Process"), this, SLOT(queryProcess()));
   QAction* view   = menu->addAction(tr("View Output File"), process, SLOT(viewOutput()));
   QAction* open   = menu->addAction(tr("Visualize Results"), this, SLOT(openOutput()));
   QAction* copy   = menu->addAction(tr("Copy Results From Server"), this, SLOT(copyResults()));
                       

   kill->setEnabled(false);
   query->setEnabled(true);
   remove->setEnabled(false);
   view->setEnabled(false);
   open->setEnabled(false);
   copy->setEnabled(false);

   Process::Status status(process->status());

   if (status == Process::Running || 
       status == Process::Queued  || 
       status == Process::Suspended) {
       kill->setEnabled(true);
   }

   if (status != Process::NotRunning &&
       status != Process::Killed     &&
       status != Process::Queued) {
       view->setEnabled(true); 
   }

   if (status == Process::Killed   || 
       status == Process::Error    ||
       status == Process::Finished) {
       remove->setEnabled(true); 
       copy->setEnabled(true); 
   }

   if (status == Process::Unknown ||
      status == Process::NotRunning) {
      remove->setEnabled(true);
   }

   if (status == Process::Finished &&
       process->jobInfo()->localFilesExist()) {
       open->setEnabled(true);
   }

   menu->exec(table->mapToGlobal(pos));
   delete menu;
}


void ProcessMonitor::killProcess()
{     
   Process* process(getSelectedProcess());
   if (!process) return;
  
   Process::Status status(process->status());

   QString msg;
   if (status == Process::Queued) {
      msg = "Are you sure you want to remove the job ";
      msg += process->name() + " from the queue?";
   }else {
      msg = "Are you sure you want to terminate the job ";
      msg += process->name() + "?";
   }     

   if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Cancel) return;
    
   Server* server = ServerRegistry::instance().get(process->serverName());
   ServerTask::Base* task = server->kill(process);
   if (task) {
      connect(task, SIGNAL(finished()), this, SLOT(processKilled()));
      task->start();
   }
}


void ProcessMonitor::processKilled()
{
   ServerTask::KillProcess* task = qobject_cast<ServerTask::KillProcess*>(sender());
   if (!task) {
      qDebug() << "Failed to cast KillProcess from sender";
      return;
   }

   QString errorMessage(task->errorMessage());
   Process* process(task->process());
   task->deleteLater();

   if (errorMessage.isEmpty()) {
      process->setStatus(Process::Killed);
   }else {
      QString msg("Failed to kill job ");
      msg += process->name() + "\n" + errorMessage;
      QMsgBox::warning(0, "IQmol", msg);
   }
}


void ProcessMonitor::openOutput()
{
   Process* process(getSelectedProcess());
   if (process) resultsAvailable(process->jobInfo());
}


void ProcessMonitor::copyResults()
{
   Process* process(getSelectedProcess());
   if (process) copyResults(process);
}


void ProcessMonitor::copyResults(Process* process)
{        
   if (process->jobInfo()->localFilesExist()) {
      QString msg("Results are in the directory:\n");
      msg += process->jobInfo()->get(JobInfo::LocalWorkingDirectory);
      QMsgBox::information(this, "IQmol", msg);
   }else if (process->status() == Process::Copying) {
      QString msg("Results are already being transfered.\nBe patient...");
      QMsgBox::information(this, "IQmol", msg);
   }else {
      if (process->getLocalSaveDirectory()) {
         Server* server = ServerRegistry::instance().get(process->serverName());
         ServerTask::Base* task = server->copyResults(process);
         if (task) {
            connect(task, SIGNAL(finished()), this, SLOT(resultsCopied()));
            task->start();
         }
      }
   }
} 


void ProcessMonitor::resultsCopied()
{
qDebug() << "ProcessMonitor::resultsCopied called";
   ServerTask::CopyResults* task = qobject_cast<ServerTask::CopyResults*>(sender());
   if (!task) {
      qDebug() << "Failed to cast CopyResults from sender";
      return;
   }

   QString errorMessage(task->errorMessage());
   Process* process(task->process());
   task->deleteLater();

   if (errorMessage.isEmpty()) {
      process->jobInfo()->localFilesExist(true);
      resultsAvailable(process->jobInfo());
   }else {
      QString msg("Problem copying files for job ");
      msg += process->name() + " from server " + process->serverName() + ":\n";
      msg += errorMessage;
      QMsgBox::warning(this, "IQmol", msg);
   }
}


void ProcessMonitor::removeProcess() 
{
   Process* process(getSelectedProcess());

   if (process) {
      QTableWidgetItem* item(m_processMap[process]);
      m_ui.processTable->removeRow(item->row());
      m_processMap.remove(process);
      Server* server = ServerRegistry::instance().get(process->serverName());
      server->removeFromWatchList(process);
      saveProcessList();
   }
}


void ProcessMonitor::queryProcess()
{
   Process* process(getSelectedProcess());
   if (process) queryProcess(process);
}


void ProcessMonitor::queryProcess(Process* process)
{
   QString msg;

   switch (process->status()) {
      case Process::NotRunning:
         msg = "Process not yet started";
         break;

      case Process::Queued:
      case Process::Running:
      case Process::Suspended: {
         Server* server = ServerRegistry::instance().get(process->serverName());
         if (server) {
            ServerTask::Base* task = server->query(process);
            if (task) {
               connect(task, SIGNAL(finished()), this, SLOT(queryFinished()));
               task->start();
            }
            return;
         }else {
            msg = "Server not found";
         }
      } break;
   
      case Process::Copying:
         msg = "Copying files from server";
         break;

      case Process::Killed:
         msg = "Process killed. R.I.P.";
         break;

      case Process::Error:
         msg = "Job failed:\n" + process->comment();
         break;

      case Process::Finished:
         if (process->jobInfo()->localFilesExist()) {
            msg = "Job finished.  Results are in\n";
            msg += process->jobInfo()->get(JobInfo::LocalWorkingDirectory);
         }else {
            msg = "Job finished.  Results not yet copied from server";
         }
         break;

      case Process::Unknown:
         msg = "Status unknown, possibly due to a timeout on the server";
         break;
   }

   QMsgBox::information(this, "IQmol", msg);
}


// This only needs to implement the actions that are different that those for
// queryProcess()
void ProcessMonitor::on_processTable_cellDoubleClicked(int, int) 
{
   Process* process(getSelectedProcess());
   if (!process) return;

   switch (process->status()) 
   {
      case Process::Error:
         if (process->jobInfo()->localFilesExist()) {
            process->viewOutput();
         }else {
            QString msg = "Job failed:\n";
            msg += process->comment();
            QMsgBox::information(this, "IQmol", msg);
         }
         break;

      case Process::Finished:
         if (process->jobInfo()->localFilesExist()) {
            resultsAvailable(process->jobInfo());
         }else {
            QString msg("Copy results from server?");
            if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Ok) copyResults(process);
          }
         break;

      default:
         queryProcess(process);
         break;
   } 
}


void ProcessMonitor::queryFinished()
{
   ServerTask::Base* task = qobject_cast<ServerTask::Base*>(sender());
   if (!task) return;

   QString msg(task->errorMessage());
   if (msg.isEmpty()) {
      msg = task->outputMessage();
      if (msg.isEmpty()) msg = "No information available";
      QMsgBox::information(this, "IQmol", msg);
   }else {
      QMsgBox::warning(this, "IQmol", msg);
   }
   task->deleteLater();
}


void ProcessMonitor::updateTable()
{
   ProcessList list(m_processMap.keys());
   ProcessList::iterator iter;
   QTableWidget* table(m_ui.processTable);
   QTableWidgetItem* item;

   bool sortingEnabled(table->isSortingEnabled());
   table->setSortingEnabled(false);
   for (iter = list.begin(); iter != list.end(); ++iter) {
       item = m_processMap[*iter];
   }
   for (iter = list.begin(); iter != list.end(); ++iter) {
       item = m_processMap[*iter];
       updateRow(item->row(), (*iter)->monitorItems());
   }
   table->setSortingEnabled(sortingEnabled);
}


void ProcessMonitor::processUpdated()
{
   updateTable();
   saveProcessList();
}


void ProcessMonitor::saveProcessList()
{
   ProcessList processes(m_processMap.keys());
   ProcessList::iterator iter;
   QVariantList list;

   for (iter = processes.begin(); iter != processes.end(); ++iter) {
       list.append((*iter)->serialize());
   }

   Preferences::CurrentProcessList(list);
}


void ProcessMonitor::loadProcessList()
{
   QLOG_DEBUG() << "Loading processes from file";
   QVariantList list(Preferences::CurrentProcessList());
   if (list.isEmpty()) return;

   bool remoteProcessesActive(false);
   QVariantList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       Process* process(Process::deserialize(*iter));
       if (process) {
          addToTable(process);
          if (process->status() == Process::Queued  ||
              process->status() == Process::Running ||
              process->status() == Process::Suspended ||
              process->status() == Process::Unknown) {

              qDebug() << "  Watching process" << process->name();
              Server* server = ServerRegistry::instance().get(process->serverName());
              if (server) {
                 server->addToWatchList(process);
                 if (server->host() == Server::Local) {
                    server->connectServer();
                    server->updateProcesses();
                 }else {
                    remoteProcessesActive = true;
                 }
              }
          }

          if (process->status() == Process::Running ||
              process->status() == Process::Suspended) {
              qDebug() << "  Setting status to unknown";
              process->setStatus(Process::Unknown);
          }
       }
   }
   updateTable();

   if (remoteProcessesActive) {
      QString msg("IQmol found processes on remote servers that were still active "
        "in the last session. \nWould you like to reconnect to the server(s)?");
      if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Ok) reconnectServers();
   }
}


void ProcessMonitor::processFinished()
{
   Process* process(qobject_cast<Process*>(sender()));
   if (!process) {
      qDebug() << "Process cast failed in ProcessMonitor::processFinished";
      return;
   }

   JobInfo* jobInfo(process->jobInfo());
   QString msg(jobInfo->get(JobInfo::BaseName));

   if (process->status() == Process::Error) {
      msg += " has failed:\n";
      msg += process->comment();
      QMsgBox::warning(this, "IQmol", msg);
   }else {
      msg += " has finished.";
      if (jobInfo->localFilesExist()) {
         resultsAvailable(jobInfo);
         QMsgBox::information(this, "IQmol", msg);
      }else {
         msg += "\nCopy results from server?";
         if (QMsgBox::question(this, "IQmol", msg) == QMessageBox::Ok) {
            copyResults(process);
         }
      }
   }
}


void ProcessMonitor::updateRow(int const row, QStringList const& list)
{
   QTableWidget* table(m_ui.processTable);
   int nCol(table->columnCount());
   if (list.size() < nCol) {
      qDebug() << "Incorrect table items passed to ProcessMonitor::updateRow()";
      return;
   }

   bool sortingEnabled(table->isSortingEnabled());
   table->setSortingEnabled(false);
   for (int i = 0; i < nCol; ++i) {
       table->item(row, i)->setText(list[i]);
   }
   if (list.size() > nCol) table->item(row, nCol-1)->setToolTip(list[nCol]);
   table->setSortingEnabled(sortingEnabled);
}


Process* ProcessMonitor::getSelectedProcess(QTableWidgetItem* item)
{
   QTableWidget* table(m_ui.processTable);
   
   if (item == 0) {
      QList<QTableWidgetItem*> items(table->selectedItems());
      if (items.isEmpty()) return 0;
      item = items.first();
   }

   int row(item->row());
   item = table->item(row,0);
   ProcessList list(m_processMap.keys(item));

   if (list.isEmpty()) {
      qDebug() << "Way not up with the where now??";
      return 0;
   }

   return list.first();  // and only
}


} // end namespace IQmol::Proces
