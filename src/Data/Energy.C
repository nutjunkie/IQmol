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

#include "Energy.h"
#include "Constants.h"
#include <QDebug>


namespace IQmol {
namespace Data {


QString Energy::toString(Units const units)
{
   QString s;

   switch (units) {
      case Hartree:       s = "Eh";        break;
      case EV:            s = "eV";        break;
      case KJMol:         s = "kJ/mol";    break;
      case KCalMol:       s = "kcal/mol";  break;
      case Wavenumber:    s = "cm^(-1)";   break;
      case MHz:           s = "MHz";       break;
   }

   return s;
}


double Energy::conversion(Units const units) const 
{
   double conv(0.0);

   switch (units) {
      case Hartree:       conv = 1.00000000;                      break;
      case EV:            conv = Constants::HartreeToEv;          break;
      case KJMol:         conv = Constants::HartreeToKJmol;       break;
      case KCalMol:       conv = Constants::HartreeToKCalmol;     break;
      case Wavenumber:    conv = Constants::HartreeToWavenumber;  break;
      case MHz:           conv = Constants::HartreeToMHz;         break;
   }

   return conv;
}


double Energy::value(Units const units) const
{
   return m_value * conversion(units) / conversion(m_units);
}


QString Energy::format(double const energy, Units const units, char const fmt, 
   int const precision)
{
   QString s(QString::number(energy, fmt, precision));
   s += " " + toString(units);
   return s;
}


QString Energy::format(char const fmt, int const precision) const
{
   return format(m_value, m_units, fmt, precision);
}

            
void Energy::dump() const
{
   qDebug() << "  " << format();
}

} } // end namespace IQmol::Data
