#ifndef QUI_QUISTRING_H
#define QUI_QUISTRING_H

/*!
 *  \file QuiString.h 
 *  
 *  \brief In theory this file should isolate the differeneces between using
 *  QStrings in the QUI and std::strings in the code.
 *  
 *  \author Andrew Gilbert
 *  \date   October 2008
 */


#ifdef QT_CORE_LIB
#include <QString>
#else
#include <string>
#include <cctype>
#include <algorithm>
#endif


namespace Qui {

#ifdef QT_CORE_LIB

typedef QString String;

inline String ToUpper(String s) {
   return s.toUpper();
}

inline String ToLower(String s) {
   return s.toLower();
}

#else

typedef std::string String;

inline String ToUpper(std::string s) {
   String t;
   std::transform(s.begin(), s.end(), t.begin(), std::toupper);
   return t;
}

inline String ToLower(String s) {
   String t;
   std::transform(s.begin(), s.end(), t.begin(), std::tolower);
   return t;
}

#endif


} // end namespace Qui


#endif
