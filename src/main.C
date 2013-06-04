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
#include "Lebedev.h"

#ifdef Q_WS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
    IQmol::IQmolApplication iqmol(argc, argv);
    Q_INIT_RESOURCE(IQmol);

    // Setup logging;
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    QString const logFile(IQmol::Preferences::LogFilePath());

    // Weird scoping going on here
    QsLogging::DestinationPtr fileDestination;
    if (IQmol::Preferences::LoggingEnabled()) {
       fileDestination = QsLogging::DestinationFactory::MakeFileDestination(logFile);
       logger.addDestination(fileDestination.get());
#ifdef Q_WS_WIN
       if (IQmol::Preferences::LogFileHidden()) {
          int fileHidden(0x2);  // Where is FILE_ATTRIBUTE_HIDDEN defined?
          WCHAR* fnam = (WCHAR*)logFile.constData();
          SetFileAttributes(fnam, fileHidden); 
       }
#endif
    }

    QLOG_INFO() << "---------- Session Started ----------";
    QLOG_INFO() << "IQmol Version: " << IQMOL_VERSION;


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
