#ifndef QUI_OPTIONREGISTER_H
#define QUI_OPTIONREGISTER_H

/*!
 *  \class OptionRegister 
 *  
 *  \brief Provides a typedef for the QChem options Register.  This should be
 *  set up so that the Register can be either Qt based (using QtNode and
 *  QString) or STL based (using Nodes and std::string).
 *  - nice idea, but never happened
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "Register.h"
#include "QtNode.h"

namespace Qui {

   typedef Register<QString, QtNode> OptionRegister;

} // end namespace Qui


#endif
