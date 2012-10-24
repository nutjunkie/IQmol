#ifndef QUI_PROCESS_H
#define QUI_PROCESS_H

/*!
 *  \file Process.h 
 *
 *  \brief Class definitions associated with process control in the Qui.
 *
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "ui_ProcessMonitor.h"

#include <QTime>
#include <QTimer>
#include <QString>
#include <QObject>
#include <QProcess>
#include <QFileInfo>
#include <QStringList>
#include <QMainWindow>
#include <map>
#include <vector>
#include <queue>



namespace Qui {
namespace Process {

// A WatchList contains a list of Monitored processes with the keys being the
// pointer value cast as a string.  This is done to make interacting with the
// process table easier.
class Monitored;
typedef std::map<QString, Monitored*> WatchList;

struct Status {
   enum ID { NotRunning = 0, Starting, Running, Queued, Crashed, Killed, 
             Error, Finished, Unknown };
};

QString ToString(Status::ID const& status);


//! \class Process is a base class for the other process types, Timed,
//! Monitored etc.  It caches the program name and arguments for use in
//! a Queue and also over-rides some of the QProcess functions with virtual
//! forms to allow polymorphism.
class Process : public QProcess {

   Q_OBJECT

   public:
      Process(QObject* parent, 
              QString const& program, 
              QStringList const& arguments) 
       : QProcess(parent),  m_program(program), m_arguments(arguments), 
         m_status(Status::Unknown), m_started(false) { }

      virtual ~Process() { }
      virtual void start();
      virtual void kill();
      virtual int  pid();
      virtual void setStandardOutputFile(QString const& fileName);
      virtual void setArguments(QStringList const& arguments);

      Status::ID status() const;

   protected:
      QString m_program;
      QStringList m_arguments;
      Status::ID m_status;
      bool m_started;

   private Q_SLOTS:
      void errorHandler(QProcess::ProcessError);
};



//! \class Timed subclasses Process by allowing the process to keep track of
//! how long it has been running for.
class Timed : public Process {

   Q_OBJECT

   public:
      Timed(QObject* parent, 
            QString const& program, 
            QStringList const& arguments);
      virtual ~Timed() { }
      virtual void start();
      QString formattedTime();
      QString formattedStartTime() { return m_formattedStartTime; }

   private Q_SLOTS:
      void anotherDay() { ++m_days; }
      void finish(int, QProcess::ExitStatus);

   private:
      int     m_days;
      int     m_elapsedTime;
      QTime   m_startTime;
      QTimer* m_dayTimer;
      QString m_formattedStartTime;
};



//! \class Monitored is designed to operate with the Monitor and so has the
//! necessary data members for all the fields in the table.
class Monitored : public Timed {

   Q_OBJECT

   friend class Queue;

   public:
      Monitored(QObject* parent, 
                QString const& program, 
                QStringList const& arguments = QStringList());
      virtual ~Monitored() { }

      void kill();

      QString error()       const { return m_error; }
      QString programName() const { return m_program; }
      QString arguments()   const { return m_arguments.join("  "); }
      QString outputFile()  const { return m_outputFile;}
      QString inputFile()   const { return m_inputFile;}
      QString auxFile()     const { return m_auxFile;}

      void setOutputFile(QString const& fileName);
      void setAuxFile(QString const& fileName)   { m_auxFile   = fileName; }
      void setInputFile(QString const& fileName) { m_inputFile = fileName; }

   protected:
      QString m_error;
      QString m_outputFile;
      QString m_inputFile;
      QString m_auxFile;

   private Q_SLOTS:
      void processFinished(int, QProcess::ExitStatus);
      void processStarted();

   private:
      int  m_exitCode;
};



//! \class Monitor is a window that displays a list of submitted jobs and
//! allows the user to kill, or view the output.
//! \b Note: 
//!  - The Monitor does not own the processes, rather it only 'observes' them,
//!    so no deletion is done.
//!  - The monitor doesn't perform the requested actions, rather it send out
//!    the appropriate signals which must be handled upstream (by the
//!    InputDialog class).
class Monitor: public QMainWindow {

   Q_OBJECT

   public:
      Monitor(QWidget* parent, WatchList* processes, int updateInterval = 1000);  // 1000 msec
      ~Monitor() { }  

      void addProcess(Monitored* process);
      void startTimer() {m_timer->start(); }

   Q_SIGNALS:
      void processRemoved(Process* process);
      void viewThisProcess(Monitored* process);
      void analyseThisProcess(Monitored* process);
      void removeThisProcess(Monitored* process);
      void stopThisProcess(Monitored* process);

   private Q_SLOTS:
      void menuClose();
      void refresh();
      void viewOutput();
      void removeProcess();
      void stopProcess();
      void analyseOutput();

      void on_closeButton_clicked(bool)   { menuClose(); }
      void on_processTable_cellDoubleClicked(int row, int col);
      void contextMenu(QPoint const& position);

   private:
      Ui::ProcessMonitor m_ui;
      QTimer* m_timer;
      WatchList* m_processList;

      void updateRow(int row, Monitored* process);
      void displayOutputFile(int row);
      void initializeMenus();
      WatchList::iterator findSelectedProcess();
};



//! \class Queue holds a list of processes to run sequentially.
class Queue : public QObject {

   Q_OBJECT

   public:
      Queue(QObject* parent, int maxProcesses = 1) 
       : QObject(parent), m_nProcessesRunning(0), m_maxProcesses(maxProcesses) { }
      ~Queue() { }
      void submit(Process* process);

   public Q_SLOTS:
      void remove(Process* process);

   private Q_SLOTS:
      void processFinished(int, QProcess::ExitStatus);

   private:
      std::queue<Process*> m_processQueue; 
      int m_nProcessesRunning;
      int m_maxProcesses;

      void runQueue();
};


} } // end namespaces Qui::Process

#endif
