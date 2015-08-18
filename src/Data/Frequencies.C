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


double Frequencies::maxFrequency() const
{
   double max(0.0);

   VibrationalModeList::const_iterator iter;
   for (iter = m_modes.begin(); iter != m_modes.end(); ++iter) {
       if ((*iter)->frequency() > max) max = (*iter)->frequency();
   }

   return max; 
}


double Frequencies::maxIntensity() const
{
   double max(0.0);

   VibrationalModeList::const_iterator iter;
   for (iter = m_modes.begin(); iter != m_modes.end(); ++iter) {
       if ((*iter)->intensity() > max) max = (*iter)->intensity();
   }

   return max; 
}


double Frequencies::maxRamanIntensity() const
{
   double max(0.0);

   VibrationalModeList::const_iterator iter;
   for (iter = m_modes.begin(); iter != m_modes.end(); ++iter) {
       if ((*iter)->ramanIntensity() > max) max = (*iter)->ramanIntensity();
   }

   return max; 
}



void Frequencies::setThermochemicalData(double const zpve, double const enthalpy, 
   double const entropy)
{
   m_zpve     = zpve; 
   m_enthalpy = enthalpy;
   m_entropy  = entropy;
}

void Frequencies::dump() const
{
   qDebug() << "  Thermochemical Data:"
            << "ZPVE =" << m_zpve     << "kcal/mol"
            << " H ="   << m_enthalpy << "kcal/mol"
            << " S ="   << m_entropy  << "cal/(mol.K)";
   m_modes.dump();
}

} } // end namespace IQmol::Data
