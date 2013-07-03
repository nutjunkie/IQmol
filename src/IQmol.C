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

#include "IQmol.h"
#include <QColor>
#include <QToolButton>

#include <QDebug>

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


QString ScanDirectory(QDir const& dir, QString const& fileName, 
   Qt::CaseSensitivity caseSensitive)
{  
   QString filePath;
   QFileInfoList list(dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
   QFileInfoList::iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       if ((*iter).isDir()) {
          filePath = ScanDirectory((*iter).filePath(), fileName);
          if (!filePath.isEmpty()) break;
       }else if ( (*iter).fileName().contains(fileName, caseSensitive)){
          filePath = (*iter).filePath();
          break;
       }
   }

   return filePath;
}


/*
qglviewer::Quaternion FromEulerAngles(double const alpha, double const beta, 
   double const gamma)
{
   double a(alpha/2.0);
   double b(beta /2.0);
   double c(gamma/2.0);

   double q0( cos(a)*cos(b)*cos(c) + sin(a)*sin(b)*sin(c) );
   double q1( sin(a)*cos(b)*cos(c) - cos(a)*sin(b)*sin(c) );
   double q2( cos(a)*sin(b)*cos(c) + sin(a)*cos(b)*sin(c) );
   double q3( cos(a)*cos(b)*sin(c) - sin(a)*sin(b)*cos(c) );

   return qglviewer::Quaternion(q1,q2,q3,q0);
}


qglviewer::Vec ToEulerAngles(qglviewer::Quaternion const& q)
{
   double q0(q[3]);
   double q1(q[0]);
   double q2(q[1]);
   double q3(q[2]);

   double alpha = atan2(2.0*(q0*q1+q2*q3), 1.0-2.0*(q1*q1+q2*q2));
   double beta  = asin(2.0*(q0*q2+q1*q3));
   double gamma = atan2(2.0*(q0*q3+q1*q2), 1.0-2.0*(q2*q2+q3*q3));

   return qglviewer::Vec(alpha, beta, gamma);
}
*/


qglviewer::Quaternion FromEulerAngles(double const alpha, double const beta, 
   double const gamma)
{
   double a(alpha/2.0);
   double b(beta /2.0);
   double c(gamma/2.0);

   double q0( cos(a-c)*sin(b) );
   double q1( sin(a-c)*sin(b) );
   double q2( sin(a+c)*cos(b) );
   double q3( cos(a+c)*cos(b) );
qDebug() << "Euler angles read in" << alpha <<  beta << gamma;

   return qglviewer::Quaternion(q0,q1,q2,q3);
}


qglviewer::Vec ToEulerAngles(qglviewer::Quaternion const& q)
{
   double q0(q[0]);
   double q1(q[1]);
   double q2(q[2]);
   double q3(q[3]);

   double alpha = atan2((q0*q2+q1*q3), -(q1*q2-q0*q3));
   double beta  = acos(-q0*q0 -q1*q1 +q2*q2 +q3*q3);
   double gamma = atan2(q0*q2-q1*q3, (q1*q2+q0*q3));
qDebug() << "Euler angles computed" << alpha <<  beta << gamma;


   return qglviewer::Vec(alpha, beta, gamma);
}

} // end namespace IQmol
