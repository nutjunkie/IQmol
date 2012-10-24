/*!
 *  \file ProcessQChem.C
 *
 *  \brief Contains the derived QChem process class definitions.
 *
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "Qui.h"
#include "System.h"
#include "ProcessQChem.h"
#include <QMessageBox>
#include <QDir>
#include <algorithm>

#include "QuiReadFile.h"
#include <QtDebug>

#ifdef QCHEM_UI
#include "../Preferences.h"
#define Preferences IQmol::Preferences
#else
#include "Preferences.h"
#endif


namespace Qui {
namespace Process {


// ********** QChem Process ********** //

QChem::QChem(QObject* parent, QString const& input, QString const& output) 
  : Monitored(parent, Preferences::QChemRunScript()) {

   setInputFile(input);
   setOutputFile(output);

   QFileInfo inputFileInfo(input);
   QFileInfo outputFileInfo(output);
   qDebug() << "Setting Process::QChem working directory to" 
            << inputFileInfo.path();
   setWorkingDirectory(inputFileInfo.path());

   QStringList args;
   args << inputFileInfo.fileName();
#ifndef Q_WS_WIN
   args << outputFileInfo.fileName();
#endif
   setArguments(args);

   connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), 
      this, SLOT(cleanUp(int, QProcess::ExitStatus)));
}



void QChem::cleanUp(int, QProcess::ExitStatus) {
   checkForErrors();
   renameFchkFile();
}


void QChem::checkForErrors() {
   QFile file(outputFile());
   QStringList lines(ReadFileToList(file));
   QRegExp rx ("*Q-Chem fatal error*");
   rx.setPatternSyntax(QRegExp::Wildcard);

   int error(lines.indexOf(rx));
   if (error >= 0 && lines.size() > error+2) {
      m_error = lines[error+2];
      m_status = Status::Error;
   } 
}


void QChem::renameFchkFile() {
   QFileInfo output(outputFile());
   QFileInfo oldFile(output.path() + "/Test.FChk");
   QFileInfo newFile(output.path() + "/" + output.completeBaseName() + ".FChk");

   if (oldFile.exists()) {
      QDir dir(output.dir());
      if (newFile.exists()) dir.remove(newFile.filePath());
      dir.rename(oldFile.filePath(), newFile.filePath());
      setAuxFile(newFile.filePath());
   }
}


void QChem::notifyJobFinished() {
}


void QChem::kill() {
   if (status() != Status::Running) {
      return; //no process to kill
   }

   if (QMessageBox::question(0, "Kill Job?", 
       "Are you sure you want to terminate the Q-Chem job " + m_inputFile, 
       QMessageBox::Cancel | QMessageBox::Ok) == QMessageBox::Cancel) {
       return;
   }

   qDebug() << "About to kill QProcess" << Process::pid() ;

// ------
   qDebug() << "Parent PID is:" << Process::pid();
   qDebug() << " QChem PID is:" << exePid();
// ------

   // This uses QChem::pid() defined below
   unsigned int  id(exePid());

   if (id > 0) {
      qDebug() << "qcprog.exe found on process" << id;
      System::KillProcessID(id);
      m_status = Status::Killed;
   }else {
      QString msg("Unable to determine process ID for job termination");
      QMessageBox::warning(0, "Kill Job Failed", msg);
   }
}


// The strategy here is:
//  (1) get a list of all the running QChem PIDs
//  (2) get the parentage of each of these
//  (3) find the one who has a parent that matches the QProcess::pid()

unsigned int QChem::exePid() {
//#ifdef Q_WS_WIN
//   QString qcexe("qchem_s.exe");
//#else
//   QString qcexe("qcprog.exe");
//#endif

   QString qcexe("qcprog.exe");

   unsigned int ppid(Process::pid());
   std::vector<unsigned int> ancestry;
   std::vector<unsigned int> pids(System::GetMatchingProcessIds(qcexe));
   std::vector<unsigned int>::iterator iter, jter;

   for (iter = pids.begin(); iter != pids.end(); ++iter) {
       qDebug() << qcexe << "process found on pid:" << *iter;
       ancestry = System::GetParentProcessChain(*iter);

       for (jter = ancestry.begin(); jter != ancestry.end(); ++jter) {
           qDebug() << " ->" << *jter;
       }

       jter = std::find(ancestry.begin(), ancestry.end(), ppid);
       if (jter != ancestry.end()) {
          return *iter;
       }
   }

   return 0; 
}


} } // end namespaces Qui::Process
