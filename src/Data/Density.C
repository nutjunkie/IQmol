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

#include "Density.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Data {


Density::Density(QString const& label, QList<double> const&  elements) : m_label(label)
{
   m_nBasis = round((std::sqrt(1.0+8.0*elements.size()) -1.0)/2.0);
   if (m_nBasis*(m_nBasis+1)/2 != elements.size()) {
      qDebug() << "Incorrect number of density matrix elements";
      return;
   }
   
   m_elements.resize(m_nBasis, m_nBasis);
   unsigned k(0);
   for (unsigned i = 0; i < m_nBasis; ++i) {
       for (unsigned j = i; j < m_nBasis; ++j, ++k) {
           m_elements(i,j) = m_elements(j,i) = elements[k];
       }
   }
}


void Density::dump() const
{
}

} } // end namespace IQmol::Data
