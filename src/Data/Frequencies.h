#ifndef IQMOL_DATA_FREQUENCIES_H
#define IQMOL_DATA_FREQUENCIES_H
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

#include "VibrationalMode.h"


namespace IQmol {
namespace Data {

   class Frequencies : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::Frequencies; }

         Frequencies() : m_zpve(0.0), m_entropy(0.0), m_enthalpy(0.0), m_haveRaman(false) { }

         void append(VibrationalMode* mode) { m_modes.append(mode); }
         void setThermochemicalData(double const zpve, double const enthalpy, 
            double const entropy);

         void haveRaman(bool tf) { m_haveRaman = tf; }
         bool haveRaman() const { return m_haveRaman; }
         double zpve() const { return m_zpve; }

         double maxFrequency() const;
         double maxIntensity() const;
         double maxRamanIntensity() const;

         void dump() const;
         VibrationalModeList const& modes() const { return m_modes; }

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & m_zpve;
            ar & m_entropy;
            ar & m_enthalpy;
            ar & m_haveRaman;
            ar & m_modes;
         }

         double m_zpve;
         double m_entropy;
         double m_enthalpy;
         bool   m_haveRaman; 
         VibrationalModeList m_modes;
   };

} } // end namespace IQmol::Data

#endif
