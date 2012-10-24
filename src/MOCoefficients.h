#ifndef IQMOL_MOCOEFFICIENTS_H
#define IQMOL_MOCOEFFICIENTS_H
/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include "IQmol.h"
#include "Shell.h"
#include "boost/numeric/ublas/matrix.hpp"
#include <QObject>
#include <QList>


typedef boost::numeric::ublas::matrix<double> Matrix;

namespace IQmol {

   class Grid;

   /// Data class that encapsulats a singel determinant wavefunction.
   class MOCoefficients : public QObject {

      Q_OBJECT

      friend class FChkDataParser;

      public:
         MOCoefficients() : m_initialized(false) { }
         ~MOCoefficients() { }

         int nAlpha() const { return m_nAlpha; }
         int nBeta()  const { return m_nBeta;  }
         int nBasis() const { return m_nBasis; }
         int nOrbitals() const { return m_nOrbitals; }
         int charge() const { return m_charge; }
         int multiplicity() const { return m_multiplicity; }
         bool restricted() const { return m_restricted; }

         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max);
         void calculateOrbitalGrids(QList<Grid*> grids);
         void calculateDensityGrids(Grid* alpha, Grid* beta);

      Q_SIGNALS:
         void progress(double);

      protected:
         void setCoefficients(QList<double>& alpha, QList<double>& beta);
         int m_nAlpha;
         int m_nBeta;
         int m_nBasis;
         int m_nOrbitals;
         int m_charge;
         int m_multiplicity;
         bool m_restricted;
         bool m_initialized;

         Matrix m_alphaCoefficients;
         Matrix m_betaCoefficients;
         QList<double> m_alphaDensity;
         QList<double> m_betaDensity;
         QList<double> m_alphaEnergies;
         QList<double> m_betaEnergies;
         ShellList m_shells;  // Should probably be a pointer

      private:
         void calculateDensityVectors();
   };

} // End namespace IQmol

#endif
