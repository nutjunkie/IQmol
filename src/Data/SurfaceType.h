#ifndef IQMOL_DATA_SURFACETYPE_H
#define IQMOL_DATA_SURFACETYPE_H
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


namespace IQmol {
namespace Data {

   class SurfaceType : public Base {

      friend class boost::serialization::access;

      public:
         enum Kind { Custom = -1, AlphaOrbital, BetaOrbital, TotalDensity, 
            SpinDensity, AlphaDensity, BetaDensity, DensityCombo, CubeData, 
            VanDerWaals, Promolecule, SolventExcluded, SID, ElectrostaticPotential,
            Geminal, Correlation, CustomDensity
// TODO
//            AlphaHole Density, BetaHole Density,
//            AlphaExcitationDensity, BetaExcitationDensity,
//            AlphaAttachmentDensity, BetaAttachmentDensity,
//            AlphaDetachmentDensity, BetaDetachmentDensity
         };

         SurfaceType(Kind const kind = Custom, unsigned index = 0) 
          : m_kind(kind), m_index(index) { }

         QString toString() const;

         Kind const& kind() const { return m_kind; }
         void setKind(Kind const kind) { m_kind = kind; }

         // used for custom surfaces
         QString const& label() const { return m_label; }
         void setLabel(QString const& label) { m_label = label; }

         unsigned const& index() const { return m_index; }
         void setIndex(unsigned const index) { m_index = index; }

         bool isDensity() const;
         bool isRegularDensity() const;
         bool isOrbital() const;
         bool isSigned() const;
         bool isIndexed() const;

         bool operator==(SurfaceType const& that) const;

         bool operator!=(SurfaceType const& that) const
         {
            return !(*this == that);
         }

         bool operator<(SurfaceType const& that) const
         {
            return (m_kind == that.m_kind) ? (m_index < that.m_index)
                                           : (m_kind  < that.m_kind); 
         }

         void serialize(InputArchive& ar, unsigned const /* version  */)
         {
            ar & m_kind;
            ar & m_index;
         }

         void serialize(OutputArchive& ar, unsigned const /* version */)
         {
            ar & m_kind;
            ar & m_index;
         }

         void dump() const;

      private:
         Kind m_kind;
         unsigned m_index;
         QString m_label;
   };

} } // end namespace IQmol::Data

#endif
