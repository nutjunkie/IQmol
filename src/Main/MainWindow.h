#ifndef IQMOL_MAINWINDOW_H
#define IQMOL_MAINWINDOW_H
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

#include "Viewer.h"
#include "ToolBar.h"
#include "HelpBrowser.h"
#include "AboutDialog.h"
#include "PreferencesBrowser.h"
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

         void initViewer() { m_viewer.initShaders(); }

      Q_SIGNALS:
         void recordingActive(bool);

      public Q_SLOTS:
         void openDir();
         void openFile();
         void openRecentFile();
         void open(QString const& filePath) { m_viewerModel.open(filePath); }
         void fileOpened(QString const& filePath);
         void addCommand(QUndoCommand* cmd) { m_undoStack.push(cmd); }

      private Q_SLOTS:
         void splitterMoved(int, int);
         void showHelp() { m_helpBrowser.show(); }
         void showAbout() { m_aboutDialog.show(); }
         void showPreferences() { m_preferencesBrowser.show(); }
         void showLogMessages();
         void showQChemUI();
         void showProcessMonitor();
         void testInternetConnection();

         void newViewer();
         void fullScreen();
         void closeEvent(QCloseEvent*);
         void quit() { QApplication::quit(); }
         void windowModified() { setWindowModified(true); }

         void setForceField();
         void setLabel();
         void reindexAtoms();

         void setRecord(bool);
         void toggleRecordingActive();
         void recordingCanceled() { setRecord(false); }

         void editServers();
         void editNewServers();
         void configureAppearance();
         void clearRecentFilesMenu();


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

         QItemSelectionModel   m_viewerSelectionModel;
         LogMessageDialog      m_logMessageDialog;
         Preferences::Browser  m_preferencesBrowser;

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
