#ifndef IQMOL_CONSTRAINTLAYER_H
#define IQMOL_CONSTRAINTLAYER_H
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

#include "QGLViewer/vec.h"
#include "AtomLayer.h"
#include "IQmol.h"
#include "openbabel/forcefield.h"


class QGLViewer;
class QFontMetrics;

namespace IQmol {

namespace Configurator {
   class Constraint;
   class ScalarConstraint;
   class VectorConstraint;
}

namespace Layer {

   /// Base class for Layers representing geometric constraints on Atoms in a
   /// Molecule.
   class Constraint : public GLObject {

      Q_OBJECT
      friend class Configurator::ScalarConstraint;
      friend class Configurator::VectorConstraint;

      public:
		 /// Specifies the type of constraint.  Note that for the time being a
		 /// Position constraint corresponds to all coordinates being fixed, i.e.
		 /// you can't fix just the x coordinate.
         enum Type { Invalid, Position, Distance, Angle, Torsion };

		 /// The type of constraint is determined by the number of Atoms.
		 Constraint();
		 Constraint(AtomList const& atoms);
         Constraint(Constraint const& that) : GLObject() { copy(that); }
         ~Constraint();

         Type constraintType() const { return m_type; }
         void draw();
         void drawFast() { }
         void drawSelected() { }

         bool isValid() const { return m_type != Invalid; }
         AtomList atomList() const { return m_atoms; }

         /// Passes the return value from the Configurator dialog 
         bool accepted() const { return m_accepted; }

		 /// Determines if the constraint is considered to be satisfied.  Note
		 /// that this function contains some hard-wired thresholds for satisfaction.
         bool satisfied() const;

         QString message() const { return m_mesg; }

         void setTargetValue(double const value);
         void setTargetValue(unsigned char const axis, qglviewer::Vec const& position);
         double targetValue() const { return m_targetValue; }

		 /// Returns the displacement required to move the constrained atom to
		 /// a location that fits the constrained axes.
         qglviewer::Vec targetDisplacement() const;

		 /// Returns the alpha value of the transparent part of the Constraint
		 /// Layer.  All Constraints have the same (static) value for transparency.
         double getAlpha() const { return s_alpha; }

		 /// Two constraints are considered equal if they involve the same
		 /// atoms in either the same or reverse order.  The target values do
		 /// not have to match.
         bool operator==(Constraint const& that) const;
         Constraint& operator=(Constraint const&); 

         /// Adds the current constraint to the OBFFConstraints object.
         void addTo(OpenBabel::OBFFConstraints&) const;

         QString formatQChem() const;


      Q_SIGNALS:
         /// Signal sent when one of the atoms is destroyed
         void invalid();

      public Q_SLOTS:
         void configure();

      protected:
         int precision() const;
         double minValue() const;
         double maxValue() const;

         AtomList m_atoms;
         QString  m_mesg;

      private:
         void setAtomList(AtomList const&);
         void copy(Constraint const& that);
         void drawPosition();
         void drawDistance();
         void drawAngle();
         void drawTorsion();

         /// The alpha value of the transparent bits of the constraint
         static GLfloat const s_alpha;
         static GLfloat const s_tubeRadius;
         static GLfloat const s_tubeResolution;
         static GLfloat const s_satisfiedColor[];
         static GLfloat const s_unsatisfiedColor[];

		 /// The Constraint Layer behaves a little differently from other
		 /// Layers in that we have different Configurators for position and
		 /// everything else.
         Configurator::Constraint* m_configurator;

         Type m_type;
         double m_targetValue;
         qglviewer::Vec m_targetPosition;
         unsigned char m_axes;
         bool m_accepted;  // used to pass the return from the Configurator
         int m_precision;
         double m_minValue;
         double m_maxValue;
   };

} // end namespace Layer

typedef QList<Layer::Constraint*> ConstraintList;

} // end namespace IQmol

#endif
