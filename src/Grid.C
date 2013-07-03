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

#include "IQmol.h"
#include "Grid.h"
#include "GridData.h"
#include "MoleculeLayer.h"
#include "QsLog.h"
#include <cmath>
#include <QApplication>
#include <openbabel/griddata.h>
#include <QTime>


using namespace qglviewer;

namespace IQmol {


// ----- Grid::DataType -----
int Grid::DataType::s_densityTypeCount = 0;

Grid::DataType::DataType(Grid::DataType::Type const type, int const index)
 : m_type(type), m_index(index)
{
   if (index < 0)  m_index = (s_densityTypeCount++);
}


Grid::DataType& Grid::DataType::operator=(Grid::DataType const& that)
{
   if (this != &that) copy(that);
   return *this;
}


Grid::DataType& Grid::DataType::operator+=(Grid::DataType const& that)
{
   if ( (m_type == AlphaDensity && that.m_type == BetaDensity) ||
        (m_type == BetaDensity  && that.m_type == AlphaDensity)) {
      m_type = TotalDensity;
   } else if (isDensity() && that.isDensity()) {
      m_type = DensityCombo;
      m_index = (s_densityTypeCount++);
   } else if (m_type == CubeData && that.m_type == CubeData) {
      m_type = CubeData;
   }else {
      m_type = Undefined;
   }

   return *this;
}


Grid::DataType& Grid::DataType::operator-=(Grid::DataType const& that)
{
   if ( (m_type == AlphaDensity && that.m_type == BetaDensity)) {
      m_type = SpinDensity;
   } else if (isDensity() && that.isDensity()) {
      m_type = DensityCombo;
      m_index = (s_densityTypeCount++);
   } else if (m_type == CubeData && that.m_type == CubeData) {
      m_type = CubeData;
   }else {
      m_type = Undefined;
   }

   return *this;
}



bool Grid::DataType::isDensity() const
{
   return (m_type == AlphaDensity) || (m_type == BetaDensity) || 
          (m_type == TotalDensity) || (m_type == SpinDensity) || 
          (m_type == DensityCombo);
}


bool Grid::DataType::isSigned() const
{
   return (m_type == AlphaOrbital) || (m_type == BetaOrbital) || 
          (m_type == SpinDensity)  || (m_type == DensityCombo) || 
          (m_type == CubeData);
}


bool Grid::DataType::isOrbital() const
{
   return (m_type == AlphaOrbital) || (m_type == BetaOrbital);
}


void Grid::DataType::copy(Grid::DataType const& that)
{
   m_type  = that.m_type;
   m_index = that.m_index;
}


QString Grid::DataType::info() const
{
   QString info(toString());
   if (m_index > 0) info += " " + QString::number(m_index);
   return  info;
}


QString Grid::DataType::toString() const
{
   QString label; 
   switch (m_type) {
      case Undefined:              label = "Undefined";               break;
      case AlphaOrbital:           label = "Alpha";                   break;
      case BetaOrbital:            label = "Beta";                    break;
      case TotalDensity:           label = "Total Density";           break;
      case SpinDensity:            label = "Spin Density";            break;
      case AlphaDensity:           label = "Alpha Density";           break;
      case BetaDensity:            label = "Beta Density";            break;
      case DensityCombo:           label = "User Defined Density";    break;
      case CubeData:               label = "Cube Data";               break;
      case VanDerWaals:            label = "van der Waals";           break;
      case Promolecule:            label = "Promolecule";             break;
      case SolventExcluded:        label = "Solvent Excluded";        break;
      case SID:                    label = "SID";                     break;
      case ElectrostaticPotential: label = "Electrostatic Potential"; break;
   }
   return label;
}



// ----- Grid::Size -----

Grid::Size::Size(Vec const& boxMin, Vec const& boxMax, int const quality, bool upsample)
{
   m_quality = quality;
   m_stepSize = stepSize();
   m_upsample = upsample;
   double mini, maxi;

   mini = std::min(boxMin.x, boxMax.x);
   maxi = std::max(boxMin.x, boxMax.x);
   m_xMin = std::floor(mini / m_stepSize);
   m_xMax = std::ceil (maxi / m_stepSize);

   mini = std::min(boxMin.y, boxMax.y);
   maxi = std::max(boxMin.y, boxMax.y);
   m_yMin = std::floor(mini / m_stepSize);
   m_yMax = std::ceil (maxi / m_stepSize);

   mini = std::min(boxMin.z, boxMax.z);
   maxi = std::max(boxMin.z, boxMax.z);
   m_zMin = std::floor(mini / m_stepSize);
   m_zMax = std::ceil (maxi / m_stepSize);
}


Grid::Size::Size(Vec const& boxMin, double const stepSize, 
   int const nx, int const ny, int const nz, bool upsample)
{
   m_quality  = -1;
   m_stepSize = stepSize;
   m_upsample = upsample;

   m_xMin = round(boxMin.x / m_stepSize);
   m_yMin = round(boxMin.y / m_stepSize);
   m_zMin = round(boxMin.z / m_stepSize);
   m_xMax = m_xMin + nx - 1;
   m_yMax = m_yMin + ny - 1;
   m_zMax = m_zMin + nz - 1;
}


Vec Grid::Size::bbMin() const
{
   double h(stepSize());
   return Vec(h*m_xMin, h*m_yMin, h*m_zMin);
}

Vec Grid::Size::bbMax() const
{
   double h(stepSize());
   return Vec(h*m_xMax, h*m_yMax, h*m_zMax);
}



               
Grid::Size& Grid::Size::operator=(Grid::Size const& that)
{
   if (this != &that) copy(that);
   return *this;
}


bool Grid::Size::operator==(Grid::Size const& that) const
{
   return m_quality == that.m_quality &&
     m_xMin == that.m_xMin  &&  m_yMin == that.m_yMin  &&  m_zMin == that.m_zMin &&
     m_xMax == that.m_xMax  &&  m_yMax == that.m_yMax  &&  m_zMax == that.m_zMax;
}


bool Grid::Size::operator<=(Grid::Size const& that) const
{
   double s(stepSize());
   double t(that.stepSize());

   return s >= t &&
     m_xMin*s >= that.m_xMin*t  &&  m_yMin*s >= that.m_yMin*t  &&  m_zMin*s >= that.m_zMin*t &&
     m_xMax*s <= that.m_xMax*t  &&  m_yMax*s <= that.m_yMax*t  &&  m_zMax*s <= that.m_zMax*t;
}


void Grid::Size::copy(Grid::Size const& that)
{
   m_quality = that.m_quality;
   m_stepSize = that.m_stepSize;
   m_upsample = that.m_upsample;
   m_xMin = that.m_xMin;  m_yMin = that.m_yMin;  m_zMin = that.m_zMin;
   m_xMax = that.m_xMax;  m_yMax = that.m_yMax;  m_zMax = that.m_zMax;
}


double Grid::Size::stepSize() const
{
   double stepSize(0.0);
   // These spacings are chosen so that each step uses roughly four times as
   // many points as the previous one.
   switch (m_quality) {
       case 0:   stepSize = 1.0000;  break;  // not used
       case 1:   stepSize = 0.6300;  break;
       case 2:   stepSize = 0.3969;  break;
       case 3:   stepSize = 0.2500;  break;
       case 4:   stepSize = 0.1575;  break;
       case 5:   stepSize = 0.0992;  break;
       case 6:   stepSize = 0.0625;  break;
       default:  stepSize = m_stepSize;  break;
   }
   return stepSize;
}


void Grid::Size::debugInfo() const
{
   int n = (m_xMax-m_xMin) * (m_yMax-m_yMin) * (m_zMax-m_zMin);
            
   QLOG_TRACE() << "Grid::Size with stepsize: " << stepSize() << "and" << n << "points";
   QLOG_TRACE() << "           limits: i"  << m_xMin << "to" << m_xMax
            << "   -->   x" << stepSize()*m_xMin << "to" << stepSize()*m_xMax;
   QLOG_TRACE() << "                   j"  << m_yMin << "to" << m_yMax
            << "   -->   y" << stepSize()*m_yMin << "to" << stepSize()*m_yMax;
   QLOG_TRACE() << "                   k"  << m_zMin << "to" << m_zMax
            << "   -->   z" << stepSize()*m_zMin << "to" << stepSize()*m_zMax;
}


int Grid::Size::dataCount() const
{
   return (m_xMax-m_xMin) * (m_yMax-m_yMin) * (m_zMax-m_zMin);
}
 

// ----- Grid -----
Grid::Grid(OpenBabel::OBGridData const& gridData) : m_dataType(Grid::DataType::CubeData)
{
   QLOG_DEBUG() << "Grid constructor called for CubeData";
   int nx, ny, nz;
   gridData.GetNumberOfPoints(nx, ny, nz);
   OpenBabel::vector3 v(gridData.GetOriginVector());
   Vec boxMin(v.x(), v.y(), v.z());

   double e1[3], e2[3], e3[3];
   gridData.GetAxes(e1, e2, e3);

   double dx(e1[0]);
   double dy(e2[1]);
   double dz(e3[2]);
   double thresh(1e-6);
   
   if ( std::abs(e1[1]) > thresh || std::abs(e1[2]) > thresh ||
        std::abs(e2[0]) > thresh || std::abs(e2[2]) > thresh ||
        std::abs(e3[0]) > thresh || std::abs(e3[1]) > thresh ) {
        QLOG_WARN() << "Warning: Non-axial grid found in Grid constructor";
   }else if (std::abs(dx-dy) > thresh ||std::abs(dx-dz) > thresh || std::abs(dy-dz) > thresh) {
        QLOG_WARN() << "Warning: Non-regular grid found in Grid constructor";
   }

   if (gridData.GetUnit() == OpenBabel::OBGridData::BOHR) {
      QLOG_TRACE() << "Scaling grid Bohr -> Angstrom";
      dx     *=  BohrToAngstrom;
      boxMin *=  BohrToAngstrom;
   }else if (gridData.GetUnit() == OpenBabel::OBGridData::ANGSTROM) {
      QLOG_TRACE() << "Grid spacings are already in Angstroms";
   }

   QLOG_TRACE() << "Size" << dx << nx << ny << nz;
   QLOG_TRACE() << "Total points" <<  nx * ny * nz;

   Size size(boxMin, dx, nx, ny, nz);
   m_size = size;
   init();

   for (int i = 0; i < nx; ++i) {
       for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                (*m_data)[i][j][k] = gridData.GetValue(i,j,k);
            }
       }
   }

}


