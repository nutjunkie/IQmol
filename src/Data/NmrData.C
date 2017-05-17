/*******************************************************************************

  Copyright (C) 2011-2015 Andrew Gilbert

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

void Nmr::dump() const 
{
   qDebug() << "NMR method:     " << m_method;
   qDebug() << "NMR shieldings: " << m_shieldings;
   qDebug() << "NMR shifts:     " << m_shifts;
   QStringList list(PrintMatrix(m_couplings)); 

   QStringList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       qDebug() << *iter;
   }

   m_reference.dump();
}


bool Nmr::haveCouplings()
{
   return (int)m_couplings.size1() == m_shieldings.size() &&
          (int)m_couplings.size2() == m_shieldings.size();
}


bool Nmr::haveReference()
{
   return !m_reference.system().isEmpty();
}


QList<double> Nmr::shifts(QString const& isotope, NmrReference const& ref) const
{ 
   QList<double> shifts;

   if (ref.contains(isotope)) {
      double offset(ref.shift(isotope));
      for (int i = 0; i < m_atomLabels.size(); ++i) {
          if (m_atomLabels[i] == isotope) {
             shifts.append(m_shieldings[i]-offset);
          }
       }
   }

   return shifts;
}

} } // end namespace IQmol::Data
