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

#include "Matrix.h"
#include "Shell.h"
#include "Surface.h"


namespace IQmol {
namespace Data {

   /// Data class for molecular orbital information
   class MolecularOrbitals : public Base {

      friend class boost::serialization::access;

      public:
         MolecularOrbitals() { }
         MolecularOrbitals(unsigned const nAlpha, unsigned const nBeta, 
            QList<double> const& alphaCoefficients, QList<double> const& alphaEnergies,  
            QList<double> const& betaCoefficients, QList<double> const& betaEnergies,
            ShellList const& shells);

         Type::ID typeID() const { return Type::MolecularOrbitals; }

         unsigned nAlpha() const { return m_nAlpha; }
         unsigned nBeta()  const { return m_nBeta; }
         unsigned nBasis() const { return m_nBasis; }
         unsigned nOrbitals() const { return m_nOrbitals; }
         bool     restricted() const { return m_restricted; }

         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max) const 
         {
            min = m_bbMin;
            max = m_bbMax;
         }

         Matrix const& alphaCoefficients() const { return m_alphaCoefficients; }
         Matrix const& betaCoefficients() const { return m_betaCoefficients; }
         ShellList const& shellList() const { return m_shellList; }
         SurfaceList& surfaceList() { return m_surfaceList; }

         void appendSurface(Data::Surface* surfaceData)
         {
            m_surfaceList.append(surfaceData);
         }

         double alphaOrbitalEnergy(unsigned i) const 
         { 
            return (i < m_nOrbitals) ? m_alphaEnergies[i] : 0.0;
         }

         double betaOrbitalEnergy(unsigned i) const 
         { 
            return (i < m_nOrbitals) ? m_betaEnergies[i] : 0.0;
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
            ar & m_nAlpha;
            ar & m_nBeta;
            ar & m_nBasis;
            ar & m_nOrbitals;
            ar & m_restricted;
            ar & m_bbMin;
            ar & m_bbMax;
            ar & m_shellList;
            ar & m_alphaEnergies;
            ar & m_betaEnergies;
            ar & m_alphaCoefficients;
            ar & m_betaCoefficients;
            ar & m_surfaceList;
         }

         void computeBoundingBox();

         unsigned m_nAlpha;
         unsigned m_nBeta;
         unsigned m_nBasis;
         unsigned m_nOrbitals;
         bool     m_restricted;

         qglviewer::Vec m_bbMin;
         qglviewer::Vec m_bbMax;

         QList<double> m_alphaEnergies;
         QList<double> m_betaEnergies;
         Matrix m_alphaCoefficients;
         Matrix m_betaCoefficients;
         ShellList m_shellList;

         SurfaceList m_surfaceList;
   };

} } // end namespace IQmol::Data

#endif
