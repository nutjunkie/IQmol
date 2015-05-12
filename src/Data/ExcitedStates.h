#ifndef IQMOL_DATA_EXCITEDSTATES_H
#define IQMOL_DATA_EXCITEDSTATES_H
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

#include "ElectronicTransition.h"


namespace IQmol {
namespace Data {

   class ExcitedStates : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::ExcitedStates; }

         void append(ElectronicTransition* transition) { m_transitions.append(transition); }

         double maxEnergy() const;
         double maxIntensity() const;

         void dump() const;
         ElectronicTransitionList const& transitions() const { return m_transitions; }

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & m_transitions;
         }

         ElectronicTransitionList m_transitions;
   };

} } // end namespace IQmol::Data

#endif
