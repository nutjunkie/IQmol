/*!
 *  \file Process.C
 *  
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "FileDisplay.h"
#include "Process.h"
#include "QMsgBox.h"
#include "Qui.h"

#include <QDir>
#include <QList>
#include <QtDebug>
#include <signal.h>
#include <QHeaderView>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#ifdef QCHEM_UI
#include "../Preferences.h"
#define Preferences IQmol::Preferences
#else
#include "Preferences.h"
#endif


namespace Qui {
namespace Process {


QString ToString(Status::ID const& state) {
   QString s;

   switch (state) {
      case Status::NotRunning: { s = "Not Running"; } break;
      case Status::Starting:   { s = "Starting";    } break;
      case Status::Running:    { s = "Running";     } break;
      case Status::Queued:     { s = "Queued";      } break;
      case Status::Crashed:    { s = "Crashed";     } break;
      case Status::Killed:     { s = "Killed";      } break;
      case Status::Error:      { s = "Error";       } break;
      case Status::Finished:   { s = "Finished";    } break;
      default:                 { s = "Unknown";     } break;
   }
   return s;
}



// ----- Process -----

void Process::start() {
   qDebug() << "Process::start() called:";
   qDebug() << "   Program set to: " << m_program;
   qDebug() << "   Arguments:      " << m_arguments;
   connect(this, SIGNAL(error(QProcess::ProcessError)),
      this, SLOT(errorHandler(QProcess::ProcessError)));

/*
   QString info("Starting process \n");
   info += "  Program:   " + m_program   + "\n";
   info += "  Arguments: ";
   for (int i = 0; i < m_arguments.size(); ++i) {
       info += m_arguments[i] + " : ";
   } 
   info += "\n";
   QMsgBox::information(0, "IQmol", info); 
*/
 
   QProcess::start(m_program, m_arguments);
}


void Process::errorHandler(QProcess::ProcessError error) {
   QString msg("Error running:\n\n");
   msg += m_program + " " + m_arguments.join(" ") + "\n\n";

   switch (error) {
      case QProcess::FailedToStart: { 
         msg += "Process failed to start";
      } break;
      case QProcess::Crashed: { 
         msg += "Process crashed";
      } break;
      case QProcess::Timedout: { 
         msg += "Process timed out";
      } break;
      case QProcess::WriteError: {
         msg += "Could not write to process";
      } break;
      case QProcess::ReadError: {
         msg += "Could not read from process";
      } break;
      default: {
         msg += "Unknown error occured";
      } break;
   }

   QMsgBox::warning(0, "Error", msg);
   // We send out a finished signal here in case the process didn't start.
   QProcess::finished(0,QProcess::CrashExit);
}


int Process::pid() {
#ifdef Q_WS_WIN
   _PROCESS_INFORMATION* pi(QProcess::pid());
   if (pi) {
      return pi->dwProcessId;
   }else {
      return 0;
   }
#else
   return QProcess::pid();
#endif
}


void Process::kill() {
   QProcess::kill();
}


void Process::setStandardOutputFile(QString const& fileName) {
   qDebug() << "Process::setStandardOutputFile() called:";
   qDebug() << "   Output file set to " << fileName;
   QProcess::setStandardOutputFile(fileName);
   QProcess::setStandardErrorFile(fileName, QIODevice::Append);
}


void Process::setArguments(QStringList const& arguments) {
   m_arguments = arguments;
}


Status::ID Process::status() const {
   int s(state());
   if (!m_started) {
      return Status::Queued;
   }else if (s == QProcess::Starting) {
      return Status::Starting;
   }else if (s == QProcess::Running) {
      return Status::Running;
   }else {
      return m_status;
   }
}



// ********** Timed ********** //

Timed::Timed(QObject* parent, 
             QString const& program, 
             QStringList const& arguments) 
  : Process(parent, program, arguments), m_days(0), m_elapsedTime(0), 
    m_dayTimer(new QTimer()) { 

   m_dayTimer->setInterval(1000 * 60 * 60 * 24); // number of msec in a day
   connect(m_dayTimer, SIGNAL(timeout()), this, SLOT(anotherDay()));
   connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), 
      this, SLOT(finish(int, QProcess::ExitStatus)));
}


