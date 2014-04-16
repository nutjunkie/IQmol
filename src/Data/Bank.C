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

#include "Bank.h"
#include <QDebug>


namespace IQmol {
namespace Data {

Bank::~Bank() 
{ 
   if (m_deleteContents) {
       for (int i = 0; i < size(); ++i) {
           //qDebug() << "Bank bust: deleting " << Type::toString(at(i)->typeID());
           delete at(i);
       }
   }
}


void Bank::dump() const 
{
   int n(size());
   for (int i = 0; i < n; ++i) {
       qDebug() << "Data::Bank item" << i+1 << "of" << n << ":";
       qDebug() <<  Type::toString(at(i)->typeID());
       at(i)->dump();
   }
}

} } // end namespace IQmol::Data
