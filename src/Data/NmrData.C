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

#include "NmrData.h"
#include <QDebug>


namespace IQmol {
namespace Data {

//template<> const Type::ID NmrReferenceList::TypeID = Type::NmrReferenceList;

void Nmr::dump() const 
{
   qDebug() << "NMR isotropic shifts: " << m_isotropicShifts;
   qDebug() << "NMR relative shifts:  " << m_relativeShifts;
   qDebug() << "NMR couplings:";
   QStringList list(PrintMatrix(m_isotropicCouplings)); 

   QStringList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       qDebug() << *iter;
   }
}


bool Nmr::haveCouplings()
{
   return m_isotropicCouplings.size1() == m_isotropicShifts.size() &&
          m_isotropicCouplings.size2() == m_isotropicShifts.size();
}


bool Nmr::haveRelativeShifts()
{
   return !m_relativeShifts.isEmpty();
}

} } // end namespace IQmol::Data
