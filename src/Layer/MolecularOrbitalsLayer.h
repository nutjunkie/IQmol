#ifndef IQMOL_LAYER_MOLECULARORBITALS_H
#define IQMOL_LAYER_MOLECULARORBITALS_H
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

#include "Surface.h"
#include "SurfaceLayer.h"
#include "SurfaceInfo.h"
#include "MolecularOrbitalsConfigurator.h"
#include "GridData.h"
#include "Matrix.h"
#include <QPair>


namespace IQmol {

namespace Data {
   class MolecularOrbitals;
}

namespace Layer {

   typedef QList<QPair<Data::SurfaceType, Data::GridSize> > GridQueue;
   typedef QList<Data::SurfaceInfo> SurfaceInfoQueue;

   class MolecularOrbitals : public Base {

      Q_OBJECT

      friend class Configurator::MolecularOrbitals;

      public:
         MolecularOrbitals(Data::MolecularOrbitals&);
         void setMolecule(Molecule* molecule);

      Q_SIGNALS:
         void progress(double);
         void softUpdate();

      protected Q_SLOTS:
         void addToQueue(Data::SurfaceInfo const&);
         void clearSurfaceQueue();
         void processSurfaceQueue();

      protected:
         unsigned nAlpha() const;
         unsigned nBeta() const;
         unsigned nBasis() const;
         unsigned nOrbitals() const;

      private Q_SLOTS:
         void showGridInfo();
         void editBoundingBox();

      private:
         // Returns false if the user cancels the calculation
         bool computeOrbitalGrids(Data::GridDataList& grids);
         bool computeDensityGrids(Data::GridData*& alpha, Data::GridData*& beta);
         void computeDensityVectors();
         void computeShellPairs(qglviewer::Vec const& gridPoint);

         Data::GridData* findGrid(Data::SurfaceType const& type, 
            Data::GridSize const& size, Data::GridDataList const& gridList);

         bool processGridQueue(GridQueue const&);
         Data::Surface* generateSurface(Data::SurfaceInfo const&);
         void dumpGridInfo() const;
         void appendSurfaces(Data::SurfaceList&);

         QString description(Data::SurfaceType const&);

         Configurator::MolecularOrbitals m_configurator;
         Data::MolecularOrbitals& m_molecularOrbitals;

         Vector m_alphaDensity;
         Vector m_betaDensity;
         Vector m_shellPairValues;
         Vector m_shellValues;

         SurfaceInfoQueue   m_surfaceInfoQueue;
         Data::GridDataList m_availableGrids;
         qglviewer::Vec     m_bbMin, m_bbMax;   // bounding box
   };

} } // End namespace IQmol::Layer 
#endif