// ----- Grid -----
Grid::Grid(Data::Grid const& gridData) : m_dataType(Grid::DataType::CubeData)
{
   QLOG_DEBUG() << "Grid constructor called for CubeData";
   Vec boxMin(gridData.getMin());
   Vec boxMax(gridData.getMax());
   Vec delta(boxMax-boxMin);

   int nx, ny, nz;
   gridData.getNumberOfPoints(nx, ny, nz);

   double dx(delta.x/(nx-1));
   double dy(delta.y/(ny-1));
   double dz(delta.z/(nz-1));
   double thresh(1.0e-6);

   if (std::abs(dx-dy) > thresh ||std::abs(dx-dz) > thresh || std::abs(dy-dz) > thresh) {
      QLOG_WARN() << "Warning: Non-regular grid found in Grid constructor";
   }

   dx     *= BohrToAngstrom;
   boxMin *= BohrToAngstrom;

   QLOG_TRACE() << "Size" << dx << nx << ny << nz;
   QLOG_TRACE() << "Total points" <<  nx * ny * nz;

   Size size(boxMin, dx, nx, ny, nz);
   m_size = size;
   init();

   int cnt(0);
   for (int i = 0; i < nx; ++i) {
       for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                (*m_data)[i][j][k] = gridData.getValue(cnt);
                ++cnt;
            }
       }
   }
}


