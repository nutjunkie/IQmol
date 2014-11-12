#ifndef IQMOL_PROCESS_JOBMONITOR_H
#define IQMOL_PROCESS_JOBMONITOR_H
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

#include "ui_JobMonitor.h"
#include <QTimer>


class QCloseEvent;
class QShowEvent;

namespace IQmol {
namespace Process2 {

   class Job;
   class QChemJobInfo;

   /// The JobMonitor handles the submission and monitoring of external 
   /// calculations such as Q-Chem jobs.  Note that the JobMonitor takes 
   /// ownership of the Job objects.

   class JobMonitor : public QMainWindow {

      Q_OBJECT

      public:
         static JobMonitor& instance();

      public Q_SLOTS:
         // Namespace qualification is required as we call this from QUI
         void submitJob(IQmol::Process2::QChemJobInfo&);
         void jobSubmissionSuccessful(Job*);
         void jobSubmissionFailed(Job*);

      Q_SIGNALS:
         /// This signal is emitted only when a job has finished successfully.
         void resultsAvailable(Job&);
         void jobAccepted();

         void postUpdateMessage(QString const&);

      protected:
         void closeEvent(QCloseEvent* event);
         void showEvent(QShowEvent* event);

      private Q_SLOTS:
         void on_clearListButton_clicked(bool);


		 /// Used to remove all jobs listed in the monitor.  This is triggered
		 /// by a MainWindow menu action and may be useful there are rogue
		 /// processes on the list which are causing problems and need to be
		 /// removed.
         void clearAllJobs();

		 /// This is really a pseudo-update to the entries in the monitor, it
         /// simply grabs the values that are cached in the Job objects.  The
         /// individual Servers decide how to handle the real updates, which 
         /// allows network requests to be minimized.  
         void updateTable();

         void reconnectServers();
         void jobUpdated();
         void jobFinished();

         // Context menu actions
         void contextMenu(QPoint const& position);
         void killJob();
         void removeJob();
         void queryJob();
         void copyResults();

      private:
         static QMap<Job*, QTableWidgetItem*> s_jobMap;
         static QList<Job*> s_deletedJobs;

         void saveJobListToPreferences() const;
         void loadJobListFromPreferences();

         void addToTable(Job*);
         void reloadJob(Job* job);
         void initializeMenus();

         QString getWorkingDirectory(QString const& message, QString const& suggestion);

         Job* getSelectedJob(QTableWidgetItem* item = 0);

		 /// Clears the jobs from the monitor and their servers.  If
		 /// finishedOnly then only the processes that have status 
         void clearJobTable(bool const finishedOnly);

         // Singleton stuff
         JobMonitor() { }
         JobMonitor(QWidget* parent);
         explicit JobMonitor(JobMonitor const&);
         ~JobMonitor() { }
         static JobMonitor* s_instance;
         static void destroy();

         Ui::JobMonitor m_ui;
         QTimer m_updateTimer;

/*

      private Q_SLOTS:
         void on_processTable_cellDoubleClicked(int row, int col);

      private:
		 // Clears all the processes from the monitor and their servers.  If
		 // finishedOnly then only the processes that have status Finished, 
         // Killed or Error are removed.

         void copyResults(Process*);
         void queryProcess(Process*);

         void updateRow(int const row, QStringList const& items);


*/
   };

} } // end namespace IQmol::Process

#endif
