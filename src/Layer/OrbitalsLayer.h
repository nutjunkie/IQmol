#ifndef IQMOL_LAYER_ORBITALS_H
#define IQMOL_LAYER_ORBITALS_H
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

#include "Orbitals.h"
#include "Layer.h"
#include "GridData.h"
#include "SurfaceInfo.h"
#include "OrbitalsConfigurator.h"
#include "Density.h"
#include <QPair>


class QProgressDialog;

namespace IQmol {

class MolecularGridEvaluator;

namespace Data {
   class GridData;
   class GridSize;
   class Surface;
   class SurfaceList;
}

namespace Layer {

   class Surface;

   class Orbitals : public Base {

      Q_OBJECT

      friend class Configurator::Orbitals;

      public:
         Orbitals(Data::Orbitals&);
         ~Orbitals() { }

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
         
         virtual double alphaOrbitalEnergy(unsigned) const { return 0.0; }
         virtual double betaOrbitalEnergy(unsigned)  const { return 0.0; }

         Data::Orbitals& m_orbitals;
         Data::DensityList m_availableDensities;
         //Data::DensityList const& densityList() const { return m_availableDensities; }
         Data::Orbitals::OrbitalType orbitalType() const {
            return m_orbitals.orbitalType();
         }

      private Q_SLOTS:
         void showGridInfo();
         void editBoundingBox();
         void gridEvaluatorFinished();
         void gridEvaluatorCanceled();
         void calculateSurfaces();

      private:
         Data::GridData* findGrid(Data::SurfaceType const& type, 
            Data::GridSize const& size, Data::GridDataList const& gridList);
         Data::Surface* generateSurface(Data::SurfaceInfo const&);
         void dumpGridInfo() const;
         void appendSurfaces(Data::SurfaceList&);

         virtual QString description(Data::SurfaceInfo const&, bool const tooltip);

         Configurator::Orbitals m_configurator;

         typedef QList<Data::SurfaceInfo> SurfaceInfoQueue;

         SurfaceInfoQueue        m_surfaceInfoQueue;
         Data::GridDataList      m_availableGrids;
         qglviewer::Vec          m_bbMin, m_bbMax;   // bounding box
         MolecularGridEvaluator* m_molecularGridEvaluator;
         QProgressDialog*        m_progressDialog;
   };

} } // End namespace IQmol::Layer 
#endif
