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

#include "MolecularOrbitalsLayer.h"
#include "MolecularOrbitals.h"
#include "MoleculeLayer.h"
#include "GridInfoDialog.h"
#include "MarchingCubes.h"
#include "MeshDecimator.h"
#include "BoundingBoxDialog.h"
#include "SurfaceType.h"
#include "SurfaceInfo.h"
#include "QMsgBox.h"
#include "QGLViewer/vec.h"
#include "QsLog.h"
#include <QTime>
#include <QProgressDialog>
#include <cmath>
#include <set>

#include <QDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {

MolecularOrbitals::MolecularOrbitals(Data::MolecularOrbitals& molecularOrbitals)
 : Base(molecularOrbitals.label()), m_configurator(*this), m_molecularOrbitals(molecularOrbitals)
{
   connect(&m_configurator, SIGNAL(queueSurface(Data::SurfaceInfo const&)),
      this, SLOT(addToQueue(Data::SurfaceInfo const&)));

   connect(&m_configurator, SIGNAL(clearQueue()),
      this, SLOT(clearSurfaceQueue()));

   connect(&m_configurator, SIGNAL(calculateSurfaces()),
      this, SLOT(processSurfaceQueue()));

   // Actions for the context menu
   connect(newAction("Show Grid Info"), SIGNAL(triggered()),
      this, SLOT(showGridInfo()));
   connect(newAction("Edit Bounding Box"), SIGNAL(triggered()),
      this, SLOT(editBoundingBox()));

   setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

   m_configurator.sync();
   setConfigurator(&m_configurator);

   unsigned N(nBasis());
   m_shellValues.resize(N);
   m_shellPairValues.resize(N*(N+1)/2);
 
   m_molecularOrbitals.boundingBox(m_bbMin, m_bbMax);
   appendSurfaces(m_molecularOrbitals.surfaceList());
   computeDensityVectors();
}


double MolecularOrbitals::alphaOrbitalEnergy(unsigned const i) const 
{ 
   return m_molecularOrbitals.alphaOrbitalEnergy(i);
}


double MolecularOrbitals::betaOrbitalEnergy(unsigned const i) const 
{ 
   return m_molecularOrbitals.betaOrbitalEnergy(i);
}


void MolecularOrbitals::appendSurfaces(Data::SurfaceList& surfaceList)
{
   Data::SurfaceList::const_iterator iter;
   for (iter = surfaceList.begin(); iter != surfaceList.end(); ++iter) {
       Layer::Surface* surfaceLayer(new Layer::Surface(*(*iter)));
       connect(surfaceLayer, SIGNAL(updated()), this, SIGNAL(softUpdate()));
       appendLayer(surfaceLayer);
   }
}


void MolecularOrbitals::setMolecule(Layer::Molecule* molecule)
{
   m_molecule = molecule;
   connect(this, SIGNAL(updated()), m_molecule, SIGNAL(updated()));
   connect(this, SIGNAL(softUpdate()), m_molecule, SIGNAL(softUpdate()));
}


unsigned MolecularOrbitals::nAlpha() const 
{ 
   return m_molecularOrbitals.nAlpha(); 
}

unsigned MolecularOrbitals::nBeta() const 
{ 
   return m_molecularOrbitals.nBeta(); 
}

unsigned MolecularOrbitals::nBasis() const 
{ 
   return m_molecularOrbitals.nBasis(); 
}

unsigned MolecularOrbitals::nOrbitals() const 
{ 
   return m_molecularOrbitals.nOrbitals(); 
}


