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

#include "System.h"
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QMessageBox>
#include <QMap>
#include <QtDebug>

#ifdef Q_WS_WIN
#include <windows.h>
#endif


namespace GetPIDs {
   std::list<unsigned int> getpids(unsigned int);
}

namespace IQmol {
namespace System {

QStringList RunCommand(QString const& command, QStringList const& arguments, 
   unsigned int waitTimeInMilliSeconds) 
{
   QStringList output;
   QFileInfo cmd(command);

   if (cmd.exists()) {
      qDebug() << "Executing command" << command << "with args:" << arguments;
               
      QProcess process;
      process.setWorkingDirectory(cmd.absolutePath());
      process.start(cmd.fileName(), arguments);

      if (process.waitForFinished(waitTimeInMilliSeconds)) {
         QString psOutput(process.readAllStandardOutput());
         output = psOutput.split(QRegExp("\\n"));
      }else {
         // should throw an exception
         process.kill();
      }

   }else {
      QString msg("Could not find command ");
      msg += command;
      QMessageBox::warning(0, "IQmol", msg);
   }

   return output;
}


/*
QString StatProcess(QString const& name, unsigned int const pid)
{
   QString time("0:00:00");
   QString pattern;
   QStringList args;
   QStringList tokens;

#ifdef Q_WS_WIN
   QFileInfo tasklist("/Windows/System32/tasklist.exe");
   if (!tasklist.exists()) return QString();

   QString spid("\"PID eq ");
   spid += QString::number(pid) + "\"";
   QString sname("\"IMAGENAME eq ");
   sname += name + "\"";
   args << "/v" << "/fo list" << "/fi " + spid << "/fi " + sname;
   args = RunCommand(tasklist.filePath(), args);
   pattern = "CPU";
#else
   args << "xc" << "-o" << "command,pid,time";
   args << QString::number(pid);
   args = RunCommand("/bin/ps", args);
   pattern = name;
#endif

   for (int i = 0; i < args.size(); ++i) {
       if (args[i].contains(pattern, Qt::CaseInsensitive)) {
          tokens = args[i].split(QRegExp("\\s+"), QString::SkipEmptyParts);
          time = tokens.last();
          break;
       } 
   }

   return time;
}
*/


QString QueryCommand()
{
   QString cmd;
#ifdef Q_WS_WIN
   QFileInfo tasklist("/Windows/System32/tasklist.exe");
   if (!tasklist.exists()) return QString("Error: tasklist.exe not found");

   QString spid("\"PID eq ${JOB_ID}\"");
   QString sname("\"IMAGENAME eq ${EXE_NAME}\"");
   QStringList args;
   args << "/v" << "/fo list" << "/fi " + spid << "/fi " + sname;
   cmd = tasklist.filePath() + " " + args.join(" ");
#else
   cmd = "/bin/ps xc -S -o command=,pid=,time= ${JOB_ID}";
#endif
   return cmd;
}


QString KillCommand() 
{
   QString cmd;
#ifdef Q_WS_WIN
   QFileInfo taskkill("/Windows/System32/taskkill.exe");
   QFileInfo tskill("/Windows/System32/tskill.exe");

   if (taskkill.exists()) {       // Vista
      cmd = taskkill.filePath() + " /f /pid ${JOB_ID}";
   }else if (tskill.exists()) {   // XP
      cmd = tskill.filePath() + " ${JOB_ID}";
   }else {
      cmd = "Error: taskkill.exe or tskill.exe not found";
   }
#else
   cmd = "/bin/kill -TERM ${JOB_ID}";
#endif
   return cmd;
}


QString SubmitCommand()
{
   QString cmd;
#ifdef Q_WS_WIN
   cmd = "${QC}/qcenv_s.bat ${JOB_NAME}.inp ${JOB_NAME}.out";
#else
   cmd = "./${JOB_NAME}.run";
#endif
   return cmd;
}



/*

void KillProcess(unsigned int const pid) 
{
#ifdef Q_WS_WIN
   QFileInfo taskkill("/Windows/System32/taskkill.exe");
   QFileInfo tskill("/Windows/System32/tskill.exe");
   QStringList args;

   if (taskkill.exists()) {  // Vista
      args << "/f" << "/pid" << QString::number(pid);
      RunCommand(taskkill.filePath(), args);
   }else {                   // XP
      args << QString::number(pid);
      RunCommand(tskill.filePath(), args);
   }
#else
   QStringList args;
   args << "-HUP" << QString::number(pid);
   RunCommand("/bin/kill", args);
#endif
}
*/


unsigned int ExecutablePid(QString const& processName, QProcess const& parent) 
{
   // The strategy here is:
   //  (1) get a list of all PIDs that match processName
   //  (2) get the parentage of each of these
   //  (3) find the one who has a parent that matches the QProcess::pid()
   unsigned int qpid(ProcessID(parent));
   QList<unsigned int> ancestry;
   QList<unsigned int> pids(System::GetMatchingProcessIds(processName));
   QList<unsigned int>::iterator iter, jter;

   qDebug() << "Parent QProcess id:" << qpid;

   for (iter = pids.begin(); iter != pids.end(); ++iter) {
       qDebug() << processName << "process found on pid:" << *iter;
       ancestry = System::GetParentProcessChain(*iter);

       if (ancestry.contains(qpid)) {
          qDebug() << "Found child process" << processName << "on PID" << *iter;
          for (jter = ancestry.begin(); jter != ancestry.end(); ++jter) {
              if (*jter == qpid) {
                 qDebug() << "  ->" << *jter << "* (QProcess)";
              }else {
                 qDebug() << "  ->" << *jter;
              }
          }
          
		  // return the PID for the spawning script in case there are multiple
		  // jobs in the input file
          if (ancestry.size() > 1) return ancestry[1];
          return *iter;
       }
   }

   return 0;
}



int ProcessID(QProcess const& process)
{
#ifdef Q_WS_WIN
   _PROCESS_INFORMATION* pi(process.pid());
   if (pi) {
      return pi->dwProcessId;
   }else {
      return 0;
   }
#else
   return process.pid();
#endif

}



QList<unsigned int> GetMatchingProcessIds(QString const& pattern) 
{
   QList<unsigned int> pids;
   QStringList args;

#ifdef Q_WS_WIN
   QString cmd("/Windows/System32/tasklist.exe");
#else
   QString cmd("/bin/ps");
   // The command,pid ordering is chosen to match the output of tasklist
   args << "-x" << "-c" << "-o" << "command,pid";
#endif

   QStringList processes(RunCommand(cmd, args));
   QStringList tokens;

   for (int i = 0; i < processes.count(); ++i) {
       if (processes[i].contains(pattern, Qt::CaseInsensitive)) {
qDebug() << "MATCHED!" << processes[i];
          tokens = processes[i].split(QRegExp("\\s+"), QString::SkipEmptyParts);
          pids.append(tokens[1].toUInt());
       }
   }
   
   return pids;
}



QList<unsigned int> GetParentProcessChain(unsigned int const pid) 
{
#ifdef Q_WS_WIN
  return QList<unsigned int>::fromStdList(GetPIDs::getpids(pid));
#else
   QList<unsigned int> ancestry;

   QString psCommand("/bin/ps");
   QStringList args;
   args << "xww" << "-o" << "pid,ppid";

   QStringList lines(RunCommand(psCommand, args));
   QStringList tokens;

   QMap<unsigned int,unsigned int> psMap;
   unsigned int child, parent;
   bool isUInt0, isUInt1;

   for (int i = 0; i < lines.size(); ++i) {
       tokens = lines[i].split(QRegExp("\\s+"), QString::SkipEmptyParts);
       if (tokens.count() == 2) {
          child  = tokens[0].toUInt(&isUInt0);
          parent = tokens[1].toUInt(&isUInt1);
          if (isUInt0 && isUInt1) {
             psMap[child] = parent;
          }else if (tokens[0].toLower() == "pid") {
             qDebug() << "Bad PID in GetParentProcessChain:" 
                       << tokens[0] << tokens[1];
          }
       }else {
          //qDebug() << "Incorrect number of tokens from ps"
          //         << "command in GetParentProcessChain:" << lines[i];
       }
   }

   QMap<unsigned int,unsigned int>::iterator iter; 
   ancestry.append(pid);
   child = pid;
      
   for (;;) {
       iter = psMap.find(child); 
       if (iter == psMap.end()) break;
       parent = iter.value();
       ancestry.append(parent);
       child = parent;
   }

   return ancestry;
#endif
}

} } // end namespace IQmol::System
