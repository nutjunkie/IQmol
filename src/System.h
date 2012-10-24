#ifndef IQMOL_SYSTEM_H
#define IQMOL_SYSTEM_H
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

#include <QStringList>


/// \brief Function delclarations for the platform-dependent functions in the
/// System namespace. 

class QProcess;

namespace IQmol {
namespace System {

   QStringList RunCommand(QString const& command, QStringList const& arguments,
      unsigned int waitTimeInMilliSeconds = 5000);

   /// A low-level function for killing a process based on its PID.
   /// \b Note: This function is outside the QProcess framework.  It is required
   /// because if the process started with a QProcess object spawns a child
   /// process (via a script, for example), the child process is not killed when
   /// QProcess::kill() is invoked on the parent.
   void KillProcess(unsigned int const pid);

   // Finds the system process ID number of the processName process.  The
   // process is expected to have been spawned by the given QProcess but 
   // the pids may not necessarily be the same as the QProcess may spawn 
   // additional processes (e.g. shell scripts) 
   unsigned int ExecutablePid(QString const& processName, QProcess const& parent);

   /// Wrapper Function around QProcess::pid() to get around Windows annoyances.
   int ProcessID(QProcess const& process);
 
   /// Provides a list of PIDs whose processes match a certain string, in a
   /// platform independent way.  \b Note: This function is outside the QProcess 
   /// framework.
   QList<unsigned int> GetMatchingProcessIds(QString const& pattern);

   /// Provides a list of PIDs which give a chain of ancestry of a child process.
   /// \b Note: This function is outside the QProcess framework - it operates on
   /// raw PIDs.
   QList<unsigned int> GetParentProcessChain(unsigned int const pid);

   /// Checks to see if if the given PID is running and returns the run time in 
   /// the format hh:mm:ss if it is.   A return of 0:00:00 indicates the process 
   /// is no longer running.  An empty string indicates that the process command 
   /// (either ps or tasklist) could not be found.  Note the match is done on 
   /// both the PID and executable name to lessen the chance of an accidental 
   /// match.
   QString StatProcess(QString const& name, unsigned int const pid);

   /// Returns the command used to query a process on the local machine
   QString QueryCommand();
   /// Returns the command used to kill a process on the local machine
   QString KillCommand();

} } // end namespace Qui

#endif
