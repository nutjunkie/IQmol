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
#include "Exception.h"
#include "IQmol.h"
#include <QStringList>
#include <QDir>
#include "QsLog.h"
#include "QsLogDest.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#endif


//************************************************************************************
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>


int LogFileDescriptor(-1);

void signalHandler(int signal) 
{
  void   *array[20];
  size_t size(backtrace(array, 20));

  if (LogFileDescriptor < 0) {
     fprintf(stderr, "Fatal error encountered: signal %d\n", signal);
     backtrace_symbols_fd(array, size, STDERR_FILENO);
  }else {
     QLOG_FATAL() << "Fatal error encountered: signal " << signal;
     backtrace_symbols_fd(array, size, LogFileDescriptor);
     backtrace_symbols_fd(array, size, STDERR_FILENO);
     QLOG_FATAL() << "Message: " << strsignal(signal);
  }

  if (signal != SIGABRT) throw IQmol::SignalException();
}

//************************************************************************************


int main(int argc, char *argv[]) 
{
    // Install our signal handler for all the signals we care about
    for (int i = 4; i < 30; i++) {
        signal(i, signalHandler);   
    }

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
       LogFileDescriptor = fileDestination->handle();

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

    int ret(0);

    try {
       ret = iqmol.exec();
       QLOG_INFO() <<  "Return code:" << ret;
       QLOG_INFO() <<  "----------- Session Ended -----------";
    } catch (std::exception& e ) {
       QLOG_FATAL() << e.what();
       QLOG_FATAL() << "------- EXCEPTION ENCOUNTERED -------";
       iqmol.exception();
    }

    return ret;
}
