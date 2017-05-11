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

#include "GridEvaluator.h"
#include "GridData.h"
#include "QsLog.h"
#include <QApplication>


namespace IQmol {

GridEvaluator::GridEvaluator(Data::GridData& grid, Function3D const& function) 
  : m_grid(grid), m_function(function)
{
   unsigned nx, ny, nz;
   m_grid.getNumberOfPoints(nx, ny, nz);
   m_totalProgress = nx;
}



void GridEvaluator::run()
{
   unsigned nx, ny, nz;
   m_grid.getNumberOfPoints(nx, ny, nz);

   qglviewer::Vec origin(m_grid.origin());
   qglviewer::Vec delta(m_grid.delta());

   double x(origin.x);
   for (unsigned i = 0; i < nx; ++i, x += delta.x) {
       double y(origin.y);
       for (unsigned j = 0; j < ny; ++j, y += delta.y) {
           double z(origin.z);
           for (unsigned k = 0; k < nz; ++k, z += delta.z) {
               m_grid(i, j, k) = m_function(x, y, z);
           }
       }
       progress(i);
       if (m_terminate) break;
   }
}


// ---------- MultiGridEvaluator ---------


MultiGridEvaluator::MultiGridEvaluator(QList<Data::GridData*> grids, 
  MultiFunction3D const& function, double const thresh, bool const coarseGrain) 
  : m_grids(grids),  m_function(function), m_thresh(thresh), m_coarseGrain(coarseGrain)
 
{
   if (grids.isEmpty()) return;

   unsigned nx, ny, nz;
   Data::GridData* g0(m_grids.first());
   g0->getNumberOfPoints(nx, ny, nz);
   m_totalProgress = m_coarseGrain ? 4*nx : nx;

   Data::GridDataList::iterator iter;
   for (iter = m_grids.begin(); iter != m_grids.end(); ++iter) {
       if ( ((*iter)->size() != g0->size()) ) {
          QLOG_ERROR() << "Different sized grids found in MultiGridEvaluator";
       }
    }
}



void MultiGridEvaluator::run()
{
   if (m_coarseGrain) return runCoarseGrain();

   unsigned nGrids(m_grids.size());
   unsigned nx, ny, nz;

   Data::GridData* g0(m_grids.first());
   g0->getNumberOfPoints(nx, ny, nz);

   qglviewer::Vec origin(g0->origin());
   qglviewer::Vec delta(g0->delta());

   double x(origin.x);
   for (unsigned i = 0; i < nx; ++i, x += delta.x) {
       double y(origin.y);
       for (unsigned j = 0; j < ny; ++j, y += delta.y) {
           double z(origin.z);
           for (unsigned k = 0; k < nz; ++k, z += delta.z) {
               Vector const& values(m_function(x, y, z));
               for (unsigned f = 0; f < nGrids; ++f) {
                    (*m_grids[f])(i, j, k) = values[f];
               }
           }
       }
       progress(i); 
       if (m_terminate) break;
   }
   progress(m_totalProgress); 
}



void MultiGridEvaluator::runCoarseGrain()
{
   unsigned nGrids(m_grids.size());
   unsigned nx, ny, nz;
   Data::GridData* g0(m_grids.first());
   g0->getNumberOfPoints(nx, ny, nz);

   qglviewer::Vec origin(g0->origin());
   qglviewer::Vec delta(g0->delta());

   // We take a two-pass approach, the first computes data on a grid with half
   // the number of points for each dimension (so a factor of 8 fewer points
   // than the target grid).  The second pass fills in the remainder of the grid
   // either using interpolation (where the values are insignificant) or explicit
   // evaluation

   int prog(0);
   double x, y, z;

   // Just use the maximum function value at each grid point for screenting
   Array3D screen;
   Array3D::extent_gen extents;
   screen.resize(extents[1+nx/2][1+ny/2][1+nz/2]);

   // First Pass (sparse)
   x = origin.x;
   for (unsigned i = 0; i < nx; i += 2, x += 2.0*delta.x) {
       y = origin.y;
       for (unsigned j = 0; j < ny; j += 2, y += 2.0*delta.y) {
           z = origin.z;
           for (unsigned k = 0; k < nz; k += 2, z += 2.0*delta.z) {
               Vector const& values(m_function(x, y, z));
               double max(0.0);
               for (unsigned f = 0; f < nGrids; ++f) {
                   (*m_grids[f])(i, j, k) = values[f];
                   max = std::max(max, std::abs(values[f]));
               }
               screen[i/2][j/2][k/2] = max;
           }
       }
       progress(prog++); 
       if (m_terminate) return;
   }

   double g000, g001, g010, g011, g100, g101, g110, g111;
   origin += delta;

   x = origin.x;
   for (unsigned i = 1;  i < nx-1;  i += 2, x += 2.0*delta.x) {
       y = origin.y;
       for (unsigned j = 1;  j < ny-1;  j += 2, y += 2.0*delta.y) {
           z = origin.z;
           for (unsigned k = 1;  k < nz-1;  k += 2, z += 2.0*delta.z) {

               // Compute exact values
               if (screen[(i-1)/2][(j-1)/2][(k-1)/2] > 0.125*m_thresh) {

                  Vector const& v0(m_function(x, y, z));
                  for (unsigned f = 0; f < nGrids; ++f) (*m_grids[f])(i,  j,  k  ) = v0[f];

                  Vector const& v1(m_function(x, y, z-delta.z));
                  for (unsigned f = 0; f < nGrids; ++f) (*m_grids[f])(i,  j,  k-1) = v1[f];

                  Vector const& v2(m_function(x, y-delta.y, z));
                  for (unsigned f = 0; f < nGrids; ++f) (*m_grids[f])(i,  j-1,k  ) = v2[f];

                  Vector const& v3(m_function(x, y-delta.y, z-delta.z));
                  for (unsigned f = 0; f < nGrids; ++f) (*m_grids[f])(i,  j-1,k-1) = v3[f];

                  Vector const& v4(m_function(x-delta.x, y, z));
                  for (unsigned f = 0; f < nGrids; ++f) (*m_grids[f])(i-1,j,  k  ) = v4[f];

                  Vector const& v5(m_function(x-delta.x, y, z-delta.z));
                  for (unsigned f = 0; f < nGrids; ++f) (*m_grids[f])(i-1,j,  k-1) = v5[f];

                  Vector const& v6(m_function(x-delta.x, y-delta.y, z));
                  for (unsigned f = 0; f < nGrids; ++f) (*m_grids[f])(i-1,j-1,k  ) = v6[f];
 
               }else {
                  // Use interpolation
                  for (unsigned f = 0; f < nGrids; ++f) {
                      g000 = (*m_grids[f])(i-1, j-1, k-1);
                      g001 = (*m_grids[f])(i-1, j-1, k+1);
                      g010 = (*m_grids[f])(i-1, j+1, k-1);
                      g011 = (*m_grids[f])(i-1, j+1, k+1);
                      g100 = (*m_grids[f])(i+1, j-1, k-1);
                      g101 = (*m_grids[f])(i+1, j-1, k+1);
                      g110 = (*m_grids[f])(i+1, j+1, k-1);
                      g111 = (*m_grids[f])(i+1, j+1, k+1);

                      (*m_grids[f])(i,  j,  k  ) = 0.125*(g000+g001+g010+g011+
                                                          g100+g101+g110+g111);
                      (*m_grids[f])(i,  j,  k-1) = 0.250*(g000+g010+g100+g110);
                      (*m_grids[f])(i,  j-1,k  ) = 0.250*(g000+g001+g100+g101);
                      (*m_grids[f])(i,  j-1,k-1) = 0.500*(g000+g100);
                      (*m_grids[f])(i-1,j,  k  ) = 0.250*(g000+g001+g010+g011);
                      (*m_grids[f])(i-1,j,  k-1) = 0.500*(g000+g010);
                      (*m_grids[f])(i-1,j-1,k  ) = 0.500*(g000+g001);
                  }

               }

            }
       }

       prog += 7;
       progress(prog); 
       if (m_terminate) return;
   }
   progress(m_totalProgress); 
}

} // end namespace IQmol
