/*!
 *  \file GetMatchingProcesses.C
 *
 *  \brief Wrapper function to provide a list of PIDs whose processes match a
 *  certain string, in a platform independent way.
 *
 *  \b Note: This function is outside the QProcess framework.
 *
 *  \author Andrew Gilbert
 *  \date   April 2009
 */

#include "System.h"
#include <vector>
#include <QString>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>

#include <QtDebug>


namespace Qui {
namespace System {


std::vector<unsigned int> GetMatchingProcessIds(QString const& pattern) {
   std::vector<unsigned int> pids;
   QStringList args;

#ifdef Q_WS_WIN
   QFileInfo psCommand("/Windows/System32/tasklist.exe");
#else
   QFileInfo psCommand("/bin/ps");
   // The command,pid ordering is chosen to match the output of tasklist
   args << "-x" << "-c" << "-o" << "command,pid";
#endif

   QStringList processes(RunCommand(psCommand, args));
   QStringList tokens;

   for (int i = 0; i < processes.count(); ++i) {
       if (processes[i].contains(pattern, Qt::CaseInsensitive)) {
          tokens = processes[i].split(QRegExp("\\s+"), QString::SkipEmptyParts);
          pids.push_back(tokens[1].toUInt());
       }
   }
   
   return pids;
}


} }  // end namespace Qui::System
