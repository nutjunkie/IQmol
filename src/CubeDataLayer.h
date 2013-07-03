#ifndef IQMOL_CUBEDATALAYER_H
#define IQMOL_CUBEDATALAYER_H
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

#include "DataLayer.h"
#include "CubeDataConfigurator.h"
#include "SpatialProperty.h"
#include "Geometry.h"


namespace IQmol {

class Grid;

namespace Layer {

   class Surface;

   /// Layer handle for the contents of a cube data file.
   class CubeData : public Data {

      Q_OBJECT

      public:
         CubeData(Grid* grid);
         ~CubeData();

         void setMolecule(Molecule*);
         GridBased* createProperty();
         Grid* grid() { return m_grid; } 

         void appendAtom(int const Z, qglviewer::Vec const& position);
         IQmol::Data::Geometry const& geometry() const { return m_geometry; }

      public Q_SLOTS:
         void calculateSurface(Layer::Surface*);

      private:
         Grid* m_grid;
         IQmol::Data::Geometry m_geometry;
         Configurator::CubeData m_configurator;
   };

} } // End namespace IQmol::Layer

#endif
