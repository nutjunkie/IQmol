#ifndef IQMOL_AXESLAYER_H
#define IQMOL_AXESLAYER_H
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

#include "GlobalLayer.h"


namespace IQmol {
namespace Layer {

   /// Representation of a set of axes centered a the world origin.  The
   /// colors are X(red), Y(green), Z(blue).
   class Axes : public Global {

      Q_OBJECT

      public:
         explicit Axes() : Global("Axes") { }
         ~Axes() { }
         void draw();

      private:
         void drawArrow(double const length, double const radius = -1.0f,
            int const resolution = 24);
   };

} } // end namespace IQmol::Layer

#endif