Grid::Grid(Grid::DataType const& dataType, Grid::Size const& size) 
 : m_dataType(dataType), m_size(size)
{
   init();
}


Grid::Grid(Grid const& that) : QObject(), m_dataType(that.m_dataType), 
   m_size(that.m_size)
{
   init(); 
   copy(that);
}


void Grid::init()
{
   int xNum = m_size.m_xMax - m_size.m_xMin + 1;
   int yNum = m_size.m_yMax - m_size.m_yMin + 1;
   int zNum = m_size.m_zMax - m_size.m_zMin + 1;
   QLOG_DEBUG() << "Attempting to allocate new grid of size:" 
                << xNum << "x" << yNum << "x" << zNum << "=" << xNum*yNum*zNum;
   m_data = new Array3D(boost::extents[xNum][yNum][zNum]);
}


Grid::~Grid()
{
   delete m_data;
}
 

Grid& Grid::operator=(Grid const& that)
{
   if (this != &that) copy(that);
   return *this;
}


Grid& Grid::operator+=(Grid const& that)
{
   m_dataType += that.m_dataType;
   if (m_dataType.isDefined()) {
      combine(1.0, 1.0, that);
   }else {
      QLOG_WARN() << "Grid::operator+= gave Undefined type";
   }
   return *this;
}


