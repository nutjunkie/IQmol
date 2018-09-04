#ifndef IQMOL_DATA_NATURALBONDORBITALS_H
#define IQMOL_DATA_NATURALBONDORBITALS_H
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

   /// Data class for molecular orbital information
   class NaturalBondOrbitals : public Orbitals {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::NaturalBondOrbitals; }

         NaturalBondOrbitals(unsigned const nAlpha, unsigned const nBeta, 
            ShellList const& shells, QList<double> const& alphaCoefficients, 
            QList<double> const& alphaOccupancies, QList<double> const& betaCoefficients,  
            QList<double> const& betaOccupancies, QString const& label);

         double alphaOccupancy(unsigned i) const;
         double betaOccupancy(unsigned i) const;
         bool   consistent() const;

         unsigned nAlpha() const { return m_nAlpha; }
         unsigned nBeta()  const { return m_nBeta; }

         QString label(unsigned index, bool alpha = true) const
         {
             QString s(QString::number(index+1));
             unsigned n(alpha ? m_nAlpha : m_nBeta);
             if (index < n)  s += " (occ)";
             return s;
         }

         unsigned labelIndex(bool const alpha) const
         {
            int n(alpha ? m_nAlpha : m_nBeta);
            return std::max(0,n-1);
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

         void dump() const;

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) 
         {
            ar & m_alphaOccupancies;
            ar & m_betaOccupancies;
            ar & m_nAlpha;
            ar & m_nBeta;
         }

         QList<double> m_alphaOccupancies;
         QList<double> m_betaOccupancies;
         unsigned      m_nAlpha;
         unsigned      m_nBeta;
   };

} } // end namespace IQmol::Data

#endif