void MolecularOrbitals::computeDensityVectors()
{
   using namespace boost::numeric::ublas;

   unsigned N(nBasis());
   unsigned Na(nAlpha());
   unsigned Nb(nBeta());

   Matrix const& alphaCoefficients(m_molecularOrbitals.alphaCoefficients());
   Matrix const& betaCoefficients(m_molecularOrbitals.betaCoefficients());

   Matrix coeffs(Na, N);
   Matrix density(N, N);

   for (unsigned i = 0; i < Na; ++i) {
       for (unsigned j = 0; j < N; ++j) {
           coeffs(i,j) = alphaCoefficients(i,j);  
       }
   }

   noalias(density) = prod(trans(coeffs), coeffs);

   unsigned ka(0);
   m_alphaDensity.resize(N*(N+1)/2);
   for (unsigned i = 0; i < N; ++i) {
       m_alphaDensity[ka] = density(i,i);
       ++ka;
       for (unsigned j = i+1; j < N; ++j, ++ka) {
           m_alphaDensity[ka] = 2.0*density(i,j);
       }
   }

   coeffs.resize(Nb, N);

   for (unsigned i = 0; i < Nb; ++i) {
       for (unsigned j = 0; j < N; ++j) {
           coeffs(i,j) = betaCoefficients(i,j);
       }
   }

   noalias(density) = prod(trans(coeffs), coeffs);

   unsigned kb(0);
   m_betaDensity.resize(N*(N+1)/2);
   for (unsigned i = 0; i < N; ++i) {
       m_betaDensity[kb] = density(i,i);
       ++kb;
       for (unsigned j = i+1; j < N; ++j, ++kb) {
           m_betaDensity[kb] = 2.0*density(i,j);
       }
   }
}



void MolecularOrbitals::addToQueue(Data::SurfaceInfo const& info) 
{ 
   m_surfaceInfoQueue.append(info); 
}


void MolecularOrbitals::clearSurfaceQueue()
{ 
   m_surfaceInfoQueue.clear(); 
}


void MolecularOrbitals::showGridInfo()
{
   GridInfoDialog dialog(&m_availableGrids, m_molecule);
   dialog.exec();
}


void MolecularOrbitals::editBoundingBox()
{
   BoundingBoxDialog dialog(&m_bbMin, &m_bbMax);
   dialog.exec();
}


Data::GridData* MolecularOrbitals::findGrid(Data::SurfaceType const& type, 
   Data::GridSize const& size, Data::GridDataList const& gridList)
{
   Data::GridData* grid(0);
   Data::GridDataList::const_iterator iter;

   qDebug() << "Looking for grid";
   type.dump();
   size.dump();
   qDebug() << "List";
   gridList.dump();
  

   for (iter = gridList.begin(); iter != gridList.end(); ++iter) {
       if ( type == (*iter)->surfaceType() && size == (*iter)->size() ) {
          grid = (*iter);
          QLOG_TRACE() << "Existing Grid data found" << type.toString();
          break;
       }
   }
   return grid;
}


void MolecularOrbitals::dumpGridInfo() const
{
   Data::GridDataList::const_iterator iter;
   for (iter = m_availableGrids.begin(); iter != m_availableGrids.end(); ++iter) {
       qDebug() << "Grid info:" << (*iter)->surfaceType().toString(); 
       (*iter)->size().dump(); 
       
   }
}



QString MolecularOrbitals::description(Data::SurfaceInfo const& info, bool const tooltip)
{
   Data::SurfaceType const& type(info.type());
   QString label(type.toString());

   if (type.isOrbital()) {
      unsigned nElectrons;
      double orbitalEnergy;
      unsigned index(type.index());
      Data::SurfaceType::Kind kind(type.kind());

      if (kind == Data::SurfaceType::AlphaOrbital) {
         nElectrons = nAlpha();
         orbitalEnergy = m_molecularOrbitals.alphaOrbitalEnergy(index-1);
      }else {
         nElectrons = nBeta();
         orbitalEnergy = m_molecularOrbitals.betaOrbitalEnergy(index-1);
      }

      if (index == nElectrons-1) {
         label += " (HOMO-1)";
      }else if (index == nElectrons) {
         label += " (HOMO)";
      }else if (index == nElectrons+1) {
         label += " (LUMO)";
      }else if (index == nElectrons+2) {
         label += " (LUMO+1)";
      }

      if (tooltip) {
         label += "\nEnergy   = " + QString::number(orbitalEnergy, 'f', 3);
      }
   }

   if (tooltip) {
      label += "\nIsovalue = " + QString::number(info.isovalue(), 'f', 3);
   }
 

   return label;
}


