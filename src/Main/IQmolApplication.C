/*******************************************************************************
       
  Copyright (C) 2011-2015 Andrew Gilbert
           
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
#include "JobMonitor.h"
#include "ServerRegistry.h"
#include "QMsgBox.h"
#include "QsLog.h"
#include <QDir>
#include <QString>
#include <QLibrary>
#include <QMessageBox>
#include <QFileOpenEvent>
#include <QSplashScreen>

#include <QThread>
#include <QThreadPool>

#include <exception>


namespace IQmol {

IQmolApplication::IQmolApplication(int &argc, char **argv )
  : QApplication(argc, argv), m_splashScreen(0), 
    m_unhandledException(QMessageBox::Critical, "IQmol", "IQmol encountered an exception.\n"
    "See log file for details.", QMessageBox::Abort)
    
{
   setOrganizationDomain("iqmol.org");
   setApplicationName("IQmol");
   // Can't log anything yet as the logger hasn't been initialized
}


IQmolApplication::~IQmolApplication()
{
// This seems to cause a crash on exit under windows
//   if (m_splashScreen) delete m_splashScreen;
}


void IQmolApplication::showSplash()
{  
    QPixmap pixmap(":resources/splash.png");
    m_splashScreen = new QSplashScreen(pixmap);
    m_splashScreen->show();
    processEvents();
}


void IQmolApplication::hideSplash()
{  
   if (m_splashScreen) m_splashScreen->close();
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
   QString path;

#ifdef Q_OS_MAC
   // Assumed directory structure: IQmol.app/Contents/MacOS/IQmol
   dir.cdUp();  
   path = dir.absolutePath();
   QApplication::addLibraryPath(path + "/Frameworks");
   QApplication::addLibraryPath(path + "/PlugIns");
#endif

#ifdef Q_OS_WIN32
   // Assumed directory structure: IQmol/bin/IQmol
   dir.cdUp();  
   path = dir.absolutePath();
   QApplication::addLibraryPath(path + "/lib");
   QApplication::addLibraryPath(path + "/lib/plugins");
#endif

#ifdef Q_OS_LINUX
   // Assumed directory structure: IQmol/IQmol
   // This is so that when the executable is created it can be 
   // run from the main IQmol directory 
   path = dir.absolutePath();
   QApplication::addLibraryPath(path + "/lib");
   QApplication::addLibraryPath(path + "/lib/plugins");
#endif
   

   // This is not used any more as we ship with a static
   // version of OpenBabel
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
#if defined(Q_OS_LINUX)
      // Overide the above for the deb package installation.
      env ="/usr/share/iqmol/openbabel";
#else
      env = path + "/share/openbabel";
#endif
      qputenv("BABEL_DATADIR", env.toLatin1());
      QLOG_INFO() << "Setting BABEL_DATADIR = " << env;
   }else {
      QLOG_INFO() << "BABEL_DATADIR already set: " << env;
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
      connect(mw, SIGNAL(quit()), this, SLOT(quitRequest()));
      QApplication::setActiveWindow(mw);
   }

   QFileInfo info(file);
   if (info.exists()) mw->open(file);
   hideSplash();
   mw->show();
   mw->raise();

   // Now we can load jobs from the preferences, if we try to do it 
   // before now, the dialog appears under the splash screen
   Process::JobMonitor::instance().loadJobListFromPreferences();

   static bool connected(false);
   if (!connected) {
      connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quitRequest()));
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


void IQmolApplication::quitRequest()
{
   Process::ServerRegistry::instance().closeAllConnections();
   QApplication::quit();  
}


void IQmolApplication::exception()
{
   m_unhandledException.exec();   
}


} // end namespace IQmol
