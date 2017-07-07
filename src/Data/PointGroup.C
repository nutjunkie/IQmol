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

#include "PointGroup.h"
#include "StringFormat.h"
#include <QDebug>


namespace IQmol {
namespace Data {


static const char* const SymmolStrings[]  = { "",
  "C1", "C2",   "C3",  "C4",  "C5",  "C6",  "C7",  "C8", 
  "Cs", "C2h",  "C3h", "C4h", "C5h", "C6h", "C7h", "C8h",
        "C2v",  "C3v", "C4v", "C5v", "C6v", "C7v", "C8v",
         "D2",  "D3",  "D4",  "D5",  "D6",  "D7",  "D8", 
         "D2d", "D3d", "D4d", "D5d", "D6d", "D7d", "D8d",
         "D2h", "D3h", "D4h", "D5h", "D6h", "D7h", "D8h",
         "Ci",         "S4",         "S6",         "S8", 
         "T",   "Td",  "Th",  "O",   "Oh",  "I",   "Ih", 
         "Civ", "Dih"
};


PointGroup::PointGroup(QString const& string)
{
   setPointGroup(string);
}


QString PointGroup::toGLString() const
{
   QString s;

   QChar ch(0x221E); // infinity
   if (m_group == Civ) {
      s = "C" + QString(ch) + "v";
   }else if (m_group == Dih) {
      s = "D" + QString(ch) + "h";
   }else if (m_group < MaxPG) {
      s = QString(SymmolStrings[m_group]);
   };

   return s;
}


QString PointGroup::toString() const
{
   QString pg(toGLString());
   pg = pg.left(1) + Util::subscript(pg.mid(1));
   return pg;
}


void PointGroup::setPointGroup(QString const& string)
{
   QString s(string.trimmed());
   m_group = undef;
   for (unsigned i = 0; i < MaxPG; ++i) {
       if (s == SymmolStrings[i]) {
          m_group = (Group)i;
          break;
       }
   }
}


void PointGroup::dump() const
{
   qDebug() << "Point Group" << toString();
}

} } // end namespace IQmol::Data
