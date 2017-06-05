#ifndef IQMOL_IQMOL_H
#define IQMOL_IQMOL_H
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

/// \file  IQmol.h
/// \brief Grab-bag of function, typedef and constant definitions that are not
///        associated with a specific class.

#include <QString>
#include <QDir>
#include "QGLViewer/vec.h"
#include "QGLViewer/quaternion.h"
#include <cmath>
#include <limits>
#define FOREVER ( std::numeric_limits<int>::max() )

#define IQMOL_VERSION "2.9.0b"
#define IQMOL_YEAR "2017"

class QToolButton;
class QColor;


namespace IQmol {

   QString const DefaultMoleculeName = "Untitled";
   double const DefaultSceneRadius = 4.0;

//   enum Spin { Alpha, Beta };

   enum Axes { XAxis = 0x1, YAxis = 0x2, ZAxis = 0x4 };

 //  inline QString SpinLabel(Spin spin) {
 //     return (spin == Alpha) ? QString("Alpha") : QString("Beta");
 //  }

   QString subscript(QString const& s);
   QString superscript(QString const& s);

   /// Converts a QString containing a plain ASCII point group label (eg Civ)
   /// to one with rich text including subscripts and the infinity character.
   QString PointGroupForDisplay(QString const& pg);

   qglviewer::Vec ToEulerAngles(qglviewer::Quaternion const&);
   qglviewer::Quaternion FromEulerAngles(double const, double const, double const);

} // end namespace IQmol

#endif
