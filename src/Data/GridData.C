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

#include "Constants.h"
#include "GridData.h"
#include "GridSize.h"
#include "QsLog.h"
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <stdexcept>


namespace IQmol {
namespace Data {


GridData::GridData(GridSize const& size, SurfaceType const& type) : m_surfaceType(type),
   m_origin(size.origin()), m_delta(size.delta())
{
   Array3D::extent_gen extents;
   m_data.resize(extents[size.nx()][size.ny()][size.nz()]);
}


GridData::GridData(GridSize const& size, SurfaceType const& type, QList<double> const& data)
 : m_surfaceType(type), m_origin(size.origin()), m_delta(size.delta())
{
   unsigned nx(size.nx());
   unsigned ny(size.ny());
   unsigned nz(size.nz());

   if ((unsigned)data.size() < nx*ny*nz) throw std::out_of_range("Insufficient Array3D data");

   Array3D::extent_gen extents;
   m_data.resize(extents[nx][ny][nz]);

   unsigned count(0);
   for (unsigned i = 0; i < nx; ++i) {
       for (unsigned j = 0; j < ny; ++j) {
           for (unsigned k = 0; k < nz; ++k, ++count) {
               m_data[i][j][k] = data[count];
           }
       }
   }
}


GridData::GridData(GridData const& that) : Base()
{
   copy(that);
}


void GridData::copy(GridData const& that)
{
   m_surfaceType  = that.m_surfaceType;
   m_origin       = that.m_origin;
   m_delta        = that.m_delta;

   unsigned nx, ny, nz;
   that.getNumberOfPoints(nx, ny, nz);
   Array3D::extent_gen extents;
   m_data.resize(extents[nx][ny][nz]);
   m_data = that.m_data;
}


void GridData::getNumberOfPoints(unsigned& nx, unsigned& ny, unsigned& nz) const
{
   nx = m_data.shape()[0];
   ny = m_data.shape()[1];
   nz = m_data.shape()[2];
}


void GridData::getBoundingBox(qglviewer::Vec& min, qglviewer::Vec& max) const
{
   unsigned nx, ny, nz;
   getNumberOfPoints(nx, ny, nz);

   min    = m_origin;
   max    = m_origin;
   max.x += nx*m_delta.x;
   max.y += ny*m_delta.y;
   max.z += nz*m_delta.z;
}


void GridData::getRange(double& min, double& max)
{
   if (m_data.num_elements() == 0) {
      min = 0.0; 
      max = 0.0;
      return;
   }

   unsigned nx, ny, nz;
   getNumberOfPoints(nx, ny, nz);

   min = m_data[0][0][0];
   max = min;

   for (unsigned i = 0; i < nx; ++i) {
       for (unsigned j = 0; j < ny; ++j) {
           for (unsigned k = 0; k < nz; ++k) {
               min = std::min(min, m_data[i][j][k]);
               max = std::max(max, m_data[i][j][k]);
           }
       }
   }
}


double GridData::dataSizeInKb() const
{
   double total(m_data.num_elements());
   return total * sizeof(double) / 1024.0;
}


GridSize GridData::size() const
{
   unsigned nx, ny, nz;
   getNumberOfPoints(nx, ny, nz);
   return GridSize(m_origin, m_delta, nx, ny, nz);
}


void GridData::combine(double const a, double const b, GridData const& B)
{  
   unsigned nx, ny, nz;
   getNumberOfPoints(nx, ny, nz);

   if (size() == B.size()) {

      for (unsigned i = 0; i < nx; ++i) {
          for (unsigned j = 0; j < ny; ++j) {
              for (unsigned k = 0; k < nz; ++k) {
                  m_data[i][j][k] = a*m_data[i][j][k] + b*B.m_data[i][j][k];
              }
          }
      }

   }else {
      
      QLOG_WARN() << "Size mismatch in GridData::combine";  // expensive route
      double x(m_origin.x);
      for (unsigned i = 0; i < nx; ++i) {
          double y(m_origin.y);
          for (unsigned j = 0; j < ny; ++j) {
              double z(m_origin.z);
              for (unsigned k = 0; k < nz; ++k) {
                  m_data[i][j][k] = a*m_data[i][j][k] + b*B.interpolate(x, y, z);
                  z += m_delta.z;
              }
              y += m_delta.y;
          }
          x += m_delta.x;
      }

   }
}


GridData& GridData::operator=(GridData const& that)
{
   if (this != &that) copy(that);
   return *this;
}


GridData& GridData::operator*=(double const scale)
{
   unsigned nx, ny, nz;
   getNumberOfPoints(nx, ny, nz);

   for (unsigned i = 0; i < nx; ++i) {
       for (unsigned j = 0; j < ny; ++j) {
           for (unsigned k = 0; k < nz; ++k) {
               m_data[i][j][k] *= scale;
           }
       }
   }
   return *this;
}


GridData& GridData::operator+=(GridData const& that)
{
   combine(1.0, 1.0, that);
   return *this;
}


GridData& GridData::operator-=(GridData const& that)
{
   combine(1.0, -1.0, that);
   return *this;
}


double GridData::interpolate(double const x, double const y, double const z) const
{
   double value(0.0);

   double gx( (x-m_origin.x)/m_delta.x );
   double gy( (y-m_origin.y)/m_delta.y );
   double gz( (z-m_origin.z)/m_delta.z );

   if (gx < 0.0 || gy < 0.0 || gz < 0.0) return value;

   unsigned x0( std::floor(gx) );
   unsigned y0( std::floor(gy) );
   unsigned z0( std::floor(gz) );

   unsigned x1( x0+1 );
   unsigned y1( y0+1 );
   unsigned z1( z0+1 );

   if (x1 >= m_data.shape()[0] ||   
       y1 >= m_data.shape()[1] || 
       z1 >= m_data.shape()[2] ) return value;

   qglviewer::Vec p0(gx-x0, gy-y0, gz-z0);
   qglviewer::Vec p1(x1-gx, y1-gy, z1-gz);

   double w000(p1.x * p1.y * p1.z);
   double w001(p1.x * p1.y * p0.z);
   double w010(p1.x * p0.y * p1.z);
   double w011(p1.x * p0.y * p0.z);
   double w100(p0.x * p1.y * p1.z);
   double w101(p0.x * p1.y * p0.z);
   double w110(p0.x * p0.y * p1.z);
   double w111(p0.x * p0.y * p0.z);

   value = w000 * m_data[x0][y0][z0]
         + w001 * m_data[x0][y0][z1]
         + w010 * m_data[x0][y1][z0]
         + w011 * m_data[x0][y1][z1]
         + w100 * m_data[x1][y0][z0]
         + w101 * m_data[x1][y0][z1]
         + w110 * m_data[x1][y1][z0]
         + w111 * m_data[x1][y1][z1];

   return value;
}


qglviewer::Vec GridData::normal(double const x, double const y, double const z) const
{
   qglviewer::Vec grad(0.0, 0.0, 0.0);

   double gx( (x-m_origin.x) /m_delta.x);
   double gy( (y-m_origin.y) /m_delta.y);
   double gz( (z-m_origin.z) /m_delta.z);

   if (gx < 1.0 || gy < 1.0 || gz < 1.0) return grad;

   unsigned x0( std::floor(gx) );
   unsigned y0( std::floor(gy) );
   unsigned z0( std::floor(gz) );

   unsigned x1( x0+1 );
   unsigned y1( y0+1 );
   unsigned z1( z0+1 );

   if (x1 >= m_data.shape()[0]-1 ||  
       y1 >= m_data.shape()[1]-1 ||
       z1 >= m_data.shape()[2]-1 )  return grad;

   qglviewer::Vec v000, v001, v010, v011, v100, v101, v110, v111;

   int i = x0; int j = y0; int k = z0;
   v000.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v000.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v000.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   i = x0; j = y0; k = z1;
   v001.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v001.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v001.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   i = x0; j = y1; k = z0;
   v010.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v010.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v010.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   i = x0; j = y1; k = z1;
   v011.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v011.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v011.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   i = x1; j = y0; k = z0;
   v100.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v100.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v100.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   i = x1; j = y0; k = z1;
   v101.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v101.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v101.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   i = x1; j = y1; k = z0;
   v110.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v110.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v110.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   i = x1; j = y1; k = z1;
   v111.x = m_data[i+1][j  ][k  ] - m_data[i-1][j  ][k  ];
   v111.y = m_data[i  ][j+1][k  ] - m_data[i  ][j-1][k  ];
   v111.z = m_data[i  ][j  ][k+1] - m_data[i  ][j  ][k-1];

   qglviewer::Vec p0(gx-x0, gy-y0, gz-z0);
   qglviewer::Vec p1(x1-gx, y1-gy, z1-gz);

   double w000(p1.x * p1.y * p1.z);
   double w001(p1.x * p1.y * p0.z);
   double w010(p1.x * p0.y * p1.z);
   double w011(p1.x * p0.y * p0.z);
   double w100(p0.x * p1.y * p1.z);
   double w101(p0.x * p1.y * p0.z);
   double w110(p0.x * p0.y * p1.z);
   double w111(p0.x * p0.y * p0.z);

   grad.x = v000.x*w000 + v001.x*w001 + v010.x*w010 + v011.x*w011
          + v100.x*w100 + v101.x*w101 + v110.x*w110 + v111.x*w111;
   grad.y = v000.y*w000 + v001.y*w001 + v010.y*w010 + v011.y*w011
          + v100.y*w100 + v101.y*w101 + v110.y*w110 + v111.y*w111;
   grad.z = v000.z*w000 + v001.z*w001 + v010.z*w010 + v011.z*w011
          + v100.z*w100 + v101.z*w101 + v110.z*w110 + v111.z*w111;

   return -grad.unit();
}


void GridData::dump() const
{
   qDebug() << "GridData data:" << m_surfaceType.toString();
   qDebug() << "  x = " << m_origin.x << m_delta.x << m_data.shape()[0];
   qDebug() << "  y = " << m_origin.y << m_delta.y << m_data.shape()[1];
   qDebug() << "  z = " << m_origin.z << m_delta.z << m_data.shape()[2];
}


bool GridData::saveToCubeFile(QString const& filePath, QStringList const& coordinates, 
   bool const invertSign) const
{
   QFile file(filePath);
   if (file.exists() || !file.open(QIODevice::WriteOnly)) return false;

   QStringList header;
   header << "Cube file for " + m_surfaceType.toString();
   header << "Generated using IQmol";
   
   qglviewer::Vec delta(Constants::AngstromToBohr*m_delta);
   qglviewer::Vec origin(Constants::AngstromToBohr*m_origin);

   unsigned nx, ny, nz;
   getNumberOfPoints(nx, ny, nz);

   header << QString("%1 %2 %3 %4").arg(coordinates.size(), 5)
                                   .arg(origin.x, 13, 'f', 6)
                                   .arg(origin.y, 13, 'f', 6)
                                   .arg(origin.z, 13, 'f', 6); 

   header << QString("%1 %2 %3 %4").arg(nx, 5)
                                   .arg(delta.x, 13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6); 
   header << QString("%1 %2 %3 %4").arg(ny, 5)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(delta.y, 13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6); 
   header << QString("%1 %2 %3 %4").arg(nz, 5)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(0.0, 13, 'f', 6)
                                   .arg(delta.z, 13, 'f', 6); 
   header << coordinates;

   QByteArray buffer;
   buffer.append(header.join("\n"));
   buffer.append("\n");
   file.write(buffer);
   buffer.clear();

   double w;
   unsigned col(0);

   for (unsigned i = 0; i < nx; ++i) {
       for (unsigned j = 0; j < ny; ++j) {
           for (unsigned k = 0; k < nz; ++k, ++col) {
               w = m_data[i][j][k];
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

} } // end namespace IQmol::Data
