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

/// \file This is a stand-alone  test function for the parser module

#include "ParseFile.h"
#include "IQmolParser.h"
#include "QsLog.h"
#include "QsLogDest.h"
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QFile>
#include <QTextStream>
 
#include "openbabel/obconversion.h"
#include "openbabel/format.h"


using namespace IQmol;

bool initOpenBabel()
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

   if (OpenBabel::OBFormat::GetPlugin("formats","xyz")) {
      qDebug() << "OpenBabel okay without init";
      return true;
   }

   // This is needed to initialize the loading of plugins, go figure
   OpenBabel::OBConversion();

   if (OpenBabel::OBFormat::GetPlugin("formats","xyz")) {
      qDebug() << "OpenBabel initialized with OBConversion";
      return true;
   }

   qDebug() << "'Tis buggered Jim";
   return false;
}



// Install this to catch all the qDebug calls as well
void myMessageHandler(QtMsgType type, const char *msg)
{
   QString txt;
   switch (type) {
      case QtDebugMsg:
         QLOG_DEBUG() << QString(msg);
         break;
      case QtWarningMsg:
         QLOG_WARN() << QString(msg);
         break;
      case QtCriticalMsg:
         QLOG_ERROR() << QString(msg);
         break;
      case QtFatalMsg:
         QLOG_FATAL() << QString(msg);
         abort();
   }
}
 

void parse(int argc, char* argv[])
{
   QApplication app(argc, argv);
   QStringList  args(QCoreApplication::arguments());

   const QString usage("Usage:\n    Parser [-a][-d] filename");
   QString input;

   bool debugLogFile(false);
   bool archiveFile(false);

   if (args.size() <= 1) throw usage;

   for (int i = 1; i < args.size(); ++i) {
       if (args[i] == "-d") {
          debugLogFile = true;
       }else if (args[i] == "-a") {
          archiveFile = true;
       }else {
          input = args[i];
          break;
       } 
   }

   if (input.isEmpty()) throw usage;

   // Setup logging.  There is some weird scoping problem if we add any of the
   // following into the if (debugLogFile) clause.
   QsLogging::Logger& logger = QsLogging::Logger::instance();
   logger.setLoggingLevel(QsLogging::TraceLevel);
   logger.setDateStamp(false);
   QString logFile(input + ".log");
   QsLogging::DestinationPtr fileDestination;

   if (debugLogFile) {
      fileDestination = QsLogging::DestinationFactory::MakeFileDestination(logFile);
      logger.addDestination(fileDestination.get());
      qInstallMsgHandler(myMessageHandler);
   }
  
   initOpenBabel();
   
   Parser::ParseFile parseFile(input);
   parseFile.start();
   parseFile.wait(); 

   QStringList errors(parseFile.errors());
   if (!errors.isEmpty()) {
      qDebug() << "Error parsing file:" << input;
      QStringList::iterator iter;
      for (iter = errors.begin(); iter != errors.end(); ++iter) {
          qDebug() << "  " << *iter;
      }
      return;
   }

   Data::Bank const& data (parseFile.data());
   data.dump();

   // Save an IQmol archive
   if (archiveFile) {
      QString output(input + ".iqmol");
      Parser::IQmol iqmol;
      iqmol.saveData(output, const_cast<Data::Bank&>(data));
   }
}


int main(int argc, char* argv[])
{
   try {
       parse(argc, argv);
       qDebug() << "*** ParseFile complete ***";
   } catch (QString const& ex) {
       qDebug() << ex;
   } catch (...) {
       qDebug() << "Don't go be throwing tis, fool";
   } 

   return 0;
}


