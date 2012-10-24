/*!
 *  \file GetParentProcessChain.C
 *
 *  \brief Wrapper function to provide a list of PIDs which give a chain of
 *  ancestry of a child process.
 *
 *  \b Note: This function is outside the QProcess framework - it operates on
 *  raw PIDs.
 *
 *  \author Andrew Gilbert
 *  \date   April 2009
 */

#include "System.h"
#include <QString>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <vector>
#include <map>

#include <QtDebug>

namespace GetPIDs {
   std::vector<unsigned int> getpids(unsigned int);
}


namespace Qui {
namespace System {

#ifdef Q_WS_WIN

std::vector<unsigned int> GetParentProcessChain(unsigned int const pid) {
   return GetPIDs::getpids(pid);
}


#else

std::vector<unsigned int> GetParentProcessChain(unsigned int const pid) {
   std::vector<unsigned int> ancestry;

   QFileInfo psCommand("/bin/ps");
   QStringList args;
   args << "xww" << "-o" << "pid,ppid";

   QStringList lines(RunCommand(psCommand, args));
   QStringList tokens;

   std::map<unsigned int,unsigned int> psMap;
   unsigned int child, parent;
   bool isUInt0, isUInt1;

   for (int i = 0; i < lines.size(); ++i) {
       tokens = lines[i].split(QRegExp("\\s+"), QString::SkipEmptyParts);
       if (tokens.count() == 2) {
          child  = tokens[0].toUInt(&isUInt0);
          parent = tokens[1].toUInt(&isUInt1);
          if (isUInt0 && isUInt1) {
             psMap[child] = parent;
qDebug() << "PSMap" << child << parent;
          }else {
             qDebug() << "Bad PID in GetParentProcessChain:" 
                       << tokens[0] << tokens[1];
          }
       }else {
          qDebug() << "Incorrect number of tokens from ps"
                   << "command in GetParentProcessChain:" << lines[i];
       }
   }

   std::map<unsigned int,unsigned int>::iterator iter; 
   ancestry.push_back(pid);
   child = pid;
      
   for (;;) {
qDebug() << "searching for" << child;
       iter = psMap.find(child); 
       if (iter == psMap.end()) break;
       parent = iter->second;
qDebug() << "adding parent" << parent;
       ancestry.push_back(parent);
       child = parent;
   }

   return ancestry;
}

#endif


} }  // end namespace Qui::System
