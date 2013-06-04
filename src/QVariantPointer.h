#ifndef QVARIANTPOINTER_H
#define QVARIANTPOINTER_H
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

#include <QVariant>

     
/// Simple class to interconvert pointers and QVariant objects;
/// 
///  \code
///    MyClass *p;
///    QVariant  v = QVariantPointer<MyClass>::toQVariant(p);
///    MyClass *p1 = QVariantPointer<MyClass>::toPointer(v);
///  \endcode
  
template <class T> class QVariantPointer {
   public:
      static T* toPointer(QVariant v)
      {
         return  (T *) v.value<void *>();
      }

      static QVariant toQVariant(T* ptr)
      {
         return qVariantFromValue((void *) ptr);
      }
};

#endif
