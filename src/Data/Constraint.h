#ifndef IQMOL_DATA_CONSTRAINT_H
#define IQMOL_DATA_CONSTRAINT_H
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

#include "Data.h"


namespace IQmol {
namespace Data {

   class Constraint : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::Constraint; }

         Constraint() : m_value(0.0) { }
         virtual ~Constraint() { }

         QList<unsigned> const& atomIndices() const { return m_atomIndices; }

         void setValue(double const value) { m_value = value; }
         double value() const { return m_value; }

         virtual void serialize(OutputArchive& ar, unsigned const /* version = 0 */) 
         {
            ar & m_atomIndices;
            ar & m_value;
         }

         virtual void serialize(InputArchive& ar, unsigned const /* version = 0 */) 
         {
            ar & m_atomIndices;
            ar & m_value;
         }

         virtual void dump() const;

      protected:
         virtual void destroy() { }
         QList<unsigned> m_atomIndices;
         double m_value;
   };



   class PositionConstraint : public Constraint {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::PositionConstraint; }

         PositionConstraint() { }

         PositionConstraint(unsigned const atomIndex, qglviewer::Vec const& position);

         qglviewer::Vec const& position() const { return m_position; }

         void serialize(InputArchive& ar, unsigned const /* version = 0 */) 
         {
            ar & m_atomIndex;
            ar & m_position;
         }

         void serialize(OutputArchive& ar, unsigned const /* version = 0 */) 
         {
            ar & m_atomIndex;
            ar & m_position;
         }

         void dump() const;

      private:
         unsigned m_atomIndex;
         qglviewer::Vec m_position;
   };



   class DistanceConstraint : public Constraint {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::DistanceConstraint; }

         DistanceConstraint() { }

         DistanceConstraint(unsigned const atomIndexA, unsigned const atomIndexB,
            double const distance);

         void dump() const;
   };


   class AngleConstraint : public Constraint {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::AngleConstraint; }

         AngleConstraint() { }

         AngleConstraint(unsigned const atomIndexA, unsigned const atomIndexB,
            unsigned const atomIndexC, double const angle);

         void dump() const;
   };


   class TorsionConstraint : public Constraint {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::TorsionConstraint; }

         TorsionConstraint() { }

         TorsionConstraint(unsigned const atomIndexA, unsigned const atomIndexB,
            unsigned const atomIndexC, unsigned const atomIndexD, double const torsion);

         void dump() const;
   };


} } // end namespace IQmol::Data

#endif
