#ifndef IQMOL_LAYER_MOLECULARSURFACES_H
#define IQMOL_LAYER_MOLECULARSURFACES_H
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
#include "MolecularSurfacesConfigurator.h"
#include "SurfaceInfo.h"
#include "Surface.h"



namespace IQmol {

namespace Data {
   class Surface;
}

namespace Layer {

   class Molecule;

   typedef QList<Data::SurfaceInfo> SurfaceInfoQueue;

   /// Main Layer forming the parent item for all molecular surfaces.  By default
   /// this contains the pseudo densities (SAD, vdW, promolecule).  
   class MolecularSurfaces : public Base {

      Q_OBJECT

      friend class Configurator::MolecularSurfaces;

      public:
         MolecularSurfaces(Layer::Molecule& molecule);

      Q_SIGNALS:
         void progress(double);
         void softUpdate();

      protected Q_SLOTS:
         void addToQueue(Data::SurfaceInfo const&);
         void processSurfaceQueue();

      private:
       
         template <class T>
         Data::Surface* 
            calculateSuperposition(Data::SurfaceInfo const&, bool doCharges = false);
         Data::Surface* calculateVanDerWaals(Data::SurfaceInfo const&);

         Layer::Molecule&  m_molecule;
         Configurator::MolecularSurfaces m_configurator;
         SurfaceInfoQueue  m_surfaceInfoQueue;
   };

} } // End namespace IQmol::Layer 
#endif
