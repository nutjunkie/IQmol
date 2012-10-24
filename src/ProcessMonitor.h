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

   //! The Process::Monitor handles the submission of external jobs, such as
   //! Q-Chem jobs.  
   class ProcessMonitor : public QMainWindow {

      Q_OBJECT

      public:
         ProcessMonitor(QWidget* parent);
         ~ProcessMonitor();

      Q_SIGNALS:
		 /// This signals the job is valid has been submitted, and the input
		 /// generator can be closed.
         void jobAccepted();

         /// This signal is emitted only when a job has finished successfully.
         void resultsAvailable(JobInfo*);

		 /// These messages inform the user that progress is being made.
		 /// Especially useful if the connection to a server is slow.
         void postStatusMessage(QString const&);

      public Q_SLOTS:
         // scope required as we call this from the Qui namespace
         void submitJob(IQmol::JobInfo*);
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
         void loadProcessList();
		 // Clears all the processes from the monitor and their servers.  If
		 // finishedOnly then only the processes that have status Finished, 
         // Killed or Error hare removed.
         void clearProcessList(bool const finishedOnly, bool const prompt);
         void copyResults(Process*);
         void queryProcess(Process*);
         void addToTable(Process*);
         void saveProcessList();
         void updateRow(int const row, QStringList const& items);

         // Performs a reverse lookup on m_processMap to find either the process that
         // corresponds to the item if non-zero or the selected process otherwise.
         Process* getSelectedProcess(QTableWidgetItem* item = 0);
         void initializeMenus();

         QTimer m_updateTimer;
         Ui::ProcessMonitor m_ui;
         QMap<Process*, QTableWidgetItem*> m_processMap;

         /// This holds the Process that is in the process of being submitted.
         Process* m_pendingProcess;
   };


} // end namespace IQmol

#endif
