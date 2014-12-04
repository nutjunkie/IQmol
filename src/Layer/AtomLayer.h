#ifndef IQMOL_ATOMLAYER_H
#define IQMOL_ATOMLAYER_H
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

#include "PrimitiveLayer.h"
#include <QMap>


class QFontMetrics;
class QColor;;

namespace OpenBabel {
   class OBAtom;
}

namespace IQmol {

class Viewer;

namespace Layer {

   /// Primitive class that represents an Atom.
   class Atom : public Primitive {

      Q_OBJECT

      friend class Molecule;
      friend class Bond;
      friend class Constraint;

      public:
         enum LabelType { None, Index, Element, Charge, Mass, Spin, Reindex, NmrShift };
         //enum ChargeType { Unknown = -1, Gasteiger, Sanderson, Mulliken };

         Atom(int Z);
         ~Atom() { }

         static double distance(Atom* A, Atom* B);
         static double angle(Atom* A, Atom* B, Atom* C);
         static double torsion(Atom* A, Atom* B, Atom* C, Atom* D);

         static void setVibrationVectorColor(QColor const& color);
         static void setVibrationVectorScale(GLfloat const& scale) { 
            s_vibrationVectorScale = scale; 
         }
         static void setVibrationAmplitude(GLfloat const& amplitude) {
            s_vibrationAmplitude  = amplitude; 
         }
         static void setDisplayVibrationVector(bool const tf) {
            s_vibrationDisplayVector= tf; 
         }

         void draw();
         void drawFast();
         void drawSelected();
         void drawLabel(Viewer& viewer, LabelType const, QFontMetrics&);

         void setAtomicNumber(unsigned int const Z);
         void setSmallerHydrogens(bool const tf) { m_smallerHydrogens = tf; }
         void setCharge(double const charge) {m_charge = charge; }
         void setSpinDensity(double const spin) {m_spin = spin; }
         void setIndex(int const index);
         void setReorderIndex(int const reorderIndex) { m_reorderIndex = reorderIndex; }
         void setNmrShift(double const nmrShift) {m_nmrShift = nmrShift; }

         int getAtomicNumber() const { return m_atomicNumber; }
         QString getAtomicSymbol() const { return m_symbol; }
         double getCharge() const { return m_charge; }
         double getMass() const { return m_mass; }
         double getSpin() const { return m_spin; }
         int getIndex() const { return m_index; }
         int getReorderIndex() const { return m_reorderIndex; }
         double getRadius(bool const selected);

      public Q_SLOTS:
         void setDisplacement(qglviewer::Vec const& displacement) { 
            m_displacement = displacement; 
         }

      protected:
         unsigned int m_atomicNumber;
         qglviewer::Vec displacedPosition() {
            return getPosition() + s_vibrationAmplitude * m_displacement;
         }
         GLfloat m_color[4];
         QString symbol() { return m_symbol; }
         double getRadius() { return getRadius(isSelected()); }

      private:
         QString getLabel(LabelType const type);
         void drawPrivate(bool selected);
         void drawDisplacement(); 
         void drawArrow(const qglviewer::Vec& from, const qglviewer::Vec& to);
         void drawArrow(float length);

         // Static Data
         static GLfloat s_radiusBallsAndSticks; // Default tube radius in angstroms
         static GLfloat s_radiusWireFrame;      // Default wire frame radius in pixels
         static GLfloat s_radiusTubes;          // Default tube radius in angstroms
         static GLfloat s_vibrationAmplitude;   // determines the animation amplitude
         static GLfloat s_vibrationVectorScale; // scales all the lengths of the vectors
         static GLfloat s_vibrationVectorColor[];
         static bool    s_vibrationDisplayVector; 
         static bool    s_vibrationColorInitialized;

         // Data
         double  m_mass;
         int     m_index;
         double  m_charge;
         double  m_spin;
         double  m_nmrShift;
         GLfloat m_vdwRadius;
         QString m_symbol;
         bool    m_smallerHydrogens;
         int     m_reorderIndex;
         qglviewer::Vec m_displacement;
   };

   
   class Atoms : public Base {

      Q_OBJECT;

      public:
         Atoms() : Base("Atoms") { }
         QList<Atom*> getAtoms() { return findLayers<Atom>(Children | Nested); }
   };
   

} // end namespace Layer

typedef QList<Layer::Atom*> AtomList;

} // end namespace IQmol

#endif
