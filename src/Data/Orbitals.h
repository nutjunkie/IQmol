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

   /// Base class for different orbital types
   class Orbitals : public Base {

      friend class boost::serialization::access;

      public:
         enum OrbitalType { Undefined = 0, 
                            Canonical, 
                            Localized, 
                            NaturalTransition, 
                            NaturalBond 
                          };

         static QString toString(OrbitalType const);

         // Required for serialization
         Orbitals() : m_orbitalType(Undefined), m_nAlpha(0), m_nBeta(0), m_nBasis(0), 
            m_nOrbitals(0), m_restricted(false) { }

         // Pass in an empty betaCoefficient list for restricted orbitals
         Orbitals(OrbitalType const orbitalType, unsigned const nAlpha, 
            unsigned const nBeta, ShellList const& shells,
            QList<double> const& alphaCoefficients, QList<double> const& betaCoefficients,
            QString const& label = QString());

         OrbitalType orbitalType() const { return m_orbitalType; }
         // TODO: get rid of this with proper subclassing of NTOs/NBOs 
         void setOrbitalType(OrbitalType orbitalType) { m_orbitalType = orbitalType; }

         unsigned nAlpha() const { return m_nAlpha; }
         unsigned nBeta()  const { return m_nBeta; }
         unsigned nBasis() const { return m_nBasis; }
         unsigned nOrbitals() const { return m_nOrbitals; }
         bool     restricted() const { return m_restricted; }

         Matrix const& alphaCoefficients() const;
         Matrix const& betaCoefficients() const; 
         ShellList& shellList();

         QString const& label() const { return m_label; }
         void setLabel(QString const& label) { m_label = label; }

         virtual bool consistent() const;

         // TODO: remove this is actually part of ShellList
         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max) const
         {
            min = m_bbMin;
            max = m_bbMax;
         }

         void serialize(InputArchive& ar, unsigned const version = 0)
         {
            privateSerialize(ar, version);
         }
 
         void serialize(OutputArchive& ar, unsigned const version = 0)
         {
            privateSerialize(ar, version);
         }

         void dump() const;


      protected:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) 
         {
            ar & m_orbitalType;
            ar & m_label;
            ar & m_nAlpha;
            ar & m_nBeta;
            ar & m_nBasis;
            ar & m_nOrbitals;
            ar & m_restricted;
            ar & m_alphaCoefficients;
            ar & m_betaCoefficients;
            ar & m_shellList;
            ar & m_bbMin;
            ar & m_bbMax;
         }

         OrbitalType m_orbitalType;
         unsigned m_nAlpha;
         unsigned m_nBeta;
         unsigned m_nBasis;
         unsigned m_nOrbitals;
         bool     m_restricted;

         QString   m_label;
         ShellList m_shellList;    // TODO: move out
         Matrix    m_alphaCoefficients;
         Matrix    m_betaCoefficients;

         qglviewer::Vec m_bbMin, m_bbMax; // bounding box, TODO: move out

   };

} } // end namespace IQmol::Data

#endif
