#ifndef QUI_CONDITIONS_H
#define QUI_CONDITIONS_H

/*!
 *  \file Conditions.h 
 *
 *  \brief Declarations for custom Condition functions.  See Logic.h for details.
 *   
 *  \author Andrew Gilbert
 *  \date August 2008
 */


namespace Qui {


bool True();
bool False();
bool isCompoundFunctional();
bool isPostHF();
bool isDFT();
bool requiresDerivatives();
bool requiresFirstDerivatives();
bool requiresSecondDerivatives();

} // end namespace Qui
#endif