void MolecularOrbitals::processSurfaceQueue()
{
   GridQueue gridQueue;

   // First, do an initial pass to determine what data needs to be calculated
   SurfaceInfoQueue::iterator iter;
   for (iter = m_surfaceInfoQueue.begin(); iter != m_surfaceInfoQueue.end(); ++iter) {
       Data::SurfaceType type((*iter).type());
       Data::GridSize size(m_bbMin, m_bbMax, (*iter).quality());
       Data::GridData* grid(findGrid(type, size, m_availableGrids));

qDebug() << "Processing surface queue" << type.toString();
       if (!grid) {
          gridQueue.append(qMakePair(type, size));
qDebug() << "Appending grid to queue";
       }else {
qDebug() << "Grid already exists";
       }
   }

   // Second, calculate any grid data that we haven't got
   try {
      if (!gridQueue.isEmpty()) {
         if (!processGridQueue(gridQueue)) {
            clearSurfaceQueue();
            return;
         }
      }
   } catch (...) {
      QMsgBox::warning(0, "IQmol", "Problem calculating grid data");
      clearSurfaceQueue();
   }

   // Third, calculate requested surfaces
   QProgressDialog* progressDialog(new QProgressDialog("Calculating Surfaces", 
      "Cancel", 0, m_surfaceInfoQueue.count()));
      
   int progress(0);
   progressDialog->setValue(progress);
   progressDialog->setWindowModality(Qt::WindowModal);
   progressDialog->show();

   Qt::CheckState checked(Qt::Checked);
   for (iter = m_surfaceInfoQueue.begin(); iter != m_surfaceInfoQueue.end(); ++iter) {
       Data::Surface* surfaceData(generateSurface(*iter));

       if (surfaceData) {
          m_molecularOrbitals.appendSurface(surfaceData);

          Layer::Surface* surfaceLayer(new Layer::Surface(*surfaceData));
          if (surfaceLayer) {
             surfaceLayer->setCheckState(checked);
             checked = Qt::Unchecked;
             connect(surfaceLayer, SIGNAL(updated()), this, SIGNAL(softUpdate()));
             if (m_molecule) {
                surfaceLayer->setFrame(m_molecule->getReferenceFrame());
             }

             surfaceLayer->setText(description(*iter, false));
             surfaceLayer->setToolTip(description(*iter, true));

             appendLayer(surfaceLayer);
          }
       }

       ++progress;
       progressDialog->setValue(progress);
       if (progressDialog->wasCanceled()) break;
   }

#ifndef Q_OS_WIN32
   // Deleting this under Windows causes a crash, go figure.
   // actually it is not just windows.
   //delete progressDialog;
#endif

   clearSurfaceQueue();
   updated(); 
}

 
bool MolecularOrbitals::processGridQueue(GridQueue const& gridQueue)
{
   // First obtain a list of the unique grid sizes
   std::set<Data::GridSize> sizes;
   GridQueue::const_iterator queued; 
   for (queued = gridQueue.begin(); queued != gridQueue.end(); ++queued) {
       sizes.insert(queued->second);    
   }

   // Second, determine what data is required for each grid size
   QLOG_TRACE() << "Computing data for" << gridQueue.size() <<"grids";
   QLOG_TRACE() << "There are" << sizes.size() << "different grid sizes";
   std::set<Data::GridSize>::iterator size;

   for (size = sizes.begin(); size != sizes.end(); ++size) {
       std::set<Data::SurfaceType> densities;
       std::set<Data::SurfaceType> alphaOrbitals;
       std::set<Data::SurfaceType> betaOrbitals;
       
       for (queued = gridQueue.begin(); queued != gridQueue.end(); ++queued) {
           if (queued->second == *size) {
              Data::SurfaceType type(queued->first);

              if (type.isDensity()) {
                 densities.insert(type);
              }else if (type.kind() == Data::SurfaceType::AlphaOrbital) {
                 alphaOrbitals.insert(type);
              }else if (type.kind() == Data::SurfaceType::BetaOrbital) {
                 betaOrbitals.insert(type); 
              }else  {
                 QLOG_WARN() << "Unknown Grid type found in processQueue";
              }
           }
       }

       if (densities.size() > 0) {
          QLOG_TRACE() << "Computing" << densities.size() << "density grids";
          Data::SurfaceType alpha(Data::SurfaceType::AlphaDensity);
          Data::GridData*   alphaGrid = new Data::GridData(*size, alpha);
          Data::SurfaceType beta(Data::SurfaceType::BetaDensity);
          Data::GridData*   betaGrid  = new Data::GridData(*size, beta);

          if (!computeDensityGrids(alphaGrid, betaGrid)) {
             // user canceled the action
             delete alphaGrid;
             delete betaGrid;
             return false;
          }

          m_availableGrids.append(alphaGrid);
          m_availableGrids.append(betaGrid);

          Data::GridData* spinGrid  = new Data::GridData(*alphaGrid);
          *spinGrid -= *betaGrid;
          spinGrid->setSurfaceType(Data::SurfaceType::SpinDensity);

          Data::GridData* totalGrid = new Data::GridData(*alphaGrid);
          *totalGrid += *betaGrid;
          totalGrid->setSurfaceType(Data::SurfaceType::TotalDensity);

          m_availableGrids.append(spinGrid);
          m_availableGrids.append(totalGrid);
       }

       Data::GridDataList grids;
       std::set<Data::SurfaceType>::iterator iter;
       for (iter = alphaOrbitals.begin(); iter != alphaOrbitals.end(); ++iter) {
           Data::GridData* grid = new Data::GridData(*size, *iter);
           grids.append(grid); 
       }

       if (grids.count() > 0) {
          QLOG_TRACE() << "Computing" << grids.size() << "alpha orbitals";
          if (computeOrbitalGrids(grids)) {
             m_availableGrids += grids;
          }else {
             for (int i = 0; i <  grids.size(); ++i) {
                 delete grids[i];
             }
             return false;
          }
       }

       grids.clear();
       for (iter = betaOrbitals.begin(); iter != betaOrbitals.end(); ++iter) {
           Data::GridData* grid = new Data::GridData(*size, *iter);
           grids.append(grid); 
       }

       if (grids.count() > 0) {
          QLOG_TRACE() << "Computing" << grids.size() << "beta orbitals";
          if (computeOrbitalGrids(grids)) {
             m_availableGrids += grids;
          }else {
             for (int i = 0; i <  grids.size(); ++i) {
                 delete grids[i];
             }
             return false;
          }
       }
       grids.clear();
   }

   return true;
}