void Timed::start() {
   m_startTime.start();
   m_dayTimer->start();
   Process::start();
   m_formattedStartTime = m_startTime.toString("hh:mm:ss");
}


void Timed::finish(int, QProcess::ExitStatus) {
   m_elapsedTime = m_startTime.elapsed();
   m_dayTimer->stop();
}


QString Timed::formattedTime() {
   int time(m_elapsedTime);
   if (state() == QProcess::Running) {
      time = m_startTime.elapsed();
   }
   
   time /= 1000;
   int secs = time % 60;    
   time /= 60;
   int mins = time % 60;    
   time /= 60;
   int hours = time;

   QString t;
   if (m_days > 0) {
      t = QString::number(m_days) + " days ";
   }

   QTime qtime(hours, mins, secs);
   t += qtime.toString("hh:mm:ss");
   return t;
}



// ********** Monitored ********** //

Monitored::Monitored(QObject* parent, QString const& program, 
   QStringList const& arguments) 
   : Timed(parent, program, arguments), m_error("") {

   qDebug() << "Monitored constructor called";

   //connect(this, SIGNAL(Qt::QProcess::started()), this, SLOT(processStarted()));
   connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), 
      this, SLOT(processFinished(int, QProcess::ExitStatus)));
}



void Monitored::processStarted() {
   qDebug() << "Monitor::processStarted() called";
   m_started = true;
}


void Monitored::kill() {
   Process::kill();
   m_status = Status::Killed;
}


void Monitored::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
   qDebug() << "Monititored::submit() called" << exitCode << "  " << exitStatus;
   if (m_status == Status::Killed) {
      // do nothing
   }else if (exitStatus == QProcess::CrashExit) {
      m_status = Status::Crashed;
   }else {
      m_status = Status::Finished;
   }
   m_exitCode = exitCode;
}


void Monitored::setOutputFile(QString const& fileName) {
   m_outputFile = fileName;
   Process::setStandardOutputFile(fileName);
}



// ********** Monitor ********** //

Monitor::Monitor(QWidget* parent, WatchList* processList, int updateInterval) 
 : QMainWindow(parent), m_processList(processList)  {
   
   m_ui.setupUi(this);

   // Alter table spacings
   m_ui.processTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
   m_ui.processTable->horizontalHeader()->setStretchLastSection(true);
   m_ui.processTable->hideColumn(0);
   m_ui.processTable->setColumnWidth(1,50);
   m_ui.processTable->setColumnWidth(2,80);
   m_ui.processTable->hideColumn(3);
   m_ui.processTable->setColumnWidth(4,180);
   m_ui.processTable->setColumnWidth(5,80);
   m_ui.processTable->setColumnWidth(6,80);

   m_ui.processTable->verticalHeader()->
      setDefaultSectionSize(fontMetrics().lineSpacing() + 5);

   setStatusBar(0);
   initializeMenus();
   int defaultSortColumn(2);
   m_ui.processTable->sortItems(defaultSortColumn, Qt::AscendingOrder);

   // Set up the context menu handler
   m_ui.processTable->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_ui.processTable, SIGNAL(customContextMenuRequested(QPoint const&)),
      this, SLOT(contextMenu(QPoint const&)));

   // Set up the refresh timer
   m_timer = new QTimer(this);
   m_timer->setInterval(updateInterval);
   m_timer->start();
   connect(m_timer, SIGNAL(timeout()), this, SLOT(refresh()));

   // Add the processes
   WatchList::const_iterator iter;
   for (iter = m_processList->begin(); iter != m_processList->end(); ++iter) {
       addProcess(iter->second);
   }
}


