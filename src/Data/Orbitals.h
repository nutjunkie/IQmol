#ifndef IQMOL_DATA_ORBITALS_H
#define IQMOL_DATA_ORBITALS_H
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

#include "Data.h"
#include "Matrix.h"
#include "ShellList.h"


namespace IQmol {
namespace Data {

   class Orbitals : public Base {

      friend class boost::serialization::access;

      public:
         enum OrbitalType { Undefined = 0, 
                            Canonical, 
                            Localized, 
                            NaturalTransition, 
                            NaturalBond,
                            Dyson
                          };

         static QString toString(OrbitalType const);

         // Required for serialization
         Orbitals() : m_orbitalType(Undefined), m_nBasis(0), m_nOrbitals(0),
            m_restricted(false) { }
            
         // Pass in an empty betaCoefficient list for restricted orbitals
         Orbitals(
            OrbitalType const orbitalType, 
            ShellList const& shellList,
            QList<double> const& alphaCoefficients, 
            QList<double> const& betaCoefficients,
            QString const& title = QString());

         OrbitalType orbitalType() const { return m_orbitalType; }

         unsigned nBasis() const { return m_nBasis; }
         unsigned nOrbitals() const { return m_nOrbitals; }
         bool     restricted() const { return m_restricted; }

         Matrix const& alphaCoefficients() const;
         Matrix const& betaCoefficients() const; 

         ShellList& shellList() { return m_shellList; }

         QString title() const { return m_title; }

         // Returns a label for the given orbital for display.
         // Default label is just the orbital index (watch for off-by-one).
         virtual QString label(unsigned index, bool alpha = true) const;

         // Returns a list of the above labels
         virtual QStringList labels(bool alpha = true) const;

         // Returns the recommended index for displaying in the configurator.
         virtual unsigned labelIndex(bool /* alpha = true */) const { return 0; }

         virtual bool consistent() const;
         virtual unsigned nAlpha() const { return 0; }
         virtual unsigned nBeta() const { return 0; }


         void serialize(InputArchive& ar, unsigned const version = 0)
         {
            privateSerialize(ar, version);
         }
 
         void serialize(OutputArchive& ar, unsigned const version = 0)
         {
            privateSerialize(ar, version);
         }

         void dump() const;

		 // TODO: This is here to enable  archiving of the surfaces associated
		 // with the orbitals, needs testing.
         //SurfaceList& surfaceList() { return m_surfaceList; }
         //void appendSurface(Data::Surface* surfaceData) {
         //   m_surfaceList.append(surfaceData);
         //}


      protected:
         bool areOrthonormal() const;

         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) 
         {
            ar & m_orbitalType;
            ar & m_title;
            ar & m_nBasis;
            ar & m_nOrbitals;
            ar & m_restricted;
            ar & m_alphaCoefficients;
            ar & m_betaCoefficients;
            ar & m_shellList;
            //ar & m_surfaceList;
         }

         OrbitalType m_orbitalType;
         QString     m_title;
         unsigned    m_nBasis;
         unsigned    m_nOrbitals;
         bool        m_restricted;
         ShellList   m_shellList;
         Matrix      m_alphaCoefficients;
         Matrix      m_betaCoefficients;
         //SurfaceList   m_surfaceList;
   };


} } // end namespace IQmol::Data

#endif
