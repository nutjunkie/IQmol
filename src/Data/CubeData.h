#ifndef IQMOL_DATA_CUBEDATA_H
#define IQMOL_DATA_CUBEDATA_H
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

#include "GridData.h"
#include "Geometry.h"


namespace IQmol {
namespace Data {

   class CubeData : public GridData {

      public:
         Type::ID typeID() const { return Type::CubeData; }

         CubeData(Geometry const& geometry, GridSize const& size, SurfaceType const& type, 
           QList<double> const& data) : GridData(size, type, data), m_geometry(geometry) { }

         CubeData() { }  // for boost::serialize;

         Geometry const& geometry() const { return m_geometry; }

         QString const& label() const { return m_label; }

         void setLabel(QString const& label) { m_label = label; }

         void serialize(InputArchive& ar, unsigned const version = 0) 
         {
            GridData::serialize(ar, version);
            ar & m_geometry;
            ar & m_label;
         }

         void serialize(OutputArchive& ar, unsigned const version = 0) 
         {
            GridData::serialize(ar, version);
            ar & m_geometry;
            ar & m_label;
         }

      private:
         Geometry m_geometry;
         QString  m_label;
   };

} } // end namespace IQmol::Data

#endif