Data::Surface* MolecularOrbitals::generateSurface(Data::SurfaceInfo const& surfaceInfo)
{
   QTime time;
   time.start();

   Data::SurfaceType type(surfaceInfo.type());
   Data::GridSize size(m_bbMin, m_bbMax, surfaceInfo.quality());
   Data::GridData* grid(findGrid(type, size, m_availableGrids));

   double delta(Data::GridSize::stepSize(surfaceInfo.quality()));

   // If the grid data is not found, it is probably because the user quit the
   // calculation or edited the bounding box.
   if (!grid)  return 0;

   MarchingCubes mc(*grid);
   Data::Surface* surfaceData(new Data::Surface(surfaceInfo));
   if (surfaceData) {
      mc.generateMesh(surfaceInfo.isovalue(), surfaceData->meshPositive());

      if (surfaceInfo.simplifyMesh()) {
         MeshDecimator decimator(surfaceData->meshPositive());
         if (!decimator.decimate(delta)) {
            QLOG_ERROR() << "Mesh decimation failed:" << decimator.error();
         }
      }


      if (type.isSigned()) {
         mc.generateMesh(-surfaceInfo.isovalue(), surfaceData->meshNegative());
         if (surfaceInfo.simplifyMesh()) {
            MeshDecimator decimator(surfaceData->meshNegative());
            if (!decimator.decimate(delta)) {
               QLOG_ERROR() << "Mesh decimation failed:" << decimator.error();
            }
         }
      }

      double t = time.elapsed() / 1000.0;
      QLOG_INFO() << "Time to compute surface" 
                  << surfaceInfo.toString() << ":" << t << "seconds";
   }

   return surfaceData;
}


