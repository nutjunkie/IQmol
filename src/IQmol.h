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

/// \file  IQmol.h
/// \brief Various function, typedef and constant definitions that are not
///        associated with a specific class.

#include <QString>
#include "boost/function.hpp"
#include <cmath>
#include <limits>
#define FOREVER ( std::numeric_limits<int>::max() )

#define IQMOL_VERSION "2.1"

class QToolButton;
class QColor;


namespace IQmol {

   double const BohrToAngstrom = 0.529177249;
   double const AngstromToBohr = 1.0/BohrToAngstrom;
   QString const DefaultMoleculeName = "Untitled";
   double const DefaultSceneRadius = 4.0;

   typedef boost::function<double (double const, double const, double const)> Function3D;

   /// Enumerates the possible Modes that the Viewer can operate in.
   enum ViewerMode { Manipulate, Select, Build, ManipulateSelection, ReindexAtoms };

   enum Spin { Alpha, Beta };

   enum Axes { XAxis = 0x1, YAxis = 0x2, ZAxis = 0x4 };

   inline QString SpinLabel(Spin spin) {
      return (spin == Alpha) ? QString("Alpha") : QString("Beta");
   }

   inline int round(double d) {
      return (int)floor(d+0.5);
   }

   inline bool isEven(int i) {
      return ( (i % 2) == 0);
   }

   inline bool isOdd(int i) {
      return ( (i % 2) == 1);
   }

   QString subscript(QString const& s);
   QString superscript(QString const& s);

   /// Converts a QString containing a plain ASCII point group label (eg Civ)
   /// to one with rich text including subscripts and the infinity character.
   QString PointGroupForDisplay(QString const& pg);

   /// Convenience function that sets the background color of a QToolButton.
   void SetButtonColor(QToolButton& button, QColor const& color);

} // end namespace IQmol

#endif
