#ifndef QUI_SYSTEM_H
#define QUI_SYSTEM_H

/*!
 *  \file System.h
 *
 *  \brief Function delclarations for the platform-dependent functions in the
 *  System namespace. 
 *   
 *  \author Andrew Gilbert
 *  \date April 2008
 */

#include <vector>
#include "QString"
#include "QFileInfo"
#include "QStringList"


namespace Qui {
namespace System {


void KillProcessID(unsigned int const pid);

std::vector<unsigned int> GetMatchingProcessIds(QString const& pattern);
std::vector<unsigned int> GetParentProcessChain(unsigned int const pid);

QStringList RunCommand(QFileInfo const& command, QStringList const& args,
   unsigned int waitTimeInMilliSeconds = 5000);


} } // end namespace Qui

#endif
