#ifndef IQMOL_LAYER_GEMINALORBITALS_H
#define IQMOL_LAYER_GEMINALORBITALS_H
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

#include "Surface.h"
#include "SurfaceLayer.h"
#include "SurfaceInfo.h"
#include "GeminalOrbitalsConfigurator.h"
#include "GridData.h"
#include "Matrix.h"
#include "SpatialProperty.h"
#include <QPair>


namespace IQmol {

namespace Data {
   class GeminalOrbitals;
}

namespace Layer {


   class GeminalOrbitals : public Base {
   typedef QList<QPair<Data::SurfaceType, Data::GridSize> > GridQueue;
   typedef QList<Data::SurfaceInfo> SurfaceInfoQueue;

      Q_OBJECT

      friend class Configurator::GeminalOrbitals;

      public:
         GeminalOrbitals(Data::GeminalOrbitals&);
         ~GeminalOrbitals();
         

         void setMolecule(Molecule* molecule);
         double geminalOrbitalEnergy(unsigned const i) const;
	 //int geminalOrbitalLimits(unsigned const i) const;

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
         unsigned nGeminals() const;
         unsigned nOpenShell() const;

      private Q_SLOTS:
         void showGridInfo();
         void editBoundingBox();

      private:
         // Returns false if the user cancels the calculation
         bool computeOrbitalGrids(Data::GridDataList& grids);
         bool computeDensityGrids(Data::GridDataList& grids);
         void computeDensityVectors();
         void initGeminalOrbitalProperties();
         void computeShellPairs(qglviewer::Vec const& gridPoint);

         Data::GridData* findGrid(Data::SurfaceType const& type, 
            Data::GridSize const& size, Data::GridDataList const& gridList);

         bool processGridQueue(GridQueue const&);
         Data::Surface* generateSurface(Data::SurfaceInfo const&);
         void dumpGridInfo() const;
         void appendSurfaces(Data::SurfaceList&);

         QString description(Data::SurfaceInfo const&, bool const tooltip);

         Configurator::GeminalOrbitals m_configurator;
         Data::GeminalOrbitals& m_geminalOrbitals;

         QList<Vector*> m_densityVectors;
         
         Vector m_shellPairValues;
         Vector m_shellValues;

         SurfaceInfoQueue   m_surfaceInfoQueue;
         Data::GridDataList m_availableGrids;
         qglviewer::Vec     m_bbMin, m_bbMax;   // bounding box
   };


   // This class provides a SpatialProperty interface that can be used to plot the 
   // value of a geminal orbital on an arbitrary surface, but most likely a geminal
   // density.
   class GeminalOrbitalProperty : public SpatialProperty {
      public:
         GeminalOrbitalProperty(Data::GeminalOrbitals const&,  unsigned const index);

      private:
         Data::GeminalOrbitals const& m_geminalOrbitals;
         double orbital(double const x, double const y, double const z) const;
         //MatrixColumn m_coeffs;
	 Matrix const& m_alpha;
	 Matrix const& m_beta;
	 QList<double> const& m_geminals; 
	 unsigned m_index;
   };

} } // End namespace IQmol::Layer 

#endif