Grid& Grid::operator-=(Grid const& that)
{
   m_dataType -= that.m_dataType;
   if (m_dataType.isDefined()) {
      combine(1.0, -1.0, that);
   }else {
      QLOG_WARN() << "Grid::operator-= gave Undefined type";
   }
   return *this;
}


Grid& Grid::operator*=(double const scale)
{
   //m_dataType -= that.m_dataType;
   Index x, y, z;
   for (GLint i = m_size.m_xMin; i <= m_size.m_xMax; ++i) {
       x = i-m_size.m_xMin;
       for (GLint j = m_size.m_yMin; j <= m_size.m_yMax; ++j) {
            y = j-m_size.m_yMin;
            for (GLint k = m_size.m_zMin; k <= m_size.m_zMax; ++k) {
                z = k-m_size.m_zMin;
                (*m_data)[x][y][z] = scale*(*m_data)[x][y][z];
            }
       }
   }  
   return *this;
}


//! computes this = a*this + b*B
void Grid::combine(double const a, double const b, Grid const& B)
{
   if (m_size != B.size()) {
      QLOG_WARN() << "Grid size mismatch";
      m_size.debugInfo();
      B.size().debugInfo();
      return;
   }

   Index x, y, z;
   for (GLint i = m_size.m_xMin; i <= m_size.m_xMax; ++i) {
       x = i-m_size.m_xMin;
       for (GLint j = m_size.m_yMin; j <= m_size.m_yMax; ++j) {
            y = j-m_size.m_yMin;
            for (GLint k = m_size.m_zMin; k <= m_size.m_zMax; ++k) {
                z = k-m_size.m_zMin;
                (*m_data)[x][y][z] = a*(*m_data)[x][y][z] + b*B(i,j,k);
            }
       }
   }  
}


double& Grid::operator()(const int ix, const int iy, const int iz) const
{
   Index x = ix-m_size.m_xMin;
   Index y = iy-m_size.m_yMin;
   Index z = iz-m_size.m_zMin;
   return (*m_data)[x][y][z];
}


void Grid::setValue(int const ix, int const iy, int const iz, double const value)
{
   Index x = ix-m_size.m_xMin;
   Index y = iy-m_size.m_yMin;
   Index z = iz-m_size.m_zMin;
   (*m_data)[x][y][z] = value;
}


QPair<double, double> Grid::range() const
{
   double val((*m_data)[0][0][0]);
   double max(val);
   double min(val);

   int nx(m_size.m_xMax - m_size.m_xMin + 1);
   int ny(m_size.m_yMax - m_size.m_yMin + 1);
   int nz(m_size.m_zMax - m_size.m_zMin + 1);

   for (int i = 0; i < nx; ++i) {
       for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                val = (*m_data)[i][j][k];
                max = std::max(max,val);
                min = std::min(min,val);
            }
       }
   }

   return QPair<double, double>(min, max);
}


