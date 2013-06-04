#ifndef IQMOL_MAINWINDOW_H
#define IQMOL_MAINWINDOW_H
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

#include "Viewer.h"
#include "ToolBar.h"
#include "HelpBrowser.h"
#include "AboutDialog.h"
#include "Preferences.h"
#include "ProcessMonitor.h"
#include "ViewerModel.h"
#include "ViewerModelView.h"
#include "LogMessageDialog.h"
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QMainWindow>
#include <QUndoStack>
#include <QUndoView>
#include <QTreeView>
#include <QSplitter>
#include <QProgressBar>
#include <QList>


namespace Qui {
   class InputDialog;
}

class QSplitter;
class QResizeEvent;
class QUndoCommand;
class QCloseEvent;
class QAction;
class QMenu;

namespace IQmol {

   class ModelView;
   class ShaderDialog;

   /// MainWindow is the top level window in IQmol which contains the
   /// ToolBar, Viewer, ViewerModelView and History.
   class MainWindow : public QMainWindow {

      Q_OBJECT

      public: 
         MainWindow(QWidget* parent = 0);
         ~MainWindow();

      Q_SIGNALS:
         void recordingActive(bool);

      public Q_SLOTS:
         void openFile();
         void openRecentFile();
         void openFile(QString const& fileName);
         void addCommand(QUndoCommand* cmd) { m_undoStack.push(cmd); }

      private Q_SLOTS:
         void splitterMoved(int, int);
         void showHelp() { m_helpBrowser.show(); }
         void showAbout() { m_aboutDialog.show(); }
         void showPreferences() { m_preferencesBrowser.show(); }
         void showLogMessages();
         void closeEvent(QCloseEvent*);
         void newViewer();
         void quit() { QApplication::quit(); }
         void clearRecentFilesMenu();
         void fullScreen();
         void setForceField();
         void setPartialChargeType();
         void setLabel();
         void setShader();
         void reindexAtoms();
         void windowModified() { setWindowModified(true); }
         void setRecordingActive(bool);
         void toggleRecordingActive();
         void openQChemUI();
         void editServers();
         void setVaultPassword();
         void recordingCancelled() { setRecordingActive(false); }
/////////////////////////
         void parseFile();
         void fileParsed();
         void configureAppearance();

         void showProcessMonitor() { 
            ProcessMonitor::instance().show(); 
            ProcessMonitor::instance().raise();
         }

      private:
         void createMenus();
         void createLayout();
         void createConnections();
         void updateRecentFilesMenu();
         void resizeEvent(QResizeEvent*);

         ToolBar         m_toolBar;
         AboutDialog     m_aboutDialog;
         HelpBrowser     m_helpBrowser;
         ViewerModel     m_viewerModel;
         ViewerModelView m_viewerView;
         QUndoStack      m_undoStack;
         QUndoView       m_undoStackView;
         QProgressBar    m_progressBar;
         QLabel          m_status;
         Viewer          m_viewer;

         QItemSelectionModel  m_viewerSelectionModel;
         LogMessageDialog     m_logMessageDialog;
         Preferences::Browser m_preferencesBrowser;

         QMenu*   m_recentFilesMenu;
         QAction* m_fullScreenAction;
         QAction* m_qchemSetupAction;
         QAction* m_recordAnimationAction;
         QList<QAction*> m_labelActions;

         QSplitter* m_sideSplitter;
         Qui::InputDialog* m_quiInputDialog;
         ShaderDialog* m_shaderDialog;
   };

} // end namespace IQmol


#endif
