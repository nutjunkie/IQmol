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
#include "ShellList.h"
#include "Matrix.h"


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

         Orbitals();
         Orbitals(unsigned const nAlpha, unsigned const nBeta, 
            QList<double> const& alphaCoefficients, QList<double> const& betaCoefficients,
            ShellList const& shells, QString const& label = QString() );


// This should be fixed so that the orbitalType function is declared virtual
         //OrbitalType orbitalType() const = 0;
         void setOrbitalType(OrbitalType const& orbitalType) { m_orbitalType = orbitalType; }
         OrbitalType orbitalType() const { return m_orbitalType; }
         OrbitalType m_orbitalType;

         unsigned nAlpha() const { return m_nAlpha; }
         unsigned nBeta()  const { return m_nBeta; }
         unsigned nBasis() const { return m_nBasis; }
         unsigned nOrbitals() const { return m_nOrbitals; }

         Matrix const& alphaCoefficients() const { return m_alphaCoefficients; }
         Matrix const& betaCoefficients()  const { return m_betaCoefficients; }
         ShellList const& shellList() const { return m_shellList; }

         QString const& label() const { return m_label; }
         void setLabel(QString const& label) { m_label = label; }

         virtual bool consistent() const { return true; }

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
            ar & m_label;
            ar & m_nAlpha;
            ar & m_nBeta;
            ar & m_nBasis;
            ar & m_nOrbitals;
            ar & m_bbMin;
            ar & m_bbMax;
            ar & m_alphaCoefficients;
            ar & m_betaCoefficients;
            ar & m_shellList;
         }

         QString m_label;

         unsigned m_nAlpha;
         unsigned m_nBeta;
         unsigned m_nBasis;
         unsigned m_nOrbitals;
         bool     m_restricted;

         qglviewer::Vec m_bbMin;
         qglviewer::Vec m_bbMax;

         Matrix m_alphaCoefficients;
         Matrix m_betaCoefficients;
         ShellList m_shellList;
   };

} } // end namespace IQmol::Data

#endif
