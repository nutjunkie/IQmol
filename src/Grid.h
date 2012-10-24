#ifndef IQMOL_GRID_H
#define IQMOL_GRID_H
/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include "IQmol.h"
#include "boost/multi_array.hpp"
#include "QGLViewer/vec.h"
#include <QList>
#include <QObject>
#include <QPair>


typedef boost::multi_array<double, 3> Array3D;
typedef Array3D::index Index;

namespace OpenBabel {
   class OBGridData;
}

namespace IQmol {

   /// A three-dimensional cartesian grid containing scalar data.  Two 
   /// auxiliary classes, DataType and Size, hold information about the 
   /// type of data held by the grid.
   class Grid : public QObject {

      Q_OBJECT

      public: class DataType {
           
         public:
            enum Type { Undefined = -1, AlphaOrbital, BetaOrbital, TotalDensity, 
               SpinDensity, AlphaDensity, BetaDensity, DensityCombo, CubeData, 
               VanDerWaals, Promolecule, SolventExcluded, SID, ElectrostaticPotential };

            DataType(Type const type, int const index = 0);
            DataType(DataType const& that) { copy(that); }

            DataType& operator=(DataType const& that);
            DataType& operator+=(DataType const& that);
            DataType& operator-=(DataType const& that);

            DataType const operator+(DataType const& that) const {
               return DataType(*this) += that;
            }
            DataType const operator-(DataType const& that) const {
               return DataType(*this) -= that;
            }
            bool operator==(DataType const& that) const {
               return ( (m_type == that.m_type) && (m_index == that.m_index) );
            }
            bool operator!=(DataType const& that) const {
               return ( ! (*this == that));
            }
            bool operator<(DataType const& that) const {
               return  (m_type == that.m_type) ? (m_index < that.m_index) 
                                               : (m_type  < that.m_type);
            }

            Type type() const { return m_type; }
            int  index() const { return m_index; }
            QString info() const;
            bool isDensity() const;
            bool isOrbital() const;
            bool isSigned() const;
            bool isDefined() const { return m_type != Undefined; }

         private:
            QString toString() const;
            static int s_densityTypeCount;
            void copy(DataType const& that);
            Type m_type;
            int  m_index;
      };


      public: class Size {

         friend class Grid;

         public:
            Size() : m_xMin(0), m_yMin(0), m_zMin(0), m_xMax(0), m_yMax(0), m_zMax(0),
               m_upsample(false), m_quality(0), m_stepSize(0) { }
            Size(qglviewer::Vec const& boxMin, qglviewer::Vec const& boxMax, 
               int const quality, bool upsample = false);
            Size(qglviewer::Vec const& boxMin, double const stepsize, int const nx,
               int const ny, int const nz, bool upsample = false);
               

            Size(Size const& that) { copy(that); }
            Size& operator=(Size const& that);
            bool operator==(Size const& that) const;
            bool operator<=(Size const& that) const;
            bool operator<(Size const& that) const {
               return ( operator<=(that) && !operator==(that));
            }
            bool operator!=(Size const& that) const {
               return ( ! (*this == that));
            }
            void debugInfo() const;
            bool upsample() const {return m_upsample; }

         protected:
            int m_xMin, m_yMin, m_zMin;
            int m_xMax, m_yMax, m_zMax;
            double stepSize() const;
            bool m_upsample;

         private:
            void copy(Size const& that);
            int m_quality;
            double m_stepSize; // only used if quality is not in the range 0-6
      };

      
      public:  // Grid
         Grid(DataType const& dataType, Size const& size);
         Grid(OpenBabel::OBGridData const& gridData);
         Grid(Grid const& that);
         ~Grid();
         Grid* upsample() const;

         void generateData(Function3D function);
         DataType const& dataType() const { return m_dataType; }
         Size const& size() const { return m_size; }
         double stepSize() const { return m_size.stepSize(); }
         double& operator()(int const ix, int const iy, int const iz) const;
         double  operator()(qglviewer::Vec const& position) const;
         qglviewer::Vec gradient(qglviewer::Vec const& position) const;
         void setValue(int i, int j, int k, double const value);
         QPair<double, double> range() const;
         void debugInfo() const;

         Grid& operator=(Grid const& that);
         Grid& operator+=(Grid const& that);
         Grid& operator-=(Grid const& that);
         Grid& operator*=(double const scale);

         Grid const operator+(Grid const& that) const {
            return Grid(*this) += that;
         }
         Grid const operator-(Grid const& that) const {
            return Grid(*this) -= that;
         }
         bool operator==(Grid const& that) const {
            return ( (m_dataType == that.m_dataType) && (m_size == that.m_size) );
         }
         bool operator!=(Grid const& that) const {
            return ( ! (*this == that));
         }
   
         int xMin() const { return m_size.m_xMin; }
         int yMin() const { return m_size.m_yMin; }
         int zMin() const { return m_size.m_zMin; }
         int xMax() const { return m_size.m_xMax; }
         int yMax() const { return m_size.m_yMax; }
         int zMax() const { return m_size.m_zMax; }


      Q_SIGNALS:
         void progress(double) const;

      private:
         void init();
         void copy(Grid const& that);
         void combine(double const, double const, Grid const&);
         DataType m_dataType;
         Size m_size;
         Array3D* m_data;

         double cubicInterpolate(double p[4], double x) const;
         double bicubicInterpolate(double p[4][4], double x, double y) const;
         double tricubicInterpolate(double p[4][4][4], double x, double y, double z) const;
   };

   typedef QList<Grid*> GridList;

} // end namespace IQmol


#endif