// This does a tri-linear interpolation of the function at each of the 8
// grid points around the given point.  
double Grid::operator()(Vec const& position) const
{
   double V000, V001, V010, V011, V100, V101, V110, V111;
   Vec gridPosition(position/stepSize());

   int x0( std::floor(gridPosition.x) );
   int y0( std::floor(gridPosition.y) );
   int z0( std::floor(gridPosition.z) );
   int x1(x0+1);
   int y1(y0+1);
   int z1(z0+1);

   if (x0 < xMin() || x1 > xMax() || 
       y0 < yMin() || y1 > yMax() ||
       z0 < zMin() || z1 > zMax() )   return 0.0;

   V000 = (*this)(x0, y0, z0);
   V001 = (*this)(x0, y0, z1);
   V010 = (*this)(x0, y1, z0);
   V011 = (*this)(x0, y1, z1);
   V100 = (*this)(x1, y0, z0);
   V101 = (*this)(x1, y0, z1);
   V110 = (*this)(x1, y1, z0);
   V111 = (*this)(x1, y1, z1);

   Vec p0(gridPosition.x-x0, gridPosition.y-y0, gridPosition.z-z0);
   Vec p1(x1-gridPosition.x, y1-gridPosition.y, z1-gridPosition.z);

   double value;
   value = V000 * p1.x * p1.y * p1.z
         + V001 * p1.x * p1.y * p0.z
         + V010 * p1.x * p0.y * p1.z
         + V011 * p1.x * p0.y * p0.z
         + V100 * p0.x * p1.y * p1.z
         + V101 * p0.x * p1.y * p0.z
         + V110 * p0.x * p0.y * p1.z
         + V111 * p0.x * p0.y * p0.z;

   return value;
}


// This does a tri-linear interpolation of the derivatives at each of the 8
// grid points around the given point.  
Vec Grid::gradient(Vec const& position) const
{
   Vec V000, V001, V010, V011, V100, V101, V110, V111;
   Vec grid(position/stepSize());

   int x0( std::floor(grid.x) );
   int y0( std::floor(grid.y) );
   int z0( std::floor(grid.z) );
   int x1(x0+1);
   int y1(y0+1);
   int z1(z0+1);

   if (x0 < xMin() || x1 > xMax() || 
       y0 < yMin() || y1 > yMax() ||
       z0 < zMin() || z1 > zMax() )   return Vec();

   double x, y, z;

   int i = x0; int j = y0; int k = z0;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V000.setValue(x, y, z);

   i = x0; j = y0; k = z1;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V001.setValue(x, y, z);

   i = x0; j = y1; k = z0;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V010.setValue(x, y, z);

   i = x0; j = y1; k = z1;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V011.setValue(x, y, z);

   i = x1; j = y0; k = z0;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V100.setValue(x, y, z);

   i = x1; j = y0; k = z1;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V101.setValue(x, y, z);

   i = x1; j = y1; k = z0;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V110.setValue(x, y, z);

   i = x1; j = y1; k = z1;
   x = (*this)(i+1, j,   k  ) - (*this)(i-1, j,   k  );
   y = (*this)(i,   j+1, k  ) - (*this)(i,   j-1, k  );
   z = (*this)(i,   j,   k+1) - (*this)(i,   j,   k-1);
   V111.setValue(x, y, z);

   Vec p0(grid.x-x0, grid.y-y0, grid.z-z0);
   Vec p1(x1-grid.x, y1-grid.y, z1-grid.z);

   double dX, dY, dZ;
   dX = V000.x * p1.x * p1.y * p1.z
      + V001.x * p1.x * p1.y * p0.z
      + V010.x * p1.x * p0.y * p1.z
      + V011.x * p1.x * p0.y * p0.z
      + V100.x * p0.x * p1.y * p1.z
      + V101.x * p0.x * p1.y * p0.z
      + V110.x * p0.x * p0.y * p1.z
      + V111.x * p0.x * p0.y * p0.z;

   dY = V000.y * p1.x * p1.y * p1.z
      + V001.y * p1.x * p1.y * p0.z
      + V010.y * p1.x * p0.y * p1.z
      + V011.y * p1.x * p0.y * p0.z
      + V100.y * p0.x * p1.y * p1.z
      + V101.y * p0.x * p1.y * p0.z
      + V110.y * p0.x * p0.y * p1.z
      + V111.y * p0.x * p0.y * p0.z;

   dZ = V000.z * p1.x * p1.y * p1.z
      + V001.z * p1.x * p1.y * p0.z
      + V010.z * p1.x * p0.y * p1.z
      + V011.z * p1.x * p0.y * p0.z
      + V100.z * p0.x * p1.y * p1.z
      + V101.z * p0.x * p1.y * p0.z
      + V110.z * p0.x * p0.y * p1.z
      + V111.z * p0.x * p0.y * p0.z;

   Vec v(dX, dY, dZ);
   return v.unit();
}




