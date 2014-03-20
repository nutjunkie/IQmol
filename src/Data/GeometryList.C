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

#include "GeometryList.h"
#include <QDebug>


namespace IQmol {
namespace Data {

//template<> const Type::ID GeometryList::TypeID = Type::GeometryList;
template<> const Type::ID Data::List<Data::Geometry>::TypeID = Type::GeometryList;

void GeometryList::setDefaultIndex(int index) 
{ 
   if (index < 0) {
      m_defaultIndex = size()-1;
   }else if (index < size()) {
      m_defaultIndex = index; 
   }
}


void GeometryList::dump() const
{
   qDebug() << "GeometryList of length" << size() << "default index:" << m_defaultIndex;
   List<Geometry>::dump();
}

} } // end namespace IQmol::Data
