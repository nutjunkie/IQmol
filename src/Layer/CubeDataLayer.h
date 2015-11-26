#ifndef IQMOL_LAYER_CUBEDATA_H
#define IQMOL_LAYER_CUBEDATA_H
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

#include "Layer.h"
#include "Geometry.h"
#include "CubeDataConfigurator.h"


namespace IQmol {

class GridBased;

namespace Data {
   class CubeData;
   class SurfaceInfo;
}

namespace Layer {

   class Surface;

   /// Layer handle for the contents of a cube data file.
   class CubeData : public Base {

      Q_OBJECT

      public:
         CubeData(Data::CubeData const&);

         Data::Geometry const& geometry() const;

         void setMolecule(Molecule*);

         GridBased* createProperty() const;

         // required for the SurfaceAnimator
         Data::CubeData const& cubeData() const 
         { 
            return m_cube; 
         } 

      public Q_SLOTS:
         Surface* calculateSurface(Data::SurfaceInfo const&);

      private:
         Configurator::CubeData m_configurator;
         Data::CubeData const& m_cube;
   };

} } // End namespace IQmol::Layer

#endif
