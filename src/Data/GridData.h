#ifndef IQMOL_DATA_GRID_H
#define IQMOL_DATA_GRID_H
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


namespace IQmol {
namespace Data {

   /// Very basic Data class for data on a grid.  Spacings should be regular in
   /// each of the cartesian coordinates.
   class Grid : public Base {

      friend class boost::serialization::access;

      public:
         Grid(QString const& description = QString(), 
            qglviewer::Vec const& min = qglviewer::Vec(),
            qglviewer::Vec const& max = qglviewer::Vec(),
            int const nx = 0, int const ny = 0, int const nz = 0, 
            QList<double> const& data = QList<double>()) : m_description(description),
            m_min(min), m_max(max), m_nx(nx), m_ny(ny), m_nz(nz), m_data(data) { }

         Type::ID typeID() const { return Type::Grid; }

         void setMin(double const x, double const y, double const z) {
            m_min = qglviewer::Vec(x,y,z);
         }
         void setMax(double const x, double const y, double const z) {
            m_max = qglviewer::Vec(x,y,z);
         }
         void setNum(int const nx, int const ny, int const nz) { 
            m_nx = nx;  m_ny = ny;  m_nz = nz;
         }
         void append(double const value) { m_data.append(value); }

         void getNumberOfPoints(int& nx, int& ny, int& nz) const {
            nx = m_nx;  ny = m_ny;  nz = m_nz;
         }
         qglviewer::Vec const& getMax() const { return m_max; }
         qglviewer::Vec const& getMin() const { return m_min; }
         double getValue(int const i) const { return m_data[i]; }

         bool isValid() const;
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
            ar & m_description;
            ar & m_min;
            ar & m_max;
            ar & m_nx;
            ar & m_ny;
            ar & m_nz;
            ar & m_data;
         }

         QString m_description;
         qglviewer::Vec m_min;
         qglviewer::Vec m_max;
         int m_nx, m_ny, m_nz;
         QList<double> m_data;
   };

   typedef Data::List<Data::Grid> GridList;

} } // end namespace IQmol::Data

#endif
