#ifndef IQMOL_PROCESSMONITOR_H
#define IQMOL_PROCESSMONITOR_H
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

#include "ui_ProcessMonitor.h"
#include <QTimer>
#include <QList>


class QCloseEvent;
class QShowEvent;

namespace IQmol {

   class JobInfo;
   class Process;

   /// The ProcessMonitor handles the submission of calculations such as Q-Chem
   /// jobs.  Note that the ProcessMonitor takes ownership of the Processes it
   /// creates and deletes them on destruction.
   class ProcessMonitor : public QMainWindow {

      Q_OBJECT

      public:
         static ProcessMonitor& instance();

      Q_SIGNALS:
		 /// This signals the most recent job is valid has been submitted.  Its
		 /// primary use is to let the input generator dialog know that it can
		 /// close.
         void jobAccepted();

         /// This signal is emitted only when a job has finished successfully.
         void resultsAvailable(JobInfo*);

		 /// These messages inform the user that progress is being made.
		 /// Especially useful if the connection to a server is slow.
         void postStatusMessage(QString const&);

      public Q_SLOTS:
		 /// This is the main entry point for submitting a calculation.
		 /// Because the submission requires communication to a (potentially
         /// remote) Server, the process is carried out in a thread chain.
         /// The scope required as we may be calling this from another namespace.
         void submitJob(IQmol::JobInfo*);

		 /// Used to remove all jobs listed in the monitor.  This is triggered
		 /// by a MainWindow menu action and may be useful there are rogue
		 /// processes on the list which are causing problems and need to be
		 /// removed.
         void clearProcessList() { clearProcessList(false, true); }


      protected:
         void closeEvent(QCloseEvent* event);
         void showEvent(QShowEvent* event);


      private Q_SLOTS:
         void on_clearListButton_clicked(bool);
         void on_processTable_cellDoubleClicked(int row, int col);

         // These form a thread chain
         void submitJob1();
         void submitJob2();
         void jobSubmitted();

		 /// This is really a pseudo-update to the entries in the monitor, it
         /// simply grabs the values that are cached in the Process objects. 
         /// The individual Servers decide how to handle the real updates, which 
         /// allows network requests to be minimized.  
		 void updateTable();

         void processUpdated();
         void processFinished();
         void reconnectServers();

         // Context menu actions
         void contextMenu(QPoint const& position);
         void killProcess();
         void processKilled();
         void removeProcess();
         void queryProcess();
         void queryFinished();
         void copyResults();
         void openOutput();
         void resultsCopied();


      private:
         /// Loads the list of processes from the users preferences file.
         void loadProcessList();
         void saveProcessList();

		 // Clears all the processes from the monitor and their servers.  If
		 // finishedOnly then only the processes that have status Finished, 
         // Killed or Error are removed.
         void clearProcessList(bool const finishedOnly, bool const prompt);

         void copyResults(Process*);
         void queryProcess(Process*);
         void addToTable(Process*);
         void updateRow(int const row, QStringList const& items);

         // Performs a reverse lookup on s_processMap to find either the process that
         // corresponds to the item if non-zero or the selected process otherwise.
         Process* getSelectedProcess(QTableWidgetItem* item = 0);
         void initializeMenus();

         QTimer m_updateTimer;
         Ui::ProcessMonitor m_ui;
         static QMap<Process*, QTableWidgetItem*> s_processMap;

		 /// This holds the Process that is in the process of being submitted
		 /// and is required for persistence across the submission thread chain.
         Process* m_pendingProcess;

         // Singleton stuff
         ProcessMonitor() { }
         ProcessMonitor(QWidget* parent);
         explicit ProcessMonitor(ProcessMonitor const&);
         ~ProcessMonitor() { }
         static ProcessMonitor* s_instance;
         static void destroy();
   };


} // end namespace IQmol

#endif