void Monitor::initializeMenus() {
   QMenuBar* menubar(menuBar());
   QAction*  action;
   QMenu*    menu;

   menubar->clear();

   // File
   menu = menubar->addMenu(tr("File"));

   // File -> Close
   action = menu->addAction(tr("Close"));
   connect(action, SIGNAL(triggered()), this, SLOT(menuClose()));
   action->setShortcut(QKeySequence::Close);

   // File -> Refresh
   action = menu->addAction(tr("Refresh"));
   connect(action, SIGNAL(triggered()), this, SLOT(refresh()));
   action->setShortcut(Qt::CTRL + Qt::Key_R);
}


void Monitor::contextMenu(QPoint const& pos) {
   QTableWidgetItem* item = m_ui.processTable->itemAt(pos);

   if (item) {
      QMenu *menu = new QMenu(this);

      QAction* kill = menu->addAction(tr("Kill Job"), 
          this, SLOT(stopProcess()));

      QAction* view = menu->addAction(tr("View Output"), 
          this, SLOT(viewOutput()));

/*
      QAction* analyse = menu->addAction(tr("Analyse Output"), 
          this, SLOT(analyseOutput()));
*/

      menu->addSeparator();

      QAction* remove = menu->addAction(tr("Remove From List"), 
          this, SLOT(removeProcess()));

      QString key(m_ui.processTable->item(item->row(),0)->text());
      Monitored* process(m_processList->find(key)->second); 

      if (process) {
         if (process->status() != Status::Running) kill->setEnabled(false);
         if (process->status() == Status::Queued ) view->setEnabled(false); 
         if (process->status() == Status::Starting ||
             process->status() == Status::Running)  remove->setEnabled(false); 
         //if (process->status() != Status::Finished) analyse->setEnabled(false); 
               
      }
      menu->exec(m_ui.processTable->mapToGlobal(pos));
   }
}



void Monitor::menuClose() {
   m_timer->stop();
   close();
   //deleteLater();
}




void Monitor::removeProcess() {
   WatchList::iterator iter(findSelectedProcess());
   if (iter != m_processList->end()) {
      if (iter->second->state() == QProcess::NotRunning) {
         qDebug() << "Need to remove process from list" << iter->first;
         m_timer->stop();
         QList<QTableWidgetItem*> items(m_ui.processTable->selectedItems());
         m_ui.processTable->removeRow(items[0]->row());      
         qDebug() << "Sending processRemoved() signal" << iter->second;
         processRemoved(iter->second);
         m_processList->erase(iter);         
         refresh();
         m_timer->start();
      }else {
         QString msg("Unable to remove active process from the list.");
         QMsgBox::warning(this, "Error", msg);
      }
   }
}



WatchList::iterator Monitor::findSelectedProcess() {
   QList<QTableWidgetItem*> items(m_ui.processTable->selectedItems());
   if (items.isEmpty()) {
      return m_processList->end();
   }else {
      int row(items[0]->row());
      QString key(m_ui.processTable->item(row,0)->text());
      return m_processList->find(key);
   }
}



void Monitor::stopProcess() {
   WatchList::iterator iter(findSelectedProcess());
   if (iter != m_processList->end()) {
      if (iter->second->state() != QProcess::NotRunning) {
         iter->second->kill();
         refresh();
      }
   }
}



//! Displays the output file for the currently selected process.
void Monitor::viewOutput() {
   QList<QTableWidgetItem*> items = m_ui.processTable->selectedItems();
   if (!items.isEmpty()) {
      // Selection mode is set to single, so only one row at a time should be
      // able to be selected.
      displayOutputFile(items[0]->row());
   }
}



//! Opens Avogadro to allow output analysis
void Monitor::analyseOutput() {
   WatchList::iterator iter(findSelectedProcess());
   if (iter != m_processList->end()) {
      analyseThisProcess(iter->second);
   }
}



void Monitor::addProcess(Monitored* process) {
   QTableWidget* table(m_ui.processTable);
   
   int row = table->rowCount();
   table->insertRow(row);
   for (int i = 0; i < table->columnCount(); ++i) {
       table->setItem(row, i, new QTableWidgetItem());
   }
   updateRow(row, process);
}


