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

#include "GLShapeLibrary.h"
#include "GLShape.h"
#include <cstdlib>


namespace IQmol {
namespace GLShape {

Library* Library::s_instance = 0;
QMap<Base*, GLuint> Library::s_catalogue;
QMap<GLuint, GLuint> Library::s_count;


Library& Library::instance()
{
   if (s_instance == 0) {
      s_instance = new Library();
      atexit(Library::destroy);
   }
   return *s_instance;
}


void Library::destroy()
{
   QMap<Base*, GLuint>::iterator iter;
   for (iter = s_catalogue.begin(); iter != s_catalogue.end(); ++iter) {
       delete iter.key();
       glDeleteLists(iter.value(), 1);
   }
}


template <class T>
GLuint Library::checkout(T const& shape)
{
   T* s;
   QMap<Base*, GLuint>::iterator iter;
   for (iter = s_catalogue.begin(); iter != s_catalogue.end(); ++iter) {
       if ( (s = dynamic_cast<T>(iter.key())) ) {
          if ( *s == shape) {
             ++s_count[iter.value()];
             return iter.value();
          }
       }
   }

   // shape is not in the Library, so we compile a new one
   s = new T(shape);
   GLuint callList(glGenLists(1));
   glNewList(callList, GL_COMPILE);
      s->draw();
   glEndList();

   s_catalogue.insert(s, callList);
   s_count.insert(callList, 1);
   return callList;
}


void Library::checkin(GLuint const callList)
{
   if (!s_count.contains(callList)) return;

   --s_count[callList];
   if (s_count[callList] == 0) {
      Base* shape(s_catalogue.key(callList));
      s_catalogue.remove(shape);
      s_count.remove(callList);
      glDeleteLists(callList, 1);
   }
}


} } // end namespace IQmol::GLShape
