#ifndef IQMOL_DATA_ATOMICPROPERTY_H
#define IQMOL_DATA_ATOMICPROPERTY_H
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

#include "Data.h"


namespace IQmol {
namespace Data {

   /// Base class for properties of atoms.  This can include things like
   /// charges, masses, chemical shifts and DMA expansions. 
   class AtomicProperty : public Base {

      public:
         Type::ID typeID() const { return Type::AtomicProperty; }
         virtual QString label() const = 0;
   };


   class AtomicSymbol : public AtomicProperty {

      friend class boost::serialization::access;

      public:
         AtomicSymbol(int const Z = 0);
         Type::ID typeID() const { return Type::AtomicSymbol; }

         QString label() const { return m_symbol; }
         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_symbol;
         }
         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_symbol;
         }

      private:
         QString m_symbol;
   };


   class ScalarProperty : public AtomicProperty {

      friend class boost::serialization::access;

      public:
         ScalarProperty() { }

         QString label() const { return QString::number(m_value, 'f', 3); }
         double getValue() const { return m_value; }
         void setValue(double const value) { m_value = value; }
         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_value;
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_value;
         }

      protected:
         double m_value;
   };


   class Mass: public ScalarProperty {
      public:
         Mass(int const Z = 0);
         Type::ID typeID() const { return Type::Mass; }
   };


   class VdwRadius : public ScalarProperty {
      public:
         VdwRadius(int const Z = 0);
         Type::ID typeID() const { return Type::VdwRadius; }
   };


   class MullikenCharge : public ScalarProperty {
      public:
         MullikenCharge(double const q = 0.0) { m_value = q; }
         Type::ID typeID() const { return Type::MullikenCharge; }
   };


   class SpinDensity : public ScalarProperty {
      public:
         SpinDensity(double const spin = 0.0) { m_value = spin; }
         Type::ID typeID() const { return Type::SpinDensity; }
   };


   class StewartCharge : public ScalarProperty {
      public:
         StewartCharge(double const q = 0.0) { m_value = q; }
         Type::ID typeID() const { return Type::StewartCharge; }
   };


   class NmrShiftIsotropic : public ScalarProperty {
      public:
         NmrShiftIsotropic(double const s = 0.0) { m_value = s; }
         Type::ID typeID() const { return Type::NmrShiftIsotropic; }
   };

   class NmrShiftRelative : public ScalarProperty {
      public:
         NmrShiftRelative(double const s = 0.0) { m_value = s; }
         Type::ID typeID() const { return Type::NmrShiftRelative; }
   };




   class AtomColor : public AtomicProperty {

      friend class boost::serialization::access;

      public:
         AtomColor(int const Z = 0);
         Type::ID typeID() const { return Type::AtomColor; }

         QString label() const { return QString(); }
         const double* asArray() const { return m_color; }
         QColor get() const;
         void set(double const red, double const green, double const blue);
         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_color;
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_color;
         }

      protected:
         double m_color[4];
   };


} } // end namespace IQmol::Data


#endif
