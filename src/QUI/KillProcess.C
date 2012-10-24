/*!
 *  \file KillProcess.C
 *
 *  \brief A low-level function for killing a process based on its PID.
 *
 *  \b Note: This function is outside the QProcess framework.  It is required
 *  because if the process started with a QProcess object spawns a child
 *  process (via a script, for example), the child process is not killed when
 *  QProcess::kill() is invoked on the parent.
 *
 *  \author Andrew Gilbert
 *  \date   April 2009
 */

#include "System.h"
#include <QString>
#include <QFileInfo>
#include <QStringList>

#include <QtDebug>


namespace Qui {
namespace System {


#ifdef Q_WS_WIN

void KillProcessID(unsigned int const pid) {
   QFileInfo taskkill("/Windows/System32/taskkill.exe");
   QFileInfo tskill("/Windows/System32/tskill.exe");
   QStringList args;

   if (taskkill.exists()) {  // Vista
      args << "/f" << "/pid" << QString::number(pid);
      RunCommand(taskkill, args);
   }else {                   // XP
      args << QString::number(pid);
      RunCommand(tskill, args);
   }
}

#else

void KillProcessID(unsigned int const pid) {
   QStringList args;
   args << "-HUP" << QString::number(pid);
   QFileInfo killCommand("/bin/kill");
   RunCommand(killCommand, args);
}

#endif


}  } // end namespace Qui::System
