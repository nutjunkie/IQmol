#ifndef IQMOL_DATA_CHARGEMULTIPLICITY_H
#define IQMOL_DATA_CHARGEMULTIPLICITY_H
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

   class ChargeMultiplicity : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::ChargeMultiplicity; }

         ChargeMultiplicity(int const charge = 0, unsigned const electrons = 0, 
            unsigned const multiplicity = 1) : m_charge(charge), m_electrons(electrons),
            m_multiplicity(multiplicity)  { }

         int charge() const { return m_charge; }
         unsigned electrons() const { return m_electrons; }
         unsigned multiplicity() const { return m_multiplicity; }

         void setValues(int const charge, unsigned const electrons,unsigned const multiplicity);
         void setCharge(int const);
         void setElectrons(unsigned const);
         void setMultiplicity(unsigned const);

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }
         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void dump() const;

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const) {
            ar & m_charge;
            ar & m_electrons;
            ar & m_multiplicity;
         }

         int m_charge;
         unsigned m_electrons;
         unsigned m_multiplicity;
   };

} } // end namespace IQmol::Data

#endif