void Grid::debugInfo() const
{
   qDebug() << m_dataType.info();
   m_size.debugInfo();
}


void Grid::copy(Grid const& that)
{
   if (m_size != that.m_size) {
      delete m_data;
      m_size = that.m_size;
      init();
   }
   m_dataType = that.m_dataType;
   combine(0.0, 1.0, that);
}


double Grid::dataSizeInKb() const
{
   return  (double)m_size.dataCount() * sizeof(double)/1024.0;
}



void Grid::generateData(Function3D function)
{
   QTime time;
   time.start();

   QLOG_TRACE() << "Generating Grid with limits: i" << m_size.m_xMin << "to" << m_size.m_xMax
            << "   -->   x" << stepSize()*m_size.m_xMin << "to" << stepSize()*m_size.m_xMax;

   QLOG_TRACE() << "                             j" << m_size.m_yMin << "to" << m_size.m_yMax
            << "   -->   y" << stepSize()*m_size.m_yMin << "to" << stepSize()*m_size.m_yMax;

   QLOG_TRACE() << "                             k" << m_size.m_zMin << "to" << m_size.m_zMax
            << "   -->   z" << stepSize()*m_size.m_zMin << "to" << stepSize()*m_size.m_zMax;

   QLOG_INFO() << "Calculation grid data with stepsize:" << stepSize() << "and"
            << (m_size.m_xMax-m_size.m_xMin) * 
               (m_size.m_yMax-m_size.m_yMin) * 
               (m_size.m_zMax-m_size.m_zMin) << "gridpoints";
   
   double delta(stepSize());
   double progressStep(1.0/(m_size.m_xMax-m_size.m_xMin+1));

   Index x, y, z;
   for (GLint i = m_size.m_xMin; i <= m_size.m_xMax; ++i) {
       x = i-m_size.m_xMin;
       progress(x*progressStep);
       for (GLint j = m_size.m_yMin; j <= m_size.m_yMax; ++j) {
           QApplication::processEvents();
            y = j-m_size.m_yMin;
            for (GLint k = m_size.m_zMin; k <= m_size.m_zMax; ++k) {
                z = k-m_size.m_zMin;
                (*m_data)[x][y][z] = function(i*delta, j*delta, k*delta);
            }
       }
   }

   double t = time.elapsed() /1000.0;
   QLOG_INFO() << "Time taken:" << t << "seconds";
}


