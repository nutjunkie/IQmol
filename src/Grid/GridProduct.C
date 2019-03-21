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

#include "GridProduct.h"
#include "GridData.h"
#include "QsLog.h"
#include <QApplication>


namespace IQmol {

GridProduct::GridProduct(Vector& values, QList<Data::GridData const*>&  grids, 
   double const binSize) : m_values(values), m_grids(grids), m_binSize(binSize)
{
   if (m_grids.isEmpty()) return;
   unsigned nx, ny, nz;
   m_grids[0]->getNumberOfPoints(nx, ny, nz);
   m_totalProgress = nx*ny*nz;
}



void GridProduct::run()
{
   unsigned nGrids(m_grids.size());

   if (nGrids == 0) {
      QLOG_ERROR() << "No available grids";
      return;
   }

   // We assume all the grids are the same size
   unsigned nx, ny, nz;
   unsigned prog(0);
   qglviewer::Vec origin((m_grids[0])->origin());
   qglviewer::Vec delta((m_grids[0])->delta());
   (m_grids[0])->getNumberOfPoints(nx, ny, nz);

   unsigned nBins((m_grids[0])->maxR()/m_binSize+1);

   m_values.resize(nBins);
   std::fill(m_values.begin(),m_values.end(), 0.0f);

   Vector f1(nGrids);
   Vector f2(nGrids);

   double x1(origin.x);
   for (unsigned i1(0); i1 < nx; ++i1, x1 += delta.x) {

      double y1(origin.y);
      for (unsigned j1(0); j1 < ny; ++j1, y1 += delta.y) {

         double z1(origin.z);
         for (unsigned k1(0); k1 < nz; ++k1, z1 += delta.z, ++prog) {

            for (unsigned g(0); g < nGrids; ++g) { f1[g] = (*(m_grids[g]))(i1, j1, k1); }

            double x2(origin.x);
            for (unsigned i2(0); i2 < nx; ++i2, x2 += delta.x) {
               double dx(x1-x2);

               double y2(origin.y);
               for (unsigned j2(0); j2 < ny; ++j2, y2 += delta.y) {
                  double dy(y1-y2);

                  double z2(origin.z);
                  for (unsigned k2(0); k2 < nz; ++k2, z2 += delta.z) {
                     double dz(z1-z2);

                     double r(std::sqrt((dx*dx + dy*dy + dz*dz)));
                     unsigned b(r/m_binSize); 

                     if (b < nBins) {
                        for (unsigned g(0); g < nGrids; ++g) { 
                            f2[g] = (*(m_grids[g]))(i2, j2, k2); 
                        }
                        double p(boost::numeric::ublas::inner_prod(f1, f2));
                        m_values[b] = std::max(std::abs(p), m_values[b]);
                     }

                  }
               }
            }

            progressValue(prog);
            if (m_terminate) return;
         }
      }
   }
}


} // end namespace IQmol
