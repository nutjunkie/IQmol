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

#include "CubeDataLayer.h"
#include "SurfaceLayer.h"
#include "MoleculeLayer.h"
#include "MarchingCubes.h"
#include "Grid.h"
#include "QsLog.h"



namespace IQmol {
namespace Layer {

CubeData::CubeData(Grid* grid) : Data("Cube Data"), m_grid(grid), m_configurator(this)
{
   connect(&m_configurator, SIGNAL(calculateSurface(Layer::Surface*)),
      this, SLOT(calculateSurface(Layer::Surface*)));
   setConfigurator(&m_configurator);
}


CubeData::~CubeData()
{
   if (m_grid) delete m_grid;
}


void CubeData::setMolecule(Molecule* molecule) 
{
   m_molecule = molecule;
   // Actions for the context menu
   connect(newAction("Surface Animator"), SIGNAL(triggered()),
      m_molecule, SLOT(openSurfaceAnimator()));
}


void CubeData::appendAtom(int const Z, qglviewer::Vec const& position)
{
   m_geometry.append(Z, position);
}


GridBased* CubeData::createProperty()
{
   return new GridBased("Cube Data", m_grid);
}


void CubeData::calculateSurface(Surface* surface)
{
   if (!m_grid) {
      QLOG_ERROR() << "Grid data not available";
      return;
   }

   MarchingCubes mc(m_grid);
   GLfloat isovalue(surface->isovalue());

   if ( surface->cubeIsSigned() ) {
      surface->setSurfaceData(mc.generateSurface( isovalue), Surface::Positive);
      surface->setSurfaceData(mc.generateSurface(-isovalue), Surface::Negative);
   }else {
      surface->setSurfaceData(mc.generateSurface(isovalue));
   }

   surface->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |
      Qt::ItemIsEnabled | Qt::ItemIsEditable);
   if (m_molecule) connect(surface, SIGNAL(updated()), m_molecule, SIGNAL(softUpdate()));
   appendRow(surface);
   surface->setCheckState(Qt::Checked);
}

} } // end namespace IQmol:;Layer
