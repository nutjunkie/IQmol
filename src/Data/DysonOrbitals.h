#ifndef IQMOL_DATA_DYSONORBITALS_H
#define IQMOL_DATA_DYSONORBITALS_H
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

#include "Orbitals.h"


namespace IQmol {
namespace Data {

   class DysonOrbitals : public Orbitals {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::DysonOrbitals; }

         DysonOrbitals(
            ShellList const& shellList, 
            QList<double> const& leftCoefficients, 
            QList<double> const& rightCoefficients,  
            QList<double> const& excitationEnergies, 
            QStringList const&  names);
            
         QStringList labels(bool const) const
         {
            return m_labels;
         }

         QString label(unsigned index) const
         {
            return ((int)index < m_labels.size() ? m_labels[index] : QString("undef"));
         }


         double excitationEnergy(unsigned index) const 
         {
            return ((int)index < m_excitationEnergies.size() ? 
                                 m_excitationEnergies[index] : 0.0);
         }

         void serialize(InputArchive& ar, unsigned const version = 0) 
         {
            Orbitals::serialize(ar, version);
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned const version = 0) 
         {
            Orbitals::serialize(ar, version);
            privateSerialize(ar, version);
         }


      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) 
         {
            ar & m_excitationEnergies;
            ar & m_labels;
         }

         QList<double> m_excitationEnergies;
         QList<QString> m_labels;
   };

} } // end namespace IQmol::Data

#endif