// Creates a new grid with double the data density (i.e. 8 times as much data) 
// by using Catmull-Rom cubic spline interpolation.
Grid* Grid::upsample() const
{
//http://onlinelibrary.wiley.com/doi/10.1002/jcc.540140212/abstract
//http://www.sciencedirect.com/science/article/pii/S0009261405016477
//http://perso.univ-rennes1.fr/philippe.rabiller/InteGriTy/ks0156.pdf
   QTime time;
   time.start();

   double delta(stepSize());    // current stepsize;
   Vec min(xMin()*delta, yMin()*delta, zMin()*delta);

   int nx( 2*(xMax()-xMin()+1) - 1);
   int ny( 2*(yMax()-yMin()+1) - 1);
   int nz( 2*(zMax()-zMin()+1) - 1);

   Grid* grid = new Grid(dataType(), Size(min, 0.5*delta, nx, ny, nz));

   QLOG_DEBUG() << "Upsampling grid data type:" << dataType().info();
   size().debugInfo();
   grid->size().debugInfo();
   

   double d[4][4][4];
   double progressStep(1.0/(xMax()-xMin()+1));

   // this is sloppy at the edges and needs fixing
   for (int i = xMin()+1; i < xMax()-1; ++i) {
       progress( (i-xMin())*progressStep );
       for (int j = yMin()+1; j < yMax()-1; ++j) {
           QApplication::processEvents();
           for (int k = zMin()+1; k < zMax()-1; ++k) {

               for (int di = 0; di < 4; ++di) {
                   for (int dj = 0; dj < 4; ++dj) {
                       for (int dk = 0; dk < 4; ++dk) {
                           d[di][dj][dk] = (*this)(i+di-1, j+dj-1, k+dk-1);
                       }
                   }
               }

               grid->setValue(2*i,   2*j,   2*k,   (*this)(i, j, k));
               grid->setValue(2*i+1, 2*j,   2*k,   tricubicInterpolate(d, 0.5, 0.0, 0.0));
               grid->setValue(2*i,   2*j+1, 2*k,   tricubicInterpolate(d, 0.0, 0.5, 0.0));
               grid->setValue(2*i+1, 2*j+1, 2*k,   tricubicInterpolate(d, 0.5, 0.5, 0.0));
               grid->setValue(2*i,   2*j,   2*k+1, tricubicInterpolate(d, 0.0, 0.0, 0.5));
               grid->setValue(2*i+1, 2*j,   2*k+1, tricubicInterpolate(d, 0.5, 0.0, 0.5));
               grid->setValue(2*i,   2*j+1, 2*k+1, tricubicInterpolate(d, 0.0, 0.5, 0.5));
               grid->setValue(2*i+1, 2*j+1, 2*k+1, tricubicInterpolate(d, 0.5, 0.5, 0.5));
           }
       }
   }

   double t = time.elapsed() /1000.0;
   QLOG_DEBUG() << "Upsampling data took:" << t << "seconds";
   return grid;
}


double Grid::cubicInterpolate(double p[4], double x) const
{

    return  p[1]*(1-x)+p[2]*x;  // linear

   double f = (6*p[1] + x*(-2*p[0] - 3*p[1] + 6*p[2] - p[3] + 
      x*(3*p[0] - 6*p[1] + 3*p[2] + x*(-p[0] + 3*p[1] - 3*p[2] + p[3]))))/6.0;

   if ( (f > p[1] && f > p[2]) || (f < p[1] && f < p[2]) ) f = p[1]*(1-x)+p[2]*x;  // linear

   return f;


   double t(x);//stepSize());
   double t2(t*t);
   double t3(t2*t);

   double dx(stepSize());

   double dp1( (p[2]-p[0]) / dx );
   double dp2( (p[3]-p[1]) / dx );
   double c1 = (2*t3-3*t2+1)*p[1] + (t3-2*t2+t)*dp1 + (3*t2-2*t3)*p[2] + (t3-t2)*dp2;


   double f1(p[1]);
   double f2(p[2]);

   //double d1( (p[2]-p[0]) / (2.0*dx) );
   //double d2( (p[3]-p[1]) / (2.0*dx) );



   double h(stepSize());
   double d1(-(2.0*p[0] + 3.0*p[1] - 6.0*p[2] + p[3])/(6.0*h));
   double d2( (p[0] - 6.0*p[1] + 3.0*p[2] + 2.0*p[3])/(6.0*h));
   double a( d1*dx - (f2-f1));
   double b(-d2*dx + (f2-f1));
   return (1.0-x)*f1 + x*f2 + x*(1.0-x) * (a*(1.0-x)+b*x);


   //if (x > 0.1) qDebug() << "Interpolated value:" << p[1] << c1 << p[2];


/*
   c1 = (-((-6 + 11*x - 6*t2 + t3)*p[0]) + 
     x*(3*(6 - 5*x + t2)*p[1] - (-1 + x)*(3*(-3 + x)*p[2] - (-2 + x)*p[3])))/6.;

   c1 = p[0] + t*((-11*p[0])/6. + 3*p[1] - (3*p[2])/2. + 
               t*(p[0] - (5*p[1])/2. + 2*p[2] + 
               t*(-p[0]/6. + p[1]/2. - p[2]/2. + p[3]/6.) - p[3]/2.) + p[3]/3.);

*/

    c1 = p[1] + 0.5*x*(p[2] - p[0] + 
      x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2]-p[3] + 
      x*(3.0*(p[1]-p[2]) + p[3]-p[0])));



   if ( (c1 > p[1] && c1 > p[2]) || (c1 < p[1] && c1 < p[2]) ) {
      c1 = p[1]*(1-x)+p[2]*x;  // linear
   }

   return c1;
}


