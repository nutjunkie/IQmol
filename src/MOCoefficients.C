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

#include "MOCoefficients.h"
#include "ProgressDialog.h"
#include "Grid.h"
#include "QsLog.h"
#include "QMsgBox.h"
#include "QGLViewer/vec.h"
#include <QTime>
#include <cmath>


using namespace qglviewer;

namespace IQmol {

// This requires all the Grids be of the same size and all the orbitals to be
// of the same spin.
void MOCoefficients::calculateOrbitalGrids(QList<Grid*> grids)
{
   if (!m_initialized || grids.isEmpty()) {
      QLOG_ERROR() << "Attempt to calculate orbitals with uninitialized data";
      return;
   }

   // Check that the grids are all of the same size and Spin
   Grid* g(grids[0]);
   QList<int> orbitals;
   QList<Grid*>::iterator iter;

   for (iter = grids.begin(); iter != grids.end(); ++iter) {
       if ( ((*iter)->size() != g->size()) ) {
          QLOG_ERROR() << "Different sized grids found";
          return;
       }
       if ( ((*iter)->dataType().type() != Grid::DataType::AlphaOrbital) &&
            ((*iter)->dataType().type() != Grid::DataType::BetaOrbital) ) {
          QLOG_ERROR() << "Incorrect grid type found";
          QLOG_ERROR() << (*iter)->dataType().info(); 
          return;
       }
       orbitals.append((*iter)->dataType().index()-1);
   }

   QTime time;
   time.start();

   Matrix* coefficients;
   if (g->dataType() != Grid::DataType::AlphaOrbital) {
      coefficients = &m_alphaCoefficients;
   }else {
      coefficients = &m_betaCoefficients;
   }
   
   int nOrb = orbitals.size();
   double delta(g->stepSize());
   int xMin(g->xMin());
   int yMin(g->yMin());
   int zMin(g->zMin());
   int xMax(g->xMax());
   int yMax(g->yMax());
   int zMax(g->zMax());

   ProgressDialog progressDialog("Computing Surface",0);
   progressDialog.setInfo("Calculating orbital data");
   connect(this, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
   progressDialog.show();

   unsigned int prog(0);
   double  progressStep(1.0/(xMax-xMin+1));
   double  x, y, z;
   double* values;
   double* tmp = new double[nOrb];
   ShellList::iterator shell;

   for (int i = xMin; i <= xMax; ++i) {
       progress(prog*progressStep);
       ++prog;
       x = i*delta;

       for (int j = yMin; j <= yMax; ++j) {
           QApplication::processEvents();
           y = j*delta;

           for (int k = zMin; k <= zMax; ++k) {
               z = k*delta;
               Vec gridPoint(x,y,z);

               for (int orb = 0; orb < nOrb; ++orb) tmp[orb] = 0.0;
               int count(0);

               //-----------------------------------------------------
               for (shell = m_shells.begin(); shell != m_shells.end(); ++shell) {
                   if ( (values = (*shell)->evaluate(gridPoint)) ) {
                      for (unsigned int s = 0; s < (*shell)->size(); ++s) {
                          for (int orb = 0; orb < nOrb; ++orb) {
                              tmp[orb] += (*coefficients)(orbitals[orb], count) * values[s];
                          }
                          ++count;
                      }
                   }else {
                      count += (*shell)->size();
                   }
               }

               for (int orb = 0; orb < nOrb; ++orb) {
                   grids.at(orb)->setValue(i,j,k, tmp[orb]);
               }
               //-----------------------------------------------------
           }
       }
   }

   delete [] tmp;

   double t = time.elapsed() / 1000.0;
   QLOG_INFO() << "Time to compute orbital grid data: " << t << "seconds";
}


void MOCoefficients::calculateDensityGrids(Grid* alpha, Grid* beta)
{
   QTime time;
   time.start();

   double delta(alpha->stepSize());
   int xMin(alpha->xMin());
   int yMin(alpha->yMin());
   int zMin(alpha->zMin());
   int xMax(alpha->xMax());
   int yMax(alpha->yMax());
   int zMax(alpha->zMax());

   double progressStep(1.0/(xMax-xMin+1));
   double a, b, x, y, z;
   unsigned int n(m_alphaDensity.size()), count(0);

   ProgressDialog progressDialog("Computing Surface",0);
   progressDialog.setInfo("Calculating density grid data");

   connect(this, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
   progressDialog.show();

   for (int i = xMin; i <= xMax; ++i) {
       progress(count*progressStep);
       ++count;
       x = i*delta; 

       for (int j = yMin; j <= yMax; ++j) {
           QApplication::processEvents();
           y = j*delta; 

           for (int k = zMin; k <= zMax; ++k) {
               z = k*delta;
               Vec gridPoint(x, y, z);

               std::vector<double> s2 = Shell::evaluateShellPairs(m_shells, m_nBasis, gridPoint);

               a = 0.0;
               b = 0.0;
               for (unsigned int ii = 0; ii < n; ++ii) {
                   a += m_alphaDensity[ii] * s2[ii];
                   b += m_betaDensity[ii]  * s2[ii];
               }
               alpha->setValue(i,j,k, a);
               beta ->setValue(i,j,k, b);
            }
       }
   }

   double t = time.elapsed() / 1000.0;
   QLOG_INFO() << "Time to compute density grid data: " << t << "seconds";
}


//! Calculates a bounding box such that outside this box all shells have a
//! value lower than Shell::s_thresh.
void MOCoefficients::boundingBox(Vec& min, Vec& max)
{
   if (m_shells.isEmpty()) {

      min.x = 0.0;
      min.y = 0.0;
      min.z = 0.0;
      max.x = 0.0;
      max.y = 0.0;
      max.z = 0.0;

   }else {

      m_shells[0]->boundingBox(min, max);
      Vec tmin, tmax;

      QList<Shell*>::iterator iter;
      for (iter = m_shells.begin(); iter != m_shells.end(); ++iter) {
          (*iter)->boundingBox(tmin, tmax);
          min.x = std::min(tmin.x, min.x);
          min.y = std::min(tmin.y, min.y);
          min.z = std::min(tmin.z, min.z);
          max.x = std::max(tmax.x, max.x);
          max.y = std::max(tmax.y, max.y);
          max.z = std::max(tmax.z, max.z);
      }
   }
}


void MOCoefficients::setCoefficients(QList<double>& alpha, QList<double>& beta)
{
   m_restricted = beta.isEmpty();
   m_nOrbitals  = alpha.size() / m_nBasis;

   if ( (alpha.size() != m_nBasis*m_nOrbitals) || 
        (!m_restricted && beta.size()  != m_nBasis*m_nOrbitals) ) {
      QString str("Invalid Fchk file\n");
      str += QString::number(alpha.size()) + "!=" + QString::number(m_nBasis*m_nOrbitals);
      QMsgBox::warning(0, "IQmol", str);
      return;
   }

   m_alphaCoefficients.resize(m_nOrbitals, m_nBasis);
   for (int i = 0; i < m_nOrbitals; ++i) {
       for (int j = 0; j < m_nBasis; ++j) {
           m_alphaCoefficients(i,j) = alpha.takeFirst();
       }
   }

   if (!m_restricted) {
      m_betaCoefficients.resize(m_nOrbitals, m_nBasis);
      for (int i = 0; i < m_nOrbitals; ++i) {
          for (int j = 0; j < m_nBasis; ++j) {
              m_betaCoefficients(i,j) = beta.takeFirst();
          }
      }
   }

   calculateDensityVectors();
}


void MOCoefficients::calculateDensityVectors()
{
   using namespace boost::numeric::ublas;

   Matrix coeffs(m_nAlpha, m_nBasis);
   Matrix density(m_nBasis, m_nBasis);

   for (int i = 0; i < m_nAlpha; ++i) {
       for (int j = 0; j < m_nBasis; ++j) {
           coeffs(i,j) = m_alphaCoefficients(i,j);  
       }
   }

   for (int i = 0; i < m_nBasis; ++i) {
       for (int j = 0; j < m_nBasis; ++j) {
           density(i,j) = 0.0;
       }
   }
   noalias(density) = prod(trans(coeffs), coeffs);

   for (int i = 0; i < m_nBasis; ++i) {
       m_alphaDensity.append(density(i,i));
       for (int j = i+1; j < m_nBasis; ++j) {
           m_alphaDensity.append(2.0*density(i,j));
       }
   }

   coeffs.resize(m_nBeta, m_nBasis);

   for (int i = 0; i < m_nBeta; ++i) {
       for (int j = 0; j < m_nBasis; ++j) {
           coeffs(i,j) = m_betaCoefficients(i,j);
       }
   }

   for (int i = 0; i < m_nBasis; ++i) {
       for (int j = 0; j < m_nBasis; ++j) {
           density(i,j) = 0.0;
       }
   }
   noalias(density) = prod(trans(coeffs), coeffs);

   for (int i = 0; i < m_nBasis; ++i) {
       m_betaDensity.append(density(i,i));
       for (int j = i+1; j < m_nBasis; ++j) {
           m_betaDensity.append(2.0*density(i,j));
       }
   }
}



} // end namespace IQmol
