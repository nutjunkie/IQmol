#ifndef IQMOL_DATA_CANONICALORBITALS_H
#define IQMOL_DATA_CANONICALORBITALS_H
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
#include "Density.h"


namespace IQmol {
namespace Data {

   /// Data class for molecular orbital information
   class CanonicalOrbitals : public Orbitals {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::CanonicalOrbitals; }

         CanonicalOrbitals(unsigned const nAlpha, unsigned const nBeta, 
            ShellList const& shells, QList<double> const& alphaCoefficients, 
            QList<double> const& alphaEnergies, QList<double> const& betaCoefficients,  
            QList<double> const& betaEnergies, QString const& label);

         // TODO: Not sure why this is here or even required, perhaps move it to Orbitals.
         SurfaceList& surfaceList() { return m_surfaceList; }

         void appendSurface(Data::Surface* surfaceData) {
            m_surfaceList.append(surfaceData);
         }

         DensityList const& densityList() const { return m_densityList; }
         void appendDensities(Data::DensityList const& densities) {
            m_densityList << densities;
         }

         double alphaOrbitalEnergy(unsigned i) const;
         double betaOrbitalEnergy(unsigned i) const;
         bool   consistent() const;

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

         void dump() const;

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) 
         {
            ar & m_alphaEnergies;
            ar & m_betaEnergies;
            ar & m_surfaceList;
         }

         QList<double> m_alphaEnergies;
         QList<double> m_betaEnergies;
         SurfaceList   m_surfaceList;
         DensityList   m_densityList;
   };

} } // end namespace IQmol::Data

#endif