double Grid::bicubicInterpolate (double p[4][4], double x, double y) const
{
	double array[4];
	array[0] = cubicInterpolate(p[0], y);
	array[1] = cubicInterpolate(p[1], y);
	array[2] = cubicInterpolate(p[2], y);
	array[3] = cubicInterpolate(p[3], y);
	return cubicInterpolate(array, x);
}


double Grid::tricubicInterpolate(double p[4][4][4], double x, double y, double z) const
{
	double array[4];
	array[0] = bicubicInterpolate(p[0], y, z);
	array[1] = bicubicInterpolate(p[1], y, z);
	array[2] = bicubicInterpolate(p[2], y, z);
	array[3] = bicubicInterpolate(p[3], y, z);
	return cubicInterpolate(array, x);
}


bool Grid::saveToCubeFile(QString const& filePath, Layer::Molecule& molecule, 
   bool const invertSign) const
{
   QFile file(filePath);
   if (file.exists() || !file.open(QIODevice::WriteOnly)) return false;

   QStringList header;
   header << "Cube file for " + m_dataType.info();
   header << "Generated using IQmol";
   
   QStringList coords(molecule.coordinatesForCubeFile());
   double h(stepSize()*AngstromToBohr);

   int nx(xMax()-xMin());
   int ny(yMax()-yMin());
   int nz(zMax()-zMin());

   header << QString("%1 %2 %3 %4").arg(coords.size(), 5)
                                   .arg(h*xMin(), 13, 'f', 6)
                                   .arg(h*yMin(), 13, 'f', 6)
                                   .arg(h*zMin(), 13, 'f', 6);
   header << QString("%1 %2 %3 %4").arg(nx, 5)
                                   .arg(h,   13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6);
   header << QString("%1 %2 %3 %4").arg(ny, 5)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(h,   13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6);
   header << QString("%1 %2 %3 %4").arg(nz, 5)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(h,   13, 'f', 6);
   header << coords;

   QByteArray buffer;
   buffer.append(header.join("\n"));
   buffer.append("\n");
   file.write(buffer);
   buffer.clear();

   double w;
   int col(0);

   for (int i = 0; i < nx; ++i) {
       for (int j = 0; j < ny; ++j) {
           for (int k = 0; k < nz; ++k, ++col) {
               w = (*m_data)[i][j][k];
               if (invertSign) w = -w;
               if (w >= 0.0) buffer += " ";
               buffer += QString::number(w, 'E', 5);
               if (col == 5) {
                  col = -1;
                  buffer += "\n";
               }else {
                  buffer += " ";
               }
           }
           file.write(buffer); 
           buffer.clear();
       }
   }

   buffer += "\n";
   file.write(buffer); 
   file.flush();
   file.close();

   return true;
}


} // end namespace IQmol
