#ifndef IQMOL_LAYER_MOLECULARORBITALS_H
#define IQMOL_LAYER_MOLECULARORBITALS_H
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

#include "SurfaceLayer.h"
#include "SurfaceInfo.h"
#include "MolecularOrbitalsConfigurator.h"
#include "Surface.h"
#include "Matrix.h"
#include "GridData.h"
#include "Function.h"
#include "GridEvaluator.h"
#include "MolecularOrbitals.h"
#include <QPair>


class QProgressDialog;

namespace IQmol {

class MolecularGridEvaluator;

namespace Data {
   class Density;
   class GridData;
}

namespace Layer {

   typedef QList<QPair<Data::SurfaceType, Data::GridSize> > GridQueue;
   typedef QList<Data::SurfaceInfo> SurfaceInfoQueue;

   class MolecularOrbitals : public Base {

      Q_OBJECT

      friend class Configurator::MolecularOrbitals;

      public:
         MolecularOrbitals(Data::MolecularOrbitals&);
         ~MolecularOrbitals() { }

         void setMolecule(Molecule* molecule);
         double alphaOrbitalEnergy(unsigned const i) const;
         double betaOrbitalEnergy(unsigned const i) const;

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
         Data::MolecularOrbitals::OrbitalType orbitalType() const {
            return m_molecularOrbitals.orbitalType();
         }

         Data::DensityList densityList() { return m_availableDensities; }

      private Q_SLOTS:
         void showGridInfo();
         void editBoundingBox();
         void gridEvaluatorFinished();
         void gridEvaluatorCanceled();
         void calculateSurfaces();

      private:
         // Returns false if the user cancels the calculation
         void computeDensityVectors();

         Data::GridData* findGrid(Data::SurfaceType const& type, 
            Data::GridSize const& size, Data::GridDataList const& gridList);
         Data::Surface* generateSurface(Data::SurfaceInfo const&);
         void dumpGridInfo() const;
         void appendSurfaces(Data::SurfaceList&);
         QString description(Data::SurfaceInfo const&, bool const tooltip);

         Configurator::MolecularOrbitals m_configurator;
         Data::MolecularOrbitals& m_molecularOrbitals;

         Data::DensityList       m_availableDensities;
         SurfaceInfoQueue        m_surfaceInfoQueue;
         Data::GridDataList      m_availableGrids;
         qglviewer::Vec          m_bbMin, m_bbMax;   // bounding box
         MolecularGridEvaluator* m_molecularGridEvaluator;

         QProgressDialog* m_progressDialog;
   };

} } // End namespace IQmol::Layer 
#endif