// This requires all the Grids be of the same size and all the orbitals to be
// of the same spin.  Returns true only if something was calculated.
bool MolecularOrbitals::computeOrbitalGrids(Data::GridDataList& grids)
{
   if (grids.isEmpty()) return false;;

   // Check that the grids are all of the same size and Spin
   Data::GridData* g0(grids[0]);
   QList<int> orbitals;
   Data::GridDataList::iterator iter;

   for (iter = grids.begin(); iter != grids.end(); ++iter) {
       QLOG_DEBUG() << "Computing grid" << (*iter)->surfaceType().toString() ;
       (*iter)->size().dump();
       if ( ((*iter)->size() != g0->size()) ) {
          QLOG_ERROR() << "Different sized grids found in molecular orbitals calculator";
          return false;
       }
       if ( ((*iter)->surfaceType().kind() != Data::SurfaceType::AlphaOrbital) &&
            ((*iter)->surfaceType().kind() != Data::SurfaceType::BetaOrbital) ) {
          QLOG_ERROR() << "Incorrect grid type found in molecular orbitals calculator";
          QLOG_ERROR() << (*iter)->surfaceType().toString(); 
          return false;
       }
       orbitals.append((*iter)->surfaceType().index()-1);
   }

   QTime time;
   time.start();

   Matrix const* coefficients;
   if (g0->surfaceType().kind() == Data::SurfaceType::AlphaOrbital) {
      QLOG_TRACE() << "Setting MO coefficient data to Alpha";
      coefficients = &(m_molecularOrbitals.alphaCoefficients());
   }else {
      QLOG_TRACE() << "Setting MO coefficient data to Beta";
      coefficients = &(m_molecularOrbitals.betaCoefficients());
   }
   
   unsigned nOrb(orbitals.size());
   unsigned nx, ny, nz;
   g0->getNumberOfPoints(nx, ny, nz);
   Vec delta(g0->delta());
   Vec origin(g0->origin());

   QProgressDialog* progressDialog(new QProgressDialog("Calculating orbital grid data", 
       "Cancel", 0, nx));
       
   int progress(0);

   progressDialog->setValue(progress);
   progressDialog->setWindowModality(Qt::WindowModal);
   progressDialog->show();

   double  x, y, z;
   double* values;
   double* tmp = new double[nOrb];
   unsigned i, j, k;

   Data::ShellList const& shells(m_molecularOrbitals.shellList());
   Data::ShellList::const_iterator shell;

   for (i = 0, x = origin.x;  i < nx;  ++i, x += delta.x) {
       for (j = 0, y = origin.y;  j < ny;  ++j, y += delta.y) {
           for (k = 0, z = origin.z;  k < nz;  ++k, z += delta.z) {
   
               Vec gridPoint(x,y,z);

               for (unsigned orb = 0; orb < nOrb; ++orb) tmp[orb] = 0.0;
               unsigned offset(0);

               //-----------------------------------------------------
               for (shell = shells.begin(); shell != shells.end(); ++shell) {
                   if ( (values = (*shell)->evaluate(gridPoint)) ) {
                      for (unsigned s = 0; s < (*shell)->nBasis(); ++s) {
                          for (unsigned orb = 0; orb < nOrb; ++orb) {
                              tmp[orb] += (*coefficients)(orbitals[orb], offset) * values[s];
                          }
                          ++offset;
                      }
                   }else {
                      offset += (*shell)->nBasis();
                   }
               }

               for (unsigned orb = 0; orb < nOrb; ++orb) {
                   (*grids.at(orb))(i, j, k) = tmp[orb];
               }
               //-----------------------------------------------------
           }
       }

       ++progress;
       progressDialog->setValue(progress);
       if (progressDialog->wasCanceled()) {
          delete [] tmp;
#ifndef Q_OS_WIN32
          //delete progressDialog;
#endif
          return false;
       }
   }

   delete [] tmp;

   double t = time.elapsed() / 1000.0;
   QLOG_INFO() << "Time to compute orbital grid data:" << t << "seconds";

   return true;
}


