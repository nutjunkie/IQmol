#ifndef IQMOL_GLSHAPELIBRARY_H
#define IQMOL_GLSHAPELIBRARY_H
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

#include <QMap>
#include "GLShape.h"


namespace IQmol {
namespace GLShape {

   /// Library is a singleton class that manages call lists for commonly used
   /// simple shapes.
   class Library {
      public:
         static Library& instance();

         template <class T>
         GLuint checkout(T const&);
         void checkin(GLuint const);

      private:
         static Library* s_instance;
         static QMap<Base*, GLuint> s_catalogue;
         static QMap<GLuint, GLuint> s_count;

         void addShape(Base*);

         static void destroy();
         Library() { }
         explicit Library(Library const&) { }
         ~Library() { }
   };


} } // end namespace IQmol::GLShape

#endif
