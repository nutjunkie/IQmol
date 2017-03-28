#ifndef IQMOL_DATA_MOLECULARORBITALS_H
#define IQMOL_DATA_MOLECULARORBITALS_H
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
#include "Surface.h"


namespace IQmol {
namespace Data {

   /// Data class for molecular orbital information
   class MolecularOrbitals : public Orbitals {

      friend class boost::serialization::access;

      public:
         MolecularOrbitals() : Orbitals() { }

         MolecularOrbitals(unsigned const nAlpha, unsigned const nBeta, unsigned const nBasis,
            QList<double> const& alphaCoefficients, QList<double> const& alphaEnergies,  
            QList<double> const& betaCoefficients,  QList<double> const& betaEnergies,
            ShellList const& shells);

         Type::ID typeID() const { return Type::MolecularOrbitals; }

         bool restricted() const { return m_restricted; }
         SurfaceList& surfaceList() { return m_surfaceList; }

         void appendSurface(Data::Surface* surfaceData)
         {
            m_surfaceList.append(surfaceData);
         }

         double alphaOrbitalEnergy(unsigned i) const 
         { 
            // Note the energies array may be empty if the orbitals are localized
            return (i < m_alphaEnergies.size()) ? m_alphaEnergies[i] : 0.0;
         }

         double betaOrbitalEnergy(unsigned i) const 
         { 
            // Note the energies array may be empty if the orbitals are localized
            return (i < m_betaEnergies.size()) ? m_betaEnergies[i] : 0.0;
         }

         bool consistent() const;

         void serialize(InputArchive& ar, unsigned const version = 0) 
         {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned const version = 0) 
         {
            privateSerialize(ar, version);
         }

         void dump() const;

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) 
         {
            ar & boost::serialization::base_object<Orbitals>(*this);
            ar & m_restricted;
            ar & m_alphaEnergies;
            ar & m_betaEnergies;
            ar & m_surfaceList;
         }

         bool m_restricted;
         QList<double> m_alphaEnergies;
         QList<double> m_betaEnergies;
         SurfaceList m_surfaceList;
   };

} } // end namespace IQmol::Data

#endif
