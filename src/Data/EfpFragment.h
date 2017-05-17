#ifndef IQMOL_DATA_EFPFRAGMENT_H
#define IQMOL_DATA_EFPFRAGMENT_H
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
#include "Geometry.h"
#include "QGLViewer/quaternion.h"


using qglviewer::Vec;
using qglviewer::Quaternion;

namespace IQmol {
namespace Data {

   /// Represents a single fragment.  Note that the geometry and parameters are
   /// not stored in the objects as this would lead to duplicated data.
   //  Rather, they need to be obtained from the EfpFragmentLibrary
   class EfpFragment : public Base {

      friend class boost::serialization::access;

      public:
         EfpFragment(QString const& name = QString(), Vec const& position = Vec(),
            double const alpha = 0.0, double const beta = 0.0, double const gamma = 0.0);

         Type::ID typeID() const { return Type::EfpFragment; }
         QString const& name() const { return m_name; }
         Vec const& position() const { return m_position; }
         Quaternion const& orientation() const { return m_orientation; }

         void setName(QString const& name) { m_name = name; }
         void setPosition(Vec const& position) { m_position = position; };
         void setOrientation(Quaternion const& orientation) { m_orientation = orientation; }
         void setEulerAngles(double const alpha, double const beta, double const gamma);
         bool align(QList<Vec> const&);

         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned int const) {
            ar & m_name;
            ar & m_position;
            ar & m_orientation;
         }

         QString m_name;
         Vec m_position;
         Quaternion m_orientation;
   };

   class EfpFragmentList : public Data::List<Data::EfpFragment> { 
      public:
         Type::ID typeID() const { return Type::EfpFragmentList; }
   };

} } // end namespace IQmol::Data

#endif
