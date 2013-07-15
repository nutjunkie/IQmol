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

#include "AtomicProperty.h"
#include <openbabel/mol.h>
#include <vector>
#include <QDebug>


namespace IQmol {
namespace Data {

//  ---------- AtomicSymbol ----------
AtomicSymbol::AtomicSymbol(int const Z)
{
   if (Z > 0) m_symbol = QString(OpenBabel::etab.GetSymbol(Z));
}


void AtomicSymbol::dump() const
{
   qDebug() << m_symbol;
}



//  ---------- ScalarProperty ----------
void ScalarProperty::dump() const
{
   qDebug() << m_value;
}



//  ---------- Mass ----------
Mass::Mass(int const Z)
{
   if (Z > 0) m_value = OpenBabel::etab.GetMass(Z);
}



//  ---------- VdwRadius ----------
VdwRadius::VdwRadius(int const Z)
{
   if (Z > 0) m_value = OpenBabel::etab.GetVdwRad(Z);
}



//  ---------- AtomColor ----------
AtomColor::AtomColor(int const Z)
{
   if (Z > 0) {
      std::vector<double> rgb(OpenBabel::etab.GetRGB(Z));
      m_color[0] = rgb[0];
      m_color[1] = rgb[1];
      m_color[2] = rgb[2];
      m_color[3] = 1.0;
   }
}
  

void AtomColor::set(double const red, double const green, double const blue)
{
   m_color[0] = red;
   m_color[1] = blue;
   m_color[2] = green;
}


QColor AtomColor::get() const
{
   QColor color;
   color.setRgbF(m_color[0], m_color[1], m_color[2]);
   return color;
}


void AtomColor::dump() const
{
   qDebug() << "Red ="   << QString::number(m_color[0], 'f', 3)
            << "Green =" << QString::number(m_color[1], 'f', 3)
            << "Blue ="  << QString::number(m_color[2], 'f', 3);
}

} } // end namespace IQmol::Data
