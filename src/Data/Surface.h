#ifndef IQMOL_DATA_SURFACE_H
#define IQMOL_DATA_SURFACE_H
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

#include "DataList.h"
#include "Mesh.h"
#include "Function.h"


namespace IQmol {
namespace Data {

   class SurfaceInfo;

   class Surface : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::Surface; }

         Surface(SurfaceInfo const&);
         Surface(Mesh const&);
         Surface() { }  // for serialization

         void computeSurfaceProperty(Function3D const&);
         void computeIndexProperty();
         void clearSurfaceProperty();
         void getPropertyRange(double& min, double& max) const;

         QList<QColor> const& colors() const { return m_colors; }
         void setColors(QList<QColor> const& colors);

         double opacity() const { return m_opacity; }
         void setOpacity(double const opacity ) { m_opacity = opacity; }
         bool isVisible() const { return m_isVisible; }
         void setVisibility(bool const tf) { m_isVisible = tf; }

         bool hasProperty() const;
         bool propertyIsSigned() const;
         bool isSigned() const { return m_isSigned; }
         QString const& description() const { return m_description; }
         void setDescription(QString const& description) { m_description = description; }

         Mesh& meshPositive() { return m_meshPositive; }
         Mesh& meshNegative() { return m_meshNegative; }

         double area() const;

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
            ar & m_description;
            ar & m_colors;
            ar & m_opacity;
            ar & m_isSigned;
            ar & m_isVisible;
            ar & m_meshPositive;
            ar & m_meshNegative;

            computeSurfacePropertyRange();
         }

         void computeSurfacePropertyRange();

         QString m_description;
         QList<QColor> m_colors;
         double m_opacity;
         bool   m_isSigned;
         bool   m_isVisible;
         Mesh   m_meshPositive;
         Mesh   m_meshNegative;

         // These don't need to be serialized
         double m_min;
         double m_max;
   };

   class SurfaceList : public Data::List<Data::Surface>  {
         Type::ID typeID() const { return Type::SurfaceList; }
   };

} } // end namespace IQmol::Data

#endif
