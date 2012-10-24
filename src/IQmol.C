#ifndef IQMOL_IQMOL_H
#define IQMOL_IQMOL_H
/*******************************************************************************

  Copyright (C) 2011 Andrew Gilbert

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

#include "IQmol.h"
#include <QColor>
#include <QToolButton>


namespace IQmol {

QString subscript(QString const& s) {
   return QString("<span style=\"font-size:16pt; vertical-align:sub;\">" + s + "</span>");
}


QString superscript(QString const& s) {
   return QString("<span style=\"font-size:16pt; vertical-align:sup;\">" + s + "</span>");
}


QString PointGroupForDisplay(QString const& pg) 
{
   QString pointGroup;
   QChar ch(0x221E); // infinity

   if (pg == "Civ") {
      pointGroup = "C" + IQmol::subscript(QString(ch) + "v");
   }else if (pg == "Dih") {
      pointGroup = "D" + subscript(QString(ch) + "h");
   }else { 
      pointGroup = pg.left(1) + subscript(pg.mid(1));
   }
   return pointGroup;
}


void SetButtonColor(QToolButton& button, QColor const& color) 
{
   if (!color.isValid()) return;
   QString bg("background-color: ");
   bg += color.name();
   button.setStyleSheet(bg);
}

} // end namespace IQmol

#endif