void Monitor::refresh() {
   QTableWidget* table(m_ui.processTable);
   QTableWidgetItem* item;
   WatchList::iterator iter;

   table->setSortingEnabled(false);

   for (int row = 0; row < table->rowCount(); ++row) {
       item = table->item(row, 0);
       iter = m_processList->find(item->text());
       if (iter != m_processList->end() ) {
          updateRow(row, iter->second);
       }else {
          //qDebug() << "!!! Could not find process" << item->text();
       }
   }
   
   //table->setSortingEnabled(true);
   m_ui.processTable->hideColumn(0);
   m_ui.processTable->hideColumn(3);
}


void Monitor::updateRow(int row, Monitored* process) {
   QTableWidget* table(m_ui.processTable);

   if (process) {
      QString s = QString::number(size_t(process));
      table->item(row,0)->setText(s);

      s = QString::number(process->pid());
      table->item(row,1)->setText(s);

      s = process->formattedStartTime();
      table->item(row,2)->setText(s);

      s = process->programName();
      table->item(row,3)->setText(s);

      s = process->arguments();
      table->item(row,4)->setText(s);

      s = process->formattedTime();
      table->item(row,5)->setText(s);

      s = ToString(process->status());
      table->item(row,6)->setText(s);
      if (s.contains("Error")) {
         table->item(row,6)->setToolTip(process->error());
      }else {
         table->item(row,6)->setToolTip(ToString(process->status()));
      }
   }
}


void Monitor::on_processTable_cellDoubleClicked(int row, int) { 
   displayOutputFile(row); 
}


void Monitor::displayOutputFile(int row) {
   QTableWidgetItem* item = m_ui.processTable->item(row, 0);
   WatchList::iterator iter = m_processList->find(item->text());

   if (iter != m_processList->end() ) {
      QString outputFile(iter->second->outputFile());
      if (!outputFile.isEmpty()) {
         FileDisplay* fileDisplay = new FileDisplay(this, outputFile);
         fileDisplay->show();
      }else {
         qDebug() << "empty file handle in displayOutputFile ";
      }
   }else {
      qDebug() << "!!! Could not find process in cellDoubleClicked row = "  << row;
   }
}



// ----- Queue -----

void Queue::submit(Process* process) {
   qDebug() << "Queue::submit() called";
   connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), 
      this, SLOT(processFinished(int, QProcess::ExitStatus)));
   m_processQueue.push(process);
   runQueue();
}


void Queue::processFinished(int, QProcess::ExitStatus) {
   qDebug() << "Queue::processFinished() called:";
   qDebug() << "   Running processes: " << m_nProcessesRunning << "->" <<
   --m_nProcessesRunning;
   runQueue();
}


void Queue::remove(Process* process) {
qDebug() << "Queue::remove() called:" << process;
qDebug() << "   Current queue size = " << m_processQueue.size();
   Process* p;
   for (unsigned int i = 0; i < m_processQueue.size(); ++i) {
       p = m_processQueue.front();
       m_processQueue.pop();
       if (p != process) m_processQueue.push(p);
   }
qDebug() << "   New queue size     = " << m_processQueue.size();
qDebug() << "#end";
}


void Queue::runQueue() {
qDebug() << "Queue::runQueue() called:";
qDebug() << "   Queue size         = " << m_processQueue.size();
qDebug() << "   Processes running  = " << m_nProcessesRunning;
qDebug() << "   Maximum processes  = " << m_maxProcesses;
   while (!m_processQueue.empty() && m_nProcessesRunning < m_maxProcesses) {
      m_processQueue.front()->start();
      Monitored* p = qobject_cast<Monitored*>(m_processQueue.front());
      p->processStarted();
      qDebug() << "      Starting = " <<  m_processQueue.front();
      m_processQueue.pop();
      ++m_nProcessesRunning;
   }
qDebug() << "      Processes running  = " << m_nProcessesRunning;
qDebug() << "#end";
}


} } // end namespaces Qui::Process
