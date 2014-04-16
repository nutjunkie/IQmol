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

#include "IQmolApplication.h"
#include "MainWindow.h"
#include "QMsgBox.h"
#include "QsLog.h"
#include <QDir>
#include <QString>
#include <QLibrary>
#include <QMessageBox>
#include <QFileOpenEvent>

#include <QThread>
#include <QThreadPool>



namespace IQmol {

IQmolApplication::IQmolApplication(int &argc, char **argv )
  : QApplication(argc, argv)
{
   setOrganizationDomain("iqmol.org");
   setApplicationName("IQmol");
   // Can't log anything yet as the logger hasn't been initialized
}


void IQmolApplication::queueOpenFiles(QStringList const& files)
{
   QStringList::const_iterator iter;
   for (iter = files.begin(); iter != files.end(); ++iter) {
       FileOpenEvent* event = new FileOpenEvent(*iter);
       QApplication::postEvent(this, event, Qt::LowEventPriority);
   }
}


void IQmolApplication::initOpenBabel()
{
   QDir dir(QApplication::applicationDirPath());
   dir.cdUp();  
   QString path(dir.absolutePath());

#ifdef Q_WS_MAC
   // Assumed directory structure: IQmol.app/Contents/MacOS/IQmol
   QApplication::addLibraryPath(path + "/Frameworks");
   QApplication::addLibraryPath(path + "/PlugIns");
#else
   // Assumed directory structure: IQmol-xx/bin/IQmol
   QApplication::addLibraryPath(path + "/lib");
   QApplication::addLibraryPath(path + "/lib/plugins");
#endif

#ifdef Q_OS_LINUX
   return;
#endif

   QString env(qgetenv("BABEL_LIBDIR"));
   if (env.isEmpty()) {
      env = path + "/lib/openbabel";
      qputenv("BABEL_LIBDIR", env.toLatin1());
      QLOG_INFO() << "Setting BABEL_LIBDIR = " << env;
   }else {
      QLOG_INFO() << "BABEL_LIBDIR already set: " << env;
   }

   env = qgetenv("BABEL_DATADIR");
   if (env.isEmpty()) {
      env = path + "/share/openbabel";
      qputenv("BABEL_DATADIR", env.toLatin1());
      QLOG_INFO() << "Setting BABEL_DATADIR = " << env;
   }else {
      QLOG_INFO() << "BABEL_DATADIR already set: " << env;
   }

return;
#ifdef Q_WS_WIN
   QLibrary openBabel("libopenbabel.dll");
#else
   QLibrary openBabel("openbabel");
#endif

   if (!openBabel.load()) {
      QString msg("Could not load library ");
      msg += openBabel.fileName();

      QLOG_ERROR() << msg << " " << openBabel.errorString();
      QLOG_ERROR() << "Library Paths:";
      QLOG_ERROR() << libraryPaths();

      msg += "\n\nPlease ensure the OpenBabel libraries have been installed correctly";
      QMsgBox::critical(0, "IQmol", msg);
      QApplication::quit();
      return;
   }
}


void IQmolApplication::open(QString const& file)
{
   // This is the first thing that is called once the event loop has started,
   // even if there is no actual file to open (empty file name).  This is an
   // ideal time to check if OpenBabel is around.  
   initOpenBabel();

   MainWindow* mw;
   QWidget* window(QApplication::activeWindow());
   if ( !(mw = qobject_cast<MainWindow*>(window)) ) {
      mw = new MainWindow();
      QApplication::setActiveWindow(mw);
   }

   QFileInfo info(file);
   if (info.exists()) mw->open(file);
   mw->show();
   mw->raise();

   static bool connected(false);
   if (!connected) {
//      connect(this, SIGNAL(lastWindowClosed()), this, SLOT(maybeQuit()));
      connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
      connected = true;
   }
   QLOG_INFO() << "Number of threads:" << QThread::idealThreadCount();
   QLOG_INFO() << "Active    threads:" << QThreadPool::globalInstance()->activeThreadCount();
}


bool IQmolApplication::event(QEvent* event)
{
   bool accepted(false);

   switch (event->type()) {
      case QEvent::FileOpen: {
         QString file(static_cast<QFileOpenEvent*>(event)->file());
         queueOpenFiles(QStringList(file));
         accepted = true;
         } break;

//      case QEvent::Close: {
//         disconnect(this, SIGNAL(lastWindowClosed()), this, SLOT(maybeQuit()));
//         accepted = QApplication::event(event);
//         } break;

      case QEvent::User: {
         QString file(static_cast<FileOpenEvent*>(event)->file());
         open(file);
         accepted = true;
         } break;

      default:
         accepted = QApplication::event(event);
         break;
   }

   return accepted;
}


void IQmolApplication::maybeQuit()
{
   disconnect(this, SIGNAL(lastWindowClosed()), this, SLOT(maybeQuit()));

   QPixmap pixmap;
   pixmap.load(":/imageInformation");
   QMessageBox messageBox(QMessageBox::NoIcon, "IQmol", "No viewer windows remain");
   QPushButton* quitButton = messageBox.addButton("Quit", QMessageBox::AcceptRole);
   QPushButton* newButton  = messageBox.addButton("New", QMessageBox::RejectRole);
   QPushButton* openButton = messageBox.addButton("Open", QMessageBox::RejectRole);
   messageBox.setIconPixmap(pixmap);
   messageBox.exec();

   if (messageBox.clickedButton() == quitButton) {
      QApplication::quit();
   }else if (messageBox.clickedButton() == newButton) {
      MainWindow* mw(new MainWindow());
      mw->show();
   }else if (messageBox.clickedButton() == openButton) {
      MainWindow* mw(new MainWindow());
      mw->show();
      mw->openFile();
   }

   connect(this, SIGNAL(lastWindowClosed()), this, SLOT(maybeQuit()));
}


} // end namespace IQmol
