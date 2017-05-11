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

#include "SurfaceGenerator.h"
#include "GridData.h"
#include "GridSize.h"
#include "SurfaceInfo.h"
#include "Surface.h"
#include "MarchingCubes.h"
#include "MeshDecimator.h"
#include "QsLog.h"


namespace IQmol {
namespace Grid {

SurfaceGenerator::SurfaceGenerator(
   Data::GridData const& grid, 
   Data::SurfaceInfo const& surfaceInfo)
 : m_grid(grid), m_surfaceInfo(surfaceInfo), m_surface(0)
{
}


Data::Surface* SurfaceGenerator::getSurface() const
{ 
   return m_surface; 
}


void SurfaceGenerator::run()
{
   double delta(Data::GridSize::stepSize(m_surfaceInfo.quality()));

   MarchingCubes mc(m_grid);
   m_surface = new Data::Surface(m_surfaceInfo);

   mc.generateMesh(m_surfaceInfo.isovalue(), m_surface->meshPositive());

   if (m_surfaceInfo.simplifyMesh()) {
      MeshDecimator decimator(m_surface->meshPositive());
      if (!decimator.decimate(delta)) {
         QLOG_ERROR() << "Mesh decimation failed:" << decimator.error();
      }   
   }   

   if (m_surfaceInfo.type().isSigned()) {
      mc.generateMesh(-m_surfaceInfo.isovalue(), m_surface->meshNegative());
      if (m_surfaceInfo.simplifyMesh()) {
         MeshDecimator decimator(m_surface->meshNegative());
         if (!decimator.decimate(delta)) {
               QLOG_ERROR() << "Mesh decimation failed:" << decimator.error();
         }   
      }   
   }   
}

} } // end namespace IQmol::Grid