bool MolecularOrbitals::computeDensityGrids(Data::GridData*& alpha, Data::GridData*& beta)
{
   QTime time;
   time.start();

   unsigned nx, ny, nz;
   alpha->getNumberOfPoints(nx, ny, nz);
   Vec delta(alpha->delta());
   Vec origin(alpha->origin());

   // We take a two pass approach, the first computes data on a grid with half
   // the number of points for each dimension (so a factor of 8 fewer points
   // than the target grid).  We then used these values in a subsequent pass to
   // refine only those parts with significant density.

   unsigned totalProgress(8*nx);  // first and second passes
   unsigned progress(0);
   unsigned i, j, k;
   double x, y, z;

   QProgressDialog progressDialog("Calculating density grid data", "Cancel", 0, 
       totalProgress, QApplication::activeWindow());
   progressDialog.setValue(progress);
   progressDialog.setWindowModality(Qt::WindowModal);
   progressDialog.show();

   for (i = 0, x = origin.x; i < nx; i += 2, x += 2.0*delta.x) {
       for (j = 0, y = origin.y; j < ny; j += 2, y += 2.0*delta.y) {
           for (k = 0, z = origin.z; k < nz; k += 2, z += 2.0*delta.z) {
               Vec gridPoint(x, y, z);
               computeShellPairs(gridPoint);
               (*alpha)(i, j, k) = inner_prod(m_alphaDensity, m_shellPairValues);
               (*beta )(i, j, k) = inner_prod(m_betaDensity,  m_shellPairValues);
           }
       }

       ++progress;
       progressDialog.setValue(progress);
       if (progressDialog.wasCanceled()) return false;
   }

   double a000, a001, a010, a011, a100, a101, a110, a111, aTot;
   double b000, b001, b010, b011, b100, b101, b110, b111, bTot;
   double thresh(0.125*Data::Shell::thresh());

   origin += delta;

   for (i = 1, x = origin.x;  i < nx-1;  i += 2, x += 2*delta.x) {
       for (j = 1, y = origin.y;  j < ny-1;  j += 2, y += 2*delta.y) {
           for (k = 1, z = origin.z;  k < nz-1;  k += 2, z += 2*delta.z) {

               a000 = (*alpha)(i-1, j-1, k-1);
               a001 = (*alpha)(i-1, j-1, k+1);
               a010 = (*alpha)(i-1, j+1, k-1);
               a011 = (*alpha)(i-1, j+1, k+1);
               a100 = (*alpha)(i+1, j-1, k-1);
               a101 = (*alpha)(i+1, j-1, k+1);
               a110 = (*alpha)(i+1, j+1, k-1);
               a111 = (*alpha)(i+1, j+1, k+1);
               aTot = a000+a001+a010+a011+a100+a101+a110+a111;

               b000 = (*beta)(i-1, j-1, k-1);
               b001 = (*beta)(i-1, j-1, k+1);
               b010 = (*beta)(i-1, j+1, k-1);
               b011 = (*beta)(i-1, j+1, k+1);
               b100 = (*beta)(i+1, j-1, k-1);
               b101 = (*beta)(i+1, j-1, k+1);
               b110 = (*beta)(i+1, j+1, k-1);
               b111 = (*beta)(i+1, j+1, k+1);
               bTot = b000+b001+b010+b011+b100+b101+b110+b111;

               if (std::abs(aTot) > thresh || std::abs(bTot) > thresh) {


                  computeShellPairs(Vec(x, y, z));
                  (*alpha)(i,  j,  k  ) = inner_prod(m_alphaDensity, m_shellPairValues);
                  (*beta )(i,  j,  k  ) = inner_prod(m_betaDensity,  m_shellPairValues);

                  computeShellPairs(Vec(x, y, z-delta.z));
                  (*alpha)(i,  j,  k-1) = inner_prod(m_alphaDensity, m_shellPairValues);
                  (*beta )(i,  j,  k-1) = inner_prod(m_betaDensity,  m_shellPairValues);

                  computeShellPairs(Vec(x, y-delta.y, z));
                  (*alpha)(i,  j-1,k  ) = inner_prod(m_alphaDensity, m_shellPairValues);
                  (*beta )(i,  j-1,k  ) = inner_prod(m_betaDensity,  m_shellPairValues);

                  computeShellPairs(Vec(x, y-delta.y, z-delta.z));
                  (*alpha)(i,  j-1,k-1) = inner_prod(m_alphaDensity, m_shellPairValues);
                  (*beta )(i,  j-1,k-1) = inner_prod(m_betaDensity,  m_shellPairValues);

                  computeShellPairs(Vec(x-delta.x, y, z));
                  (*alpha)(i-1,j,  k  ) = inner_prod(m_alphaDensity, m_shellPairValues);
                  (*beta )(i-1,j,  k  ) = inner_prod(m_betaDensity,  m_shellPairValues);

                  computeShellPairs(Vec(x-delta.x, y, z-delta.z));
                  (*alpha)(i-1,j,  k-1) = inner_prod(m_alphaDensity, m_shellPairValues);
                  (*beta )(i-1,j,  k-1) = inner_prod(m_betaDensity,  m_shellPairValues);

                  computeShellPairs(Vec(x-delta.x, y-delta.y, z));
                  (*alpha)(i-1,j-1,k  ) = inner_prod(m_alphaDensity, m_shellPairValues);
                  (*beta )(i-1,j-1,k  ) = inner_prod(m_betaDensity,  m_shellPairValues);
                  
               }else {

                  (*alpha)(i,  j,  k  ) = 0.125*aTot;
                  (*beta )(i,  j,  k  ) = 0.125*bTot;

                  (*alpha)(i,  j,  k-1) = 0.25*(a000+a010+a100+a110);
                  (*beta )(i,  j,  k-1) = 0.25*(b000+b010+b100+b110);

                  (*alpha)(i,  j-1,k  ) = 0.25*(a000+a001+a100+a101);
                  (*beta )(i,  j-1,k  ) = 0.25*(b000+b001+b100+b101);

                  (*alpha)(i,  j-1,k-1) = 0.50*(a000+a100);
                  (*beta )(i,  j-1,k-1) = 0.50*(b000+b100);

                  (*alpha)(i-1,j,  k  ) = 0.25*(a000+a001+a010+a011);
                  (*beta )(i-1,j,  k  ) = 0.25*(b000+b001+b010+b011);

                  (*alpha)(i-1,j,  k-1) = 0.50*(a000+a010);
                  (*beta )(i-1,j,  k-1) = 0.50*(b000+b010);

                  (*alpha)(i-1,j-1,k  ) = 0.50*(a000+a001);
                  (*beta )(i-1,j-1,k  ) = 0.50*(b000+b001);

               }
            }
       }

       progress += 7;
       progressDialog.setValue(progress);
       if (progressDialog.wasCanceled()) return false;
   }

   double t = time.elapsed() / 1000.0;
   QLOG_INFO() << "Time to compute density grid data:" << t << "seconds";

   return true;
}


void MolecularOrbitals::computeShellPairs(Vec const& gridPoint)
{
   Data::ShellList const& shells(m_molecularOrbitals.shellList());

   double* values;
   Data::ShellList::const_iterator shell;
   unsigned k(0);
   for (shell = shells.begin(); shell != shells.end(); ++shell) {
       if ( (values = (*shell)->evaluate(gridPoint)) ) {
          for (unsigned j = 0; j < (*shell)->nBasis(); ++j, ++k) {
              m_shellValues[k] = values[j];
          }
       }else {
          for (unsigned j = 0; j < (*shell)->nBasis(); ++j, ++k) {
              m_shellValues[k] = 0.0;
          }
       }
   }

   k = 0;
   double xi, xj;
   unsigned N(nBasis());
   for (unsigned i = 0; i < N; ++i) {
       xi = m_shellValues[i];
       m_shellPairValues[k] = xi*xi;
       ++k;
       for (unsigned j = i+1; j < N; ++j, ++k) {
           xj = m_shellValues[j];
           m_shellPairValues[k] = xi*xj;
       }
   }
}

} } // end namespace IQmol::Layer
