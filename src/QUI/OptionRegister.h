#ifndef QUI_OPTIONREGISTER_H
#define QUI_OPTIONREGISTER_H

/*!
 *  \class OptionRegister 
 *  
 *  \brief Provides a typedef for the QChem options Register.  This should be
 *  set up so that the Register can be either Qt based (using QtNode and
 *  QString) or STL based (using Nodes and std::string).
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "Register.h"
#include "QuiString.h"

#ifdef QT_CORE_LIB
#include "QtNode.h"
#else
#include "Node.h"
#endif


namespace Qui {

#ifdef QT_CORE_LIB
   typedef QtNode NodeT;
#else
   typedef Node<String> NodeT;
#endif

typedef Register<String, NodeT> OptionRegister;

} // end namespace Qui


#endif
