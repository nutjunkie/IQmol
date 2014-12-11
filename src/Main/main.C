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

/*!
 *  \mainpage IQmol
 *
 *  \section Introduction
 *   This is IQmol, a molecular visualization package.
 *
 *  \author Andrew Gilbert
 *  \date   November 2011
 */

#include "IQmolApplication.h"
#include "Preferences.h"
#include "IQmol.h"
#include <QStringList>
#include <QDir>
#include "QsLog.h"
#include "QsLogDest.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
    IQmol::IQmolApplication iqmol(argc, argv);
    Q_INIT_RESOURCE(IQmol);

    iqmol.showSplash();

    // Setup logging;
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    QString const logFile(IQmol::Preferences::LogFilePath());

    // Weird scoping going on here
    QsLogging::DestinationPtr fileDestination;
    QsLogging::DestinationPtr debugDestination;

    if (IQmol::Preferences::LoggingEnabled()) {
       fileDestination  = QsLogging::DestinationFactory::MakeFileDestination(logFile);
       debugDestination = QsLogging::DestinationFactory::MakeDebugOutputDestination();

       logger.addDestination(fileDestination.get());
       logger.addDestination(debugDestination.get());
#ifdef Q_OS_WIN32
       if (IQmol::Preferences::LogFileHidden()) {
          int fileHidden(0x2);  // Where is FILE_ATTRIBUTE_HIDDEN defined?
          WCHAR* fnam = (WCHAR*)logFile.constData();
          SetFileAttributes(fnam, fileHidden); 
       }
#endif
    }

    QLOG_INFO() << "---------- Session Started ----------";
    QLOG_INFO() << "IQmol Version: " << IQMOL_VERSION;

#ifdef Q_OS_LINUX
    QString env(qgetenv("QT_PLUGIN_PATH"));
    if (env.isEmpty()) {
       QDir dir(QApplication::applicationDirPath());
       dir.cdUp();  
       QString path(dir.absolutePath());
       env = path + "/lib/plugins";
       qputenv("QT_PLUGIN_PATH", env.toLatin1());
       QLOG_INFO() << "Setting QT_PLUGIN_PATH = " << env;
    }else {
       QLOG_INFO() << "QT_PLUGIN path already set: " << env;
    }
#endif

    QStringList args(QCoreApplication::arguments());
    args.removeFirst();
    // This ensures we always have something to open
    if (args.isEmpty()) args.push_back("");
    iqmol.queueOpenFiles(args);

    int ret(iqmol.exec());
    QLOG_INFO() << "Return code:" << ret;
    QLOG_INFO() << "----------- Session Ended -----------";
    return ret;
}
