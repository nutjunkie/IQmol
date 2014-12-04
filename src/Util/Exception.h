#ifndef IQMOL_EXCEPTION_H
#define IQMOL_EXCEPTION_H
/*******************************************************************************
         
  Copyright (C) 2011-2013 Andrew Gilbert
      
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

#include "QsLog.h"
#include <exception>


namespace IQmol {

   /// Base class for IQmol exceptions which logs the exception.  
   /// Note: This class cannot be used for out of memory conditions 
   /// as it stores the message as a QByteArray.
   class Exception : public std::exception { 
      public:
         Exception(QString const& message) { 
            QLOG_ERROR() << "throw: " << message;
            m_byteArray = message.toLocal8Bit();
         } 

         ~Exception() throw () { }

         const char* what() const throw() { return m_byteArray.data(); }

      private:
         QByteArray m_byteArray;
   };

} // end namespace IQmol

#endif
