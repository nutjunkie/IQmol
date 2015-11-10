#ifndef IQMOL_DATA_NMR_H
#define IQMOL_DATA_NMR_H
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

#include "DataList.h"
#include "Matrix.h"
#include "NmrReference.h"


namespace IQmol {
namespace Data {

   class Nmr: public Base {

      friend class boost::serialization::access;

      public:
         Nmr();

         Type::ID typeID() const { return Type::Nmr; }

         NmrReference& reference() { return m_reference; }

         void setIsotropicShifts(QList<double> const& shifts) {
            m_isotropicShifts = shifts;
         }

         QList<double> const& isotropicShifts() const { return m_isotropicShifts; }

         void setRelativeShifts(QList<double> const& shifts) {
            m_relativeShifts = shifts;
         }

         QList<double> const& relativeShifts() const { return m_relativeShifts; }

         void setAtomLabels(QStringList const& labels) {
            m_atomLabels = labels;
         }

         QStringList const& atomLabels() const { return m_atomLabels; }

         void setIsotropicCouplings(Matrix const& couplings) {
            m_isotropicCouplings = couplings;
         }

         bool haveCouplings();
         bool haveRelativeShifts();

         void dump() const;

         void serialize(InputArchive& ar, unsigned int const /*version*/) 
         {
            ar & m_isotropicShifts;
            ar & m_relativeShifts;
            ar & m_reference;
         }

         void serialize(OutputArchive& ar, unsigned int const /*version*/) 
         {
            ar & m_isotropicShifts;
            ar & m_relativeShifts;
            ar & m_reference;
         }

      private:
         QStringList   m_atomLabels;
         QList<double> m_isotropicShifts;
         QList<double> m_relativeShifts;
         Matrix        m_isotropicCouplings;
         NmrReference  m_reference;
   };

} } // end namespace IQmol::Data

#endif
