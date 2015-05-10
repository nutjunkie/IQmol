#ifndef IQMOL_DATA_SURFACEINFO_H
#define IQMOL_DATA_SURFACEINFO_H
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
#include "SurfaceType.h"


namespace IQmol {
namespace Data {

   class SurfaceInfo : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::SurfaceInfo; }

         SurfaceInfo() { } // for boost::serialization

         SurfaceInfo(SurfaceType const&, unsigned const quality = 3,  
            double const isovalue = 0.02, QColor const& positive = QColor(), 
            QColor const& negative = QColor(), bool isSigned = false,
            bool simplifyMesh = false, double const opacity = 0.999); 
			// Note the opacity is set to slightly less than 1.0 to ensure all
			// surfaces are draw after the Primitives when sorted, however,
			// because it is > 0.99 it won't trigger the transparency overhead
			// in draw()


         SurfaceType& type() { return m_surfaceType; }
         SurfaceType const& type() const { return m_surfaceType; }

         unsigned quality() const { return m_quality; }
         double isovalue() const { return m_isovalue; }
         double opacity() const { return m_opacity; }
         bool isSigned() const;
         bool simplifyMesh() const { return m_simplifyMesh; }
         void setIsSigned(bool const tf) { m_isSigned = tf; }
         QString toString() const;

         QColor const& positiveColor() const { return m_positiveColor; }
         QColor const& negativeColor() const { return m_negativeColor; }

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
         void privateSerialize(Archive& ar, unsigned const)
         {
            ar & m_surfaceType;
            ar & m_quality;
            ar & m_isovalue;
            ar & m_positiveColor;
            ar & m_negativeColor;
            ar & m_isSigned;
            ar & m_simplifyMesh;
            ar & m_opacity;
         }

         SurfaceType m_surfaceType;
         unsigned    m_quality;
         double      m_isovalue;
         QColor      m_positiveColor;
         QColor      m_negativeColor;
         bool        m_isSigned;
         bool        m_simplifyMesh;
         double      m_opacity;
   };

   typedef Data::List<Data::SurfaceInfo> SurfaceInfoList;

} } // end namespace IQmol::Data

#endif
