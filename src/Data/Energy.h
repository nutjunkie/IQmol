#ifndef IQMOL_DATA_ENERGY_H
#define IQMOL_DATA_ENERGY_H
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

   class Energy : public Base {

      friend class boost::serialization::access;

      public:
         enum Units { Hartree, EV, KJMol, KCalMol, Wavenumber, MHz };

         Energy(double const value = 0.0, Units const units = Hartree) : m_value(value),
            m_units(units), m_label("") { }

         Type::ID typeID() const { return Type::Energy; }

         QString label() const { return m_label; }
         void setLabel(QString const& label) { m_label = label; }
         
         /// Returns the raw value of the energy in the current units
         double value() const { return m_value; }

         /// Returns the energy in the given units
         double value(Units const unit) const;

         /// Returns the current energy units 
         Units units() const { return m_units; }

         void setValue(double const energy, Units const units) {
            m_value = energy;
            m_units = units;
         }

         /// Converts the given unit to a string suitable for printing
         static QString toString(Units const);

         static QString format(double const energy, Units const units, char const fmt = 'g',
            int const precision = 6);

         QString format(char const fmt = 'g', int const precision = 6) const;
         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & m_value;
            ar & m_units;
            ar & m_label;
         }

         // Returns the conversion factor from Hartree
         double conversion(Units const) const;

         double  m_value;
         Units   m_units;
         QString m_label;
   };


   class TotalEnergy : public Energy {
      public:
         TotalEnergy() : Energy() { }
         Type::ID typeID() const { return Type::TotalEnergy; }
   };


   class ScfEnergy : public Energy {
      public:
         Type::ID typeID() const { return Type::ScfEnergy; }
   };


   class ForceFieldEnergy : public Energy {
      public:
         Type::ID typeID() const { return Type::ForceFieldEnergy; }
   };


} } // end namespace IQmol::Data

#endif
