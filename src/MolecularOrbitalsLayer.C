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

#include "MolecularOrbitalsLayer.h"
#include "SurfaceLayer.h"
#include "MoleculeLayer.h"
#include "ProgressDialog.h"
#include "MarchingCubes.h"
#include "QMsgBox.h"
#include "QGLViewer/vec.h"
#include "QsLog.h"
#include <QTime>
#include <cmath>
#include <set>


using namespace qglviewer;

namespace IQmol {
namespace Layer {

MolecularOrbitals::MolecularOrbitals(unsigned int const nAlpha, unsigned int nBeta,
   unsigned int nBasis,
   QList<double>& alphaCoefficients, QList<double> const& alphaEnergies,
   QList<double>&  betaCoefficients, QList<double> const&  betaEnergies,
   ShellList const& shells) : Data("Surfaces"),
   m_nAlpha(nAlpha), m_nBeta(nBeta), m_nBasis(nBasis), m_configurator(this),
   m_alphaEnergies(alphaEnergies), m_betaEnergies(betaEnergies), m_shells(shells)
{
   setCoefficients(alphaCoefficients, betaCoefficients);

   connect(&m_configurator, SIGNAL(queueSurface(Layer::Surface*)),
      this, SLOT(addToQueue(Layer::Surface*)));
   connect(&m_configurator, SIGNAL(clearQueue()),
      this, SLOT(clearSurfaceQueue()));
   connect(&m_configurator, SIGNAL(calculateSurfaces()),
      this, SLOT(processSurfaceQueue()));

   m_configurator.sync();
   setConfigurator(&m_configurator);
}


void MolecularOrbitals::setCoefficients(QList<double>& alpha, QList<double>& beta)
{
   m_restricted = beta.isEmpty();
   m_nOrbitals  = alpha.size() / m_nBasis;
   QLOG_DEBUG() << "MolecularOrbitals number of functions  " << m_nBasis;
   QLOG_DEBUG() << "MolecularOrbitals number of orbitals   " << m_nOrbitals;
   QLOG_DEBUG() << "MolecularOrbitals restricted is set to:" << m_restricted;

   if ( (alpha.size() != (int)(m_nBasis*m_nOrbitals)) || 
        (!m_restricted && beta.size() != (int)(m_nBasis*m_nOrbitals)) ) {
      QString str("Invalid Fchk file\n");
      str += QString::number(alpha.size()) + "!=" + QString::number(m_nBasis*m_nOrbitals);
      QMsgBox::warning(0, "IQmol", str);
      QLOG_WARN() << "Invalid molecular orbital coefficients";
      return;
   }

   m_alphaCoefficients.resize(m_nOrbitals, m_nBasis);
   for (unsigned int i = 0; i < m_nOrbitals; ++i) {
       for (unsigned int j = 0; j < m_nBasis; ++j) {
           m_alphaCoefficients(i,j) = alpha.takeFirst();
       }
   }

   m_betaCoefficients.resize(m_nOrbitals, m_nBasis);
   if (m_restricted) {
      for (unsigned int i = 0; i < m_nOrbitals; ++i) {
          for (unsigned int j = 0; j < m_nBasis; ++j) {
              m_betaCoefficients(i,j) = m_alphaCoefficients(i,j);
          }
      }
   }else {
      for (unsigned int i = 0; i < m_nOrbitals; ++i) {
          for (unsigned int j = 0; j < m_nBasis; ++j) {
              m_betaCoefficients(i,j) = beta.takeFirst();
          }
      }
   }

   calculateDensityVectors();
}


void MolecularOrbitals::calculateDensityVectors()
{
   using namespace boost::numeric::ublas;

   Matrix coeffs(m_nAlpha, m_nBasis);
   Matrix density(m_nBasis, m_nBasis);

   for (unsigned int i = 0; i < m_nAlpha; ++i) {
       for (unsigned int j = 0; j < m_nBasis; ++j) {
           coeffs(i,j) = m_alphaCoefficients(i,j);  
       }
   }

   for (unsigned int i = 0; i < m_nBasis; ++i) {
       for (unsigned int j = 0; j < m_nBasis; ++j) {
           density(i,j) = 0.0;
       }
   }
   noalias(density) = prod(trans(coeffs), coeffs);

   for (unsigned int i = 0; i < m_nBasis; ++i) {
       m_alphaDensity.append(density(i,i));
       for (unsigned int j = i+1; j < m_nBasis; ++j) {
           m_alphaDensity.append(2.0*density(i,j));
       }
   }

   coeffs.resize(m_nBeta, m_nBasis);

   for (unsigned int i = 0; i < m_nBeta; ++i) {
       for (unsigned int j = 0; j < m_nBasis; ++j) {
           coeffs(i,j) = m_betaCoefficients(i,j);
       }
   }

   for (unsigned int i = 0; i < m_nBasis; ++i) {
       for (unsigned int j = 0; j < m_nBasis; ++j) {
           density(i,j) = 0.0;
       }
   }
   noalias(density) = prod(trans(coeffs), coeffs);

   for (unsigned int i = 0; i < m_nBasis; ++i) {
       m_betaDensity.append(density(i,i));
       for (unsigned int j = i+1; j < m_nBasis; ++j) {
           m_betaDensity.append(2.0*density(i,j));
       }
   }
}


// This requires all the Grids be of the same size and all the orbitals to be
// of the same spin.
void MolecularOrbitals::calculateOrbitalGrids(QList<Grid*> grids)
{
   if (grids.isEmpty()) return;

   // Check that the grids are all of the same size and Spin
   Grid* g(grids[0]);
   QList<int> orbitals;
   QList<Grid*>::iterator iter;

   for (iter = grids.begin(); iter != grids.end(); ++iter) {
       if ( ((*iter)->size() != g->size()) ) {
          QLOG_ERROR() << "Different sized grids found in MolecularOrbitals::calculateOrbitalGrids";
          return;
       }
       if ( ((*iter)->dataType().type() != Grid::DataType::AlphaOrbital) &&
            ((*iter)->dataType().type() != Grid::DataType::BetaOrbital) ) {
          QLOG_ERROR() << "Incorrect grid type found in MolecularOrbitals::calculateOrbitalGrids";
          QLOG_ERROR() << (*iter)->dataType().info(); 
          return;
       }
       orbitals.append((*iter)->dataType().index()-1);
   }

   QTime time;
   time.start();

   Matrix* coefficients;
   if (g->dataType().type() == Grid::DataType::AlphaOrbital) {
      QLOG_TRACE() << "Setting MO coefficient data to Alpha";
      coefficients = &m_alphaCoefficients;
   }else {
      QLOG_TRACE() << "Setting MO coefficient data to Beta";
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
   QLOG_INFO() << "Time to compute orbital grid data:" << t << "seconds";
}


void MolecularOrbitals::calculateDensityGrids(Grid*& alpha, Grid*& beta)
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

   for (int i = 0; i < 0; ++i) {
       QLOG_DEBUG() << "Alpha" << i << m_alphaDensity[i];
   }
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
   QLOG_INFO() << "Time to compute density grid data:" << t << "seconds";

   // upsample the grids

   if (alpha->size().upsample()) {
      progressDialog.setInfo("Upsampling grid data");
      disconnect(this, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
      connect(alpha, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
      Grid* upAlpha(alpha->upsample());

      disconnect(this, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
      connect(alpha, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
      Grid* upBeta  = beta->upsample();

      delete alpha;
      delete beta;
      alpha = upAlpha;
      beta  = upBeta;
   }
}


void MolecularOrbitals::calculateDensityGrids2(Grid*& alpha, Grid*& beta)
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

   double progressStep(0.25/(xMax-xMin+1)); 
   double a, b, x, y, z;
   unsigned int n(m_alphaDensity.size()), count(0);

   ProgressDialog progressDialog("Computing Surface",0);
   progressDialog.setInfo("Calculating density grid data");

   connect(this, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
   progressDialog.show();

   for (int i = 0; i < 0; ++i) {
       QLOG_DEBUG() << "Alpha" << i << m_alphaDensity[i];
   }

   // We take a two pass approach, the first computes data on a grid with half
   // the density (so a factor of 8 fewer points than the target grid).  We
   // then used these values in a subsequent pass to refine only those parts
   // with significant density.
   for (int i = xMin; i <= xMax; i=i+2) {
       progress(count*progressStep);
       ++count;
       x = i*delta; 

       for (int j = yMin; j <= yMax; j=j+2) {
           QApplication::processEvents();
           y = j*delta; 

           for (int k = zMin; k <= zMax; k=k+2) {
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

   double x000, x001, x010, x011, x100, x101, x110, x111;
   progressStep *= 7.0;
   count = 0;

   for (int i = xMin+1; i <= xMax-1; i=i+2) {
       progress(count*progressStep+0.25);
       ++count;
       x = i*delta; 

       for (int j = yMin+1; j <= yMax-1; j=j+2) {
           QApplication::processEvents();
           y = j*delta; 

           for (int k = zMin+1; k <= zMax-1; k=k+2) {
               z = k*delta;

               x000 = (*alpha)(i-1, j-1, k-1);
               x001 = (*alpha)(i-1, j-1, k+1);
               x010 = (*alpha)(i-1, j+1, k-1);
               x011 = (*alpha)(i-1, j+1, k+1);
               x100 = (*alpha)(i+1, j-1, k-1);
               x101 = (*alpha)(i+1, j-1, k+1);
               x110 = (*alpha)(i+1, j+1, k-1);
               x111 = (*alpha)(i+1, j+1, k+1);

               if (x000+x001+x010+x011+x100+x101+x110+x111 > 0.125*Shell::thresh()) {
                  QPair<double,double> rho;

                  //  0  0  0 
                  rho = computeGridPointDensity(m_shells, m_nBasis, n, Vec(x, y, z));
                  alpha->setValue(i,j,k, rho.first);
                  beta ->setValue(i,j,k, rho.second);
                  //  0  0  1 
                  rho = computeGridPointDensity(m_shells, m_nBasis, n, Vec(x, y, z-delta));
                  alpha->setValue(i,j,k-1, rho.first);
                  beta ->setValue(i,j,k-1, rho.second);
                  //  0  1  0 
                  rho = computeGridPointDensity(m_shells, m_nBasis, n, Vec(x, y-delta, z));
                  alpha->setValue(i,j-1,k, rho.first);
                  beta ->setValue(i,j-1,k, rho.second);
                  //  0  1  1 
                  rho = computeGridPointDensity(m_shells, m_nBasis, n, Vec(x, y-delta, z-delta));
                  alpha->setValue(i,j-1,k-1, rho.first);
                  beta ->setValue(i,j-1,k-1, rho.second);
                  //  1  0  0 
                  rho = computeGridPointDensity(m_shells, m_nBasis, n, Vec(x-delta, y, z));
                  alpha->setValue(i-1,j,k, rho.first);
                  beta ->setValue(i-1,j,k, rho.second);
                  //  1  0  1 
                  rho = computeGridPointDensity(m_shells, m_nBasis, n, Vec(x-delta, y, z-delta));
                  alpha->setValue(i-1,j,k-1, rho.first);
                  beta ->setValue(i-1,j,k-1, rho.second);
                  //  1  1  0 
                  rho = computeGridPointDensity(m_shells, m_nBasis, n, Vec(x-delta, y-delta, z));
                  alpha->setValue(i-1,j-1,k, rho.first);
                  beta ->setValue(i-1,j-1,k, rho.second);
                  
               }else {
                  double rho;

                  rho = 0.125*(x000+x001+x010+x011+x100+x101+x110+x111);
                  alpha->setValue(i,  j,  k,   rho);

                  rho = 0.25*(x000+x010+x100+x110);
                  alpha->setValue(i,  j,  k-1, rho);

                  rho = 0.25*(x000+x001+x100+x101);
                  alpha->setValue(i,  j-1,k,   rho);

                  rho = 0.5*(x000+x100);
                  alpha->setValue(i,  j-1,k-1, rho);

                  rho = 0.25*(x000+x001+x010+x011);
                  alpha->setValue(i-1,j,  k,   rho);

                  rho = 0.5*(x000+x010);
                  alpha->setValue(i-1,j,  k-1, rho);

                  rho = 0.5*(x000+x001);
                  alpha->setValue(i-1,j-1,k,   rho);

                  x000 = (*beta)(i-1, j-1, k-1);
                  x001 = (*beta)(i-1, j-1, k+1);
                  x010 = (*beta)(i-1, j+1, k-1);
                  x011 = (*beta)(i-1, j+1, k+1);
                  x100 = (*beta)(i+1, j-1, k-1);
                  x101 = (*beta)(i+1, j-1, k+1);
                  x110 = (*beta)(i+1, j+1, k-1);
                  x111 = (*beta)(i+1, j+1, k+1);

                  rho = 0.125*(x000+x001+x010+x011+x100+x101+x110+x111);
                  beta ->setValue(i,  j,  k,   rho);

                  rho = 0.25*(x000+x010+x100+x110);
                  beta ->setValue(i,  j,  k-1, rho);

                  rho = 0.25*(x000+x001+x100+x101);
                  beta ->setValue(i,  j-1,k,   rho);

                  rho = 0.5*(x000+x100);
                  beta ->setValue(i,  j-1,k-1, rho);

                  rho = 0.25*(x000+x001+x010+x011);
                  beta ->setValue(i-1,j,  k,   rho);

                  rho = 0.5*(x000+x010);
                  beta ->setValue(i-1,j,  k-1, rho);

                  rho = 0.5*(x000+x001);
                  beta ->setValue(i-1,j-1,k,   rho);

               }

            }
       }
   }

   double t = time.elapsed() / 1000.0;
   QLOG_INFO() << "Time to compute density grid data:" << t << "seconds";
}


QPair<double, double> MolecularOrbitals::computeGridPointDensity(ShellList const& shells, 
   int const nBasis, int const nOrbs, Vec const& point)
{
   std::vector<double> s2(Shell::evaluateShellPairs(shells, nBasis, point));
   double a(0.0);
   double b(0.0);

   for (int ii = 0; ii < nOrbs; ++ii) {
       a += m_alphaDensity[ii] * s2[ii];
       b += m_betaDensity[ii]  * s2[ii];
   }
   return QPair<double,double>(a,b);
}


//! Calculates a bounding box such that outside this box all shells have a
//! value lower than Shell::s_thresh.
void MolecularOrbitals::boundingBox(Vec& min, Vec& max)
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


void MolecularOrbitals::addToQueue(Surface* surface) 
{ 
   bool add(true);
   SurfaceList::iterator iter;
   for (iter = m_surfaceQueue.begin(); iter != m_surfaceQueue.end(); ++iter) {
       add = add && (*(*iter) != *surface);
   }

   if (add) {
      m_surfaceQueue.append(surface); 
      QLOG_TRACE() << "Adding surface to queue" << surface->text();
   }else {
      QLOG_TRACE() << "Surface Already queued" << surface->text();
   }
}


void MolecularOrbitals::clearSurfaceQueue()
{ 
   m_surfaceQueue.clear(); 
}


Grid* MolecularOrbitals::findGrid(Grid::DataType const& dataType, Grid::Size const& size, 
   GridList const& gridList)
{
   Grid* grid(0);
   GridList::const_iterator iter;
   for (iter = gridList.begin(); iter != gridList.end(); ++iter) {
       if ( dataType == (*iter)->dataType() && size <= (*iter)->size() ) {
          grid = (*iter);
          QLOG_TRACE() << "Existing Grid data found";
          (*iter)->dataType().info();
          break;
       }
   }
   return grid;
}


void MolecularOrbitals::processSurfaceQueue()
{
   Vec min, max;
   boundingBox(min, max);
   SurfaceList::iterator surface;

   // First, do an initial pass to determine what data needs to be calculated
   for (surface = m_surfaceQueue.begin(); surface != m_surfaceQueue.end(); ++surface) {
       Grid::DataType dataType((*surface)->gridDataType());
       Grid::Size size(min, max, (*surface)->quality(), (*surface)->upsample());
       Grid* grid(findGrid(dataType, size, m_availableGrids));
       if (!grid) m_gridQueue.append(qMakePair(dataType, size));

       // Add orbital energy information
       if (dataType.type() == Grid::DataType::AlphaOrbital ||
           dataType.type() == Grid::DataType::BetaOrbital) {
          int nElectrons;
          double orbitalEnergy;
          if (dataType.type() == Grid::DataType::AlphaOrbital) {
             nElectrons = m_nAlpha;
             orbitalEnergy = m_alphaEnergies[dataType.index()-1];
          }else {
             nElectrons = m_nBeta;
             orbitalEnergy = m_betaEnergies[dataType.index()-1];
          }

          QString label(dataType.info());

          if (dataType.index() == nElectrons-1) {
             label += " (HOMO-1)";
          }else if (dataType.index() == nElectrons) {
             label += " (HOMO)";
          }else if (dataType.index() == nElectrons+1) {
             label += " (LUMO)";
          }else if (dataType.index() == nElectrons+2) {
             label += " (LUMO+1)";
          }

          (*surface)->setText(label);
          label += "\nEnergy   = ";
          label += QString::number(orbitalEnergy, 'f', 3);
          (*surface)->createToolTip(label);
       }
   }

   // Second, calculate any grid data that we haven't got
   if (!m_gridQueue.isEmpty()) processGridQueue();

   // Third, calculate requested surfaces
   ProgressDialog progressDialog("Computing Surfaces",0);
   connect(this, SIGNAL(progress(double)), &progressDialog, SLOT(updateProgress(double)));
   progressDialog.show();
   QString info("Calculating surface ");
   double progressStep(1.0/(m_surfaceQueue.count()+1));
   int prog(0);

   for (surface = m_surfaceQueue.begin(); surface != m_surfaceQueue.end(); ++surface) {
       progressDialog.setInfo(info + QString::number(prog+1));
       progress(prog*progressStep);
       ++prog;
       QApplication::processEvents();
       calculateSurface(*surface);
   }

   // Finally, tell the world we have a new surface available
   for (surface = m_surfaceQueue.begin(); surface != m_surfaceQueue.end(); ++surface) {
       (*surface)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |
          Qt::ItemIsEnabled | Qt::ItemIsEditable);
       (*surface)->setCheckState(Qt::Unchecked);
       if (m_molecule) connect((*surface), SIGNAL(updated()), m_molecule, SIGNAL(softUpdate()));
       appendRow(*surface);
   }

   if (!m_surfaceQueue.isEmpty()) m_surfaceQueue[0]->setCheckState(Qt::Checked);
   clearSurfaceQueue();
}


 
void MolecularOrbitals::processGridQueue()
{
   // First obtain a list of the unique grid sizes
   std::set<Grid::Size> sizes;
   GridQueue::iterator queued; 
   for (queued = m_gridQueue.begin(); queued != m_gridQueue.end(); ++queued) {
       sizes.insert(queued->second);    
   }

   // Second, determine what data is required for each grid size
   QLOG_TRACE() << "There are" << sizes.size() << "different grid sizes";
   std::set<Grid::Size>::iterator size;
   for (size = sizes.begin(); size != sizes.end(); ++size) {
       std::set<Grid::DataType> densities;
       std::set<Grid::DataType> alphaOrbitals;
       std::set<Grid::DataType> betaOrbitals;
       
       for (queued = m_gridQueue.begin(); queued != m_gridQueue.end(); ++queued) {
           if (queued->second == *size) {
              Grid::DataType type(queued->first);

              if (type.isDensity()) {
                 densities.insert(type);
              }else if (type.type() == Grid::DataType::AlphaOrbital) {
                 alphaOrbitals.insert(type);
              }else if (type.type() == Grid::DataType::BetaOrbital) {
                 betaOrbitals.insert(type); 
              }else  {
                 QLOG_WARN() << "Unknown Grid type found in processQueue";
              }
           }
       }

       if (densities.size() > 0) {
          Grid* alphaGrid = new Grid(Grid::DataType::AlphaDensity, *size);
          Grid* betaGrid  = new Grid(Grid::DataType::BetaDensity,  *size);

          calculateDensityGrids2(alphaGrid, betaGrid);

          m_availableGrids.append(alphaGrid);
          m_availableGrids.append(betaGrid);

          Grid* spinGrid  = new Grid(*alphaGrid - *betaGrid);
          Grid* totalGrid = new Grid(*alphaGrid + *betaGrid);

          m_availableGrids.append(spinGrid);
          m_availableGrids.append(totalGrid);
       }

       GridList grids;
       std::set<Grid::DataType>::iterator iter;
       for (iter = alphaOrbitals.begin(); iter != alphaOrbitals.end(); ++iter) {
           Grid* grid = new Grid(*iter, *size);
           grids.append(grid); 
       }

       if (grids.count() > 0) {
          calculateOrbitalGrids(grids);
          m_availableGrids += grids;
       }

       grids.clear();
       for (iter = betaOrbitals.begin(); iter != betaOrbitals.end(); ++iter) {
           Grid* grid = new Grid(*iter, *size);
           grids.append(grid); 
       }

       if (grids.count() > 0) {
          calculateOrbitalGrids(grids);
          m_availableGrids += grids;
       }
   }

   m_gridQueue.clear();
}



void MolecularOrbitals::calculateSurface(Surface* surface)
{
   QTime time;
   time.start();

   Vec min, max;
   boundingBox(min, max);

   Grid::DataType dataType(surface->gridDataType());
   Grid::Size size(min, max, surface->quality());
   Grid* grid(findGrid(dataType, size, m_availableGrids));

   if (!grid) {
      QLOG_ERROR() << "Grid data not found in MolecularOrbitals::calculateSurfaces()";
      return;
   }

   MarchingCubes mc(grid);
   GLfloat isovalue(surface->isovalue());
   
   if ( grid->dataType().isSigned() ) {
      surface->setSurfaceData(mc.generateSurface( isovalue), Surface::Positive);
      surface->setSurfaceData(mc.generateSurface(-isovalue), Surface::Negative);
   }else {
      surface->setSurfaceData(mc.generateSurface(isovalue));
   }

   double t = time.elapsed() / 1000.0;
   QLOG_INFO() << "Time to compute" << dataType.info() << "surface at level" 
            << surface->quality() << ":" << t << "seconds";
}



} } // end namespace IQmol::Layer
