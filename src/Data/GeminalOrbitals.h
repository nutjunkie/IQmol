#ifndef IQMOL_DATA_GEMINALORBITALS_H
#define IQMOL_DATA_GEMINALORBITALS_H
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
#include "ShellList.h"
#include "Surface.h"


namespace IQmol {
namespace Data {

   /// Data class for geminal orbital information
   class GeminalOrbitals : public Base {

      friend class boost::serialization::access;

      public:
         GeminalOrbitals() { }
         GeminalOrbitals(unsigned const nAlpha, unsigned const nBeta, unsigned const nBasis,
            QList<double> const& alphaCoefficients, QList<double> const& betaCoefficients, 
            QList<double> const& geminalEnergies, QList<double> const& geminalCoefficients, 
            QList<int> const& geminalMoMap, ShellList const& shells);
   
         Type::ID typeID() const { return Type::GeminalOrbitals; }

         unsigned nAlpha()    const { return m_nAlpha; }
         unsigned nBeta()     const { return m_nBeta; }
         unsigned nBasis()    const { return m_nBasis; }
         unsigned nOrbitals() const { return m_nOrbitals; }
         unsigned nGeminals() const { return m_nGeminals; }
         QList<unsigned> const& geminalOrbitalLimits() const { return m_geminalOrbitalLimits ; }

         double geminalOrbitalEnergy(unsigned i) const 
         { 
            return ((int)i < m_geminalEnergies.length()) ? m_geminalEnergies[i] : 0.0;
         }

         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max) const 
         {
            min = m_bbMin;
            max = m_bbMax;
         }

         Matrix const& alphaCoefficients() const { return m_alphaCoefficients; }
         Matrix const& betaCoefficients() const { return m_betaCoefficients; }
         QList<double> const& geminalCoefficients() const { return  m_geminalCoefficients;}
         QList<int> const& geminalMoMap() const { return  m_geminalMoMap;}
         ShellList const& shellList() const { return m_shellList; }
         SurfaceList& surfaceList() { return m_surfaceList; }

         void appendSurface(Data::Surface* surfaceData)
         {
            m_surfaceList.append(surfaceData);
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
            ar & m_nGeminals;

            ar & m_bbMin;
            ar & m_bbMax;

            ar & m_geminalEnergies;
            ar & m_geminalCoefficients; 
            ar & m_geminalMoMap; 
            ar & m_shellList;
            ar & m_alphaCoefficients;
            ar & m_betaCoefficients; 
            ar & m_surfaceList;
         }

         void computeBoundingBox();
         void computeGeminalLimits();

         unsigned m_nAlpha;
         unsigned m_nBeta;
         unsigned m_nBasis;
         unsigned m_nOrbitals;
         unsigned m_nGeminals;

         qglviewer::Vec m_bbMin;
         qglviewer::Vec m_bbMax;

         QList<double> m_geminalEnergies;
         QList<double> m_geminalCoefficients;
         QList<int>    m_geminalMoMap;
         ShellList     m_shellList;
         QList<unsigned> m_geminalOrbitalLimits;

         Matrix m_alphaCoefficients;
         Matrix m_betaCoefficients;

         SurfaceList m_surfaceList;
   };

} } // end namespace IQmol::Data

#endif
