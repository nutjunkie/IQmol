#ifndef IQMOL_DATA_GRIDDATA_H
#define IQMOL_DATA_GRIDDATA_H
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
#include "GridSize.h"
#include "SurfaceType.h"
#include "Geometry.h"
#include "Matrix.h"


namespace IQmol {
namespace Data {

   class GridSize;

   /// Basic Data class for holding real data on a 3D grid.
   class GridData : public Base {

      using Base::copy;
      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::GridData; }

         GridData(GridSize const&, SurfaceType const&);
         GridData(GridSize const&, SurfaceType const&, QList<double> const& data);
         GridData(GridData const&);

         GridData() { }  // for boost::serialize;

         void getNumberOfPoints(unsigned& nx, unsigned& ny, unsigned& nz) const;

         void getBoundingBox(qglviewer::Vec& min, qglviewer::Vec& max) const;

         void getRange(double& min, double& max);

         double dataSizeInKb() const;

         SurfaceType const& surfaceType() const { return m_surfaceType; }

         void setSurfaceType(SurfaceType::Kind const& kind) { m_surfaceType.setKind(kind); }

         GridSize size() const;

         qglviewer::Vec const& origin() const { return m_origin; }

         qglviewer::Vec const& delta() const {return m_delta; }

         bool saveToCubeFile(QString const& filePath, QStringList const& coordinates,
            bool const invertSign) const;
           
         GridData& operator=(GridData const& that);
         GridData& operator+=(GridData const& that);
         GridData& operator-=(GridData const& that);
         GridData& operator*=(double const that);

         double const& operator()(unsigned const i, unsigned const j, unsigned const k) const
         {
            return m_data[i][j][k];
         }

         double& operator()(unsigned const i, unsigned const j, unsigned const k)
         {
            return m_data[i][j][k];
         }

		 /// Performs a tri-linear interpolation of the grid data at each of 
		 /// the 8 nearest grid points about (x,y,z). Returns 0 outside the 
         /// range of the grid
         double interpolate(double const x, double const y, double const z) const;

		 /// Performs a tri-linear interpolation of the gradient grid data at each
		 /// of the 8 nearest grid points about (x,y,z). Returns (0,0,0) outside 
         /// the range of the grid
         qglviewer::Vec normal(double const x, double const y, double const z) const;

         // computes this = a*this + b*B
         void combine(double const a, double const b, GridData const& B);

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
         void copy(GridData const&);

         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const) 
         {
            ar & m_surfaceType;
            ar & m_origin;
            ar & m_delta;
            ar & m_data;
         }

         SurfaceType m_surfaceType;
         qglviewer::Vec m_origin;
         qglviewer::Vec m_delta;
         Array3D m_data;
   };


   class GridDataList : public Data::List<Data::GridData> {
      public:
         Type::ID typeID() const { return Type::GridDataList; }
   };

} } // end namespace IQmol::Data

#endif
