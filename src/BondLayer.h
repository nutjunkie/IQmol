#ifndef IQMOL_BONDLAYER_H
#define IQMOL_BONDLAYER_H
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

#include "PrimitiveLayer.h"
#include "DataLayer.h"


class QColor;

namespace IQmol {
namespace Layer {

   class Atom;

   /// Concrete Primitive class tht represents a Bond.
   class Bond : public Primitive {

      Q_OBJECT

      friend class Molecule;

      public:
         Bond(Atom* begin, Atom* end);
         ~Bond() { }

         // Note the default cameraPosition will only work for single bonds
         void draw(qglviewer::Vec const& cameraPosition);
         void drawSelected(qglviewer::Vec const& cameraPosition);
         void setOrder(int const order) { m_order = order; }
         void setIndex(int const index);

         int getOrder() const { return m_order; }
         Atom* beginAtom() { return m_begin; }
         Atom* endAtom() { return m_end; }
         double length();

      protected:
         Atom* m_begin;
         Atom* m_end;
         int m_order;

      private:
         void updateOrientation();
         void drawPrivate(qglviewer::Vec const& cameraPosition, bool selected);
         void drawBallsAndSticks(qglviewer::Vec const& cameraPosition, bool selected);
         void drawTubes(qglviewer::Vec const& cameraPosition, bool selected);
         void drawWireFrame(qglviewer::Vec const& cameraPosition, bool selected);

         // Static Data
         static GLfloat s_defaultColor[];        // Gray bonds for BallsAndSticks
         static GLfloat s_radiusBallsAndSticks;  // Default tube radius in angstroms
         static GLfloat s_radiusTubes;           // Default tube radius in angstroms
         static GLfloat s_radiusWireFrame;       // Default wire frame radius in pixels
         static qglviewer::Vec s_zAxis;
   };


   class Bonds: public Data {

      Q_OBJECT;

      public:
         Bonds() : Data("Bonds") { }
         QList<Bond*> getBonds() { return findLayers<Bond>(Children); }
   };


} 

typedef QList<Layer::Bond*> BondList;

} // end namespace IQmol::Layer


#endif
