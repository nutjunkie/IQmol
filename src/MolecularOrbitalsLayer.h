#ifndef IQMOL_MOLECULARORBITALSLAYER_H
#define IQMOL_MOLECULARORBITALSLAYER_H
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
#include "SurfaceLayer.h"
#include "Shell.h"
#include "Grid.h"
#include "MolecularOrbitalsConfigurator.h"
#include "boost/numeric/ublas/matrix.hpp"
#include <QList>
#include <QPair>


typedef boost::numeric::ublas::matrix<double> Matrix;

namespace IQmol {

typedef QList<QPair<Grid::DataType, Grid::Size> >  GridQueue;

namespace Layer {

   /// Layer::Data class that encapsulats a single determinant wavefunction.
   class MolecularOrbitals : public Data {

      Q_OBJECT

      friend class Configurator::MolecularOrbitals;

      public:
         MolecularOrbitals(unsigned int const nAlpha, unsigned int nBeta, 
            unsigned int nBasis,
            QList<double>& alphaCoefficients, QList<double> const& alphaEnergies,
            QList<double>&  betaCoefficients, QList<double> const&  betaEnergies,
            ShellList const& shells);
         ~MolecularOrbitals() { }

         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max);

      Q_SIGNALS:
         void progress(double);

      public Q_SLOTS:
         void addToQueue(Layer::Surface* surface);
         void clearSurfaceQueue();
         void processSurfaceQueue();

      protected:
         // Returns false if the user cancels the calculation
         bool calculateOrbitalGrids(QList<Grid*> grids);
         bool calculateDensityGrids(Grid*& alpha, Grid*& beta);
         unsigned int m_nAlpha;
         unsigned int m_nBeta;
         unsigned int m_nBasis;
         unsigned int m_nOrbitals;
         bool m_restricted;

      private Q_SLOTS:
         void showGridInfo();
         void editBoundingBox();

      private:
         void setCoefficients(QList<double>& alpha, QList<double>& beta);
         void calculateDensityVectors();

         Grid* findGrid(Grid::DataType const& dataType, Grid::Size const& size,
            GridList const& gridList);
         bool processGridQueue();
         void calculateSurface(Layer::Surface* surface);

         QPair<double, double> computeGridPointDensity(ShellList const&, 
            int const nBasis, int const nOrbs, qglviewer::Vec const& point);

         Configurator::MolecularOrbitals m_configurator;

         Matrix m_alphaCoefficients;
         Matrix m_betaCoefficients;
         QList<double> m_alphaDensity;
         QList<double> m_betaDensity;
         QList<double> m_alphaEnergies;
         QList<double> m_betaEnergies;
         ShellList m_shells;

         SurfaceList m_surfaceQueue;
         GridList m_availableGrids;
         GridQueue  m_gridQueue;

         qglviewer::Vec m_bbMin, m_bbMax;
         bool m_bbSet;
   };

} } // End namespace IQmol::Layer 
#endif
