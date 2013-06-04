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

#include "Frequencies.h"
#include <QDebug>


namespace IQmol {
namespace Data {

void Frequencies::setThermochemicalData(double const zpve, double const enthalpy, 
   double const entropy)
{
   m_zpve     = zpve; 
   m_enthalpy = enthalpy;
   m_entropy  = entropy;
}

void Frequencies::dump() const
{
   qDebug() << "Thermochemical Data: " << m_zpve << m_enthalpy << m_entropy;
   m_modes.dump();
}

} } // end namespace IQmol::Data
