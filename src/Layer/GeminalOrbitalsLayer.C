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

#include "GeminalOrbitalsLayer.h"
#include "GeminalOrbitals.h"
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

GeminalOrbitals::GeminalOrbitals(Data::GeminalOrbitals& molecularOrbitals)
 : Base("Geminal Orbitals"), m_configurator(*this), m_geminalOrbitals(molecularOrbitals)
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
 
   m_geminalOrbitals.boundingBox(m_bbMin, m_bbMax);
   appendSurfaces(m_geminalOrbitals.surfaceList());
}


GeminalOrbitals::~GeminalOrbitals()
{
   for (int i = 0; i < m_densityVectors.size(); ++i) {
       delete m_densityVectors[i];
   }
   m_densityVectors.clear();
}


void GeminalOrbitals::appendSurfaces(Data::SurfaceList& surfaceList)
{
   Data::SurfaceList::const_iterator iter;
   for (iter = surfaceList.begin(); iter != surfaceList.end(); ++iter) {
       Layer::Surface* surfaceLayer(new Layer::Surface(*(*iter)));
       connect(surfaceLayer, SIGNAL(updated()), this, SIGNAL(softUpdate()));
       appendLayer(surfaceLayer);
   }
}


void GeminalOrbitals::setMolecule(Layer::Molecule* molecule)
{
   m_molecule = molecule;
   connect(this, SIGNAL(updated()), m_molecule, SIGNAL(updated()));
   connect(this, SIGNAL(softUpdate()), m_molecule, SIGNAL(softUpdate()));
}


unsigned GeminalOrbitals::nAlpha() const 
{ 
   return m_geminalOrbitals.nAlpha(); 
}

unsigned GeminalOrbitals::nBeta() const 
{ 
   return m_geminalOrbitals.nBeta(); 
}

unsigned GeminalOrbitals::nBasis() const 
{ 
   return m_geminalOrbitals.nBasis(); 
}

unsigned GeminalOrbitals::nOrbitals() const 
{ 
   return m_geminalOrbitals.nOrbitals(); 
}

unsigned GeminalOrbitals::nGeminals() const 
{ 
// Is this the most useful definition of the number of geminals?
   return m_geminalOrbitals.nBeta(); 
}


unsigned GeminalOrbitals::nOpenShell() const 
{ 
   return m_geminalOrbitals.nAlpha()-m_geminalOrbitals.nBeta(); 
}


double GeminalOrbitals::geminalOrbitalEnergy(unsigned const i) const 
{ 
   return m_geminalOrbitals.geminalOrbitalEnergy(i);
}


void GeminalOrbitals::addToQueue(Data::SurfaceInfo const& info) 
{ 
   m_surfaceInfoQueue.append(info); 
}


void GeminalOrbitals::clearSurfaceQueue()
{ 
   m_surfaceInfoQueue.clear(); 
}


void GeminalOrbitals::showGridInfo()
{
   GridInfoDialog dialog(&m_availableGrids, m_molecule->text(), 
      m_molecule->coordinatesForCubeFile());
   dialog.exec();
}


void GeminalOrbitals::editBoundingBox()
{
   BoundingBoxDialog dialog(&m_bbMin, &m_bbMax);
   dialog.exec();
}


Data::GridData* GeminalOrbitals::findGrid(Data::SurfaceType const& type, 
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


void GeminalOrbitals::dumpGridInfo() const
{
   Data::GridDataList::const_iterator iter;
   for (iter = m_availableGrids.begin(); iter != m_availableGrids.end(); ++iter) {
       qDebug() << "Grid info:" << (*iter)->surfaceType().toString(); 
       (*iter)->size().dump(); 
       
   }
}


QString GeminalOrbitals::description(Data::SurfaceInfo const& info, bool const tooltip)
{
   Data::SurfaceType const& type(info.type());
   QString label(type.toString());

   if (type.kind() == Data::SurfaceType::Geminal) {
      unsigned index(type.index());
      //unsigned nGeminals(nBeta());

      if (index == nBeta()-1) {
         label += " (HOG-1)";
      }else if (index == nBeta()) {
         label += " (HOG)";
      }else if (index > nBeta()) {
         label += " (OS)";
      }

      if (tooltip) {
         double geminalEnergy(m_geminalOrbitals.geminalOrbitalEnergy(index-1));
         label += "\nEnergy   = " + QString::number(geminalEnergy, 'f', 3);
      }
   }

   if (tooltip) {
      label += "\nIsovalue = " + QString::number(info.isovalue(), 'f', 3);
   }
 
   return label;
}


void GeminalOrbitals::processSurfaceQueue()
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
          m_geminalOrbitals.appendSurface(surfaceData);

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

 
bool GeminalOrbitals::processGridQueue(GridQueue const& gridQueue)
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
       std::set<Data::SurfaceType> geminals;
       
       for (queued = gridQueue.begin(); queued != gridQueue.end(); ++queued) {
           if (queued->second == *size) {
              Data::SurfaceType type(queued->first);

              if (type.isDensity()) {
                 densities.insert(type);
              }else if (type.kind() == Data::SurfaceType::Geminal) {
                 geminals.insert(type);
              }else  {
                 QLOG_WARN() << 
                     "Unknown Grid type found in Layer::GeminalOrbitals::processQueue";
              }
           }
       }

       // Allocate density grids
       Data::GridDataList densityGrids;
       std::set<Data::SurfaceType>::iterator iter;
       for (iter = densities.begin(); iter != densities.end(); ++iter) {
           Data::GridData* grid = new Data::GridData(*size, *iter);
           densityGrids.append(grid); 
       }

       if (densityGrids.count() > 0) {
          QLOG_TRACE() << "Computing" << densityGrids.size() << "Geminal Densities";
          if (computeDensityGrids(densityGrids)) {
             m_availableGrids += densityGrids;
          }else {
             for (int i = 0; i <  densityGrids.size(); ++i) {
                 delete densityGrids[i];
             }
             return false;
          }
       }

       // Allocate geminal grids
       Data::GridDataList geminalGrids;
       for (iter = geminals.begin(); iter != geminals.end(); ++iter) {
           Data::GridData* grid = new Data::GridData(*size, *iter);
           geminalGrids.append(grid); 
       }

       if (geminalGrids.count() > 0) {
          QLOG_TRACE() << "Computing" << geminalGrids.size() << "Geminals";
          if (computeOrbitalGrids(geminalGrids)) {
             m_availableGrids += geminalGrids;
          }else {
             for (int i = 0; i <  geminalGrids.size(); ++i) {
                 delete geminalGrids[i];
             }
             return false;
          }
       }

       geminalGrids.clear();
   }

   return true;
}


Data::Surface* GeminalOrbitals::generateSurface(Data::SurfaceInfo const& surfaceInfo)
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
bool GeminalOrbitals::computeOrbitalGrids(Data::GridDataList& grids)
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
       if ( ((*iter)->surfaceType().kind() != Data::SurfaceType::Geminal) ) {
          QLOG_ERROR() << "Incorrect grid type found in molecular orbitals calculator";
          QLOG_ERROR() << (*iter)->surfaceType().toString(); 
          return false;
       }
       orbitals.append((*iter)->surfaceType().index()-1);
   }

   QTime time;
   time.start();

   Matrix const* coefficientsA;
   //Matrix const* coefficientsB;
   //QList<double> const* coefficientsG;

/* MOGem is never used
   QList<int> const* MOGem;
      coefficientsA = &(m_geminalOrbitals.alphaCoefficients());
      //coefficientsB = &(m_geminalOrbitals.betaCoefficients());
      //coefficientsG = &(m_geminalOrbitals.geminalCoefficients());
      MOGem = &(m_geminalOrbitals.geminalMoMap());
*/

   
   unsigned nOrb(orbitals.size());
   unsigned nGem = m_geminalOrbitals.nAlpha();
   //unsigned nGemOrb = m_geminalOrbitals.nOrbitals();
   //unsigned nOS = m_geminalOrbitals.nAlpha() - m_geminalOrbitals.nBeta();
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
   double* tmp = new double[nGem];
   unsigned i, j, k;
   
   Data::ShellList const& shells(m_geminalOrbitals.shellList());
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
                              tmp[orb] += (*coefficientsA)(orbitals[orb], offset) * values[s]; //VAR temp
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


// This function is in charge of computing *all* the density vectors
void GeminalOrbitals::computeDensityVectors()
{
   if (!m_densityVectors.isEmpty()) return;
   using namespace boost::numeric::ublas;

   Matrix const& alphaCoefficients(m_geminalOrbitals.alphaCoefficients());
   Matrix const& betaCoefficients(m_geminalOrbitals.betaCoefficients());
   QList<double> const& geminalCoefficients(m_geminalOrbitals.geminalCoefficients());
   //QList<int>    const& geminalMoMap(m_geminalOrbitals.geminalMoMap());
   QList<unsigned> const& Limits(m_geminalOrbitals.geminalOrbitalLimits());

   unsigned N(nBasis());
   // unused (?)
   // unsigned Na(nAlpha());
   unsigned Nb(nBeta());
   unsigned nGemOrb(nOrbitals());
   // unused (?)
   //unsigned nOS=Na-Nb;
   unsigned nGeminals(nAlpha());
   unsigned NV = N*(N+1)/2;
   unsigned i,j,n1,n;
   
   //unsigned* GemL = new unsigned[nGeminals+1];
   double* GemD = new double[nGemOrb];
   //Each geminal has a subset of orbitals associated with it, with limits stored in GemL
   // For geminal j it is from GemL[j] to GemL[j+1].  We populate GemL below by looking
   // through MOGem which contains a geminal index of each orbital
//    GemL[0] = 0; j = 1;
//    for (i = 1; i < nGemOrb; i++) { 
//     if ( geminalMoMap[i] == j ) { GemL[j] = i; j++; }
//    }
//    GemL[nGeminals] = nGemOrb;
//    // Open-shells need a separate assignement, as they are lumped toghether in MOGem
//    if (nOS > 1) { for (i = nGeminals - nOS  + 1; i < nGeminals + 1; i++) { GemL[i] = GemL[i-1] + 1; } }
   std::cout<<" Debugging Limits list for "<<nGeminals<<" geminals and "<<nGemOrb<<" orbitals \n";
   for(i=0; i<nGeminals+1; i++){std::cout<<Limits[i]<<" ";} std::cout<<"\n";
   
   //for (n=0; n<nGeminals+1; n++) { m_geminalOrbitalLimits.append(GemL[n]);}
   for (n = 0; n<nGemOrb; n++) {GemD[n] = geminalCoefficients[n] * geminalCoefficients[n]; }

   for (n = 0; n < nGeminals; ++n) {
     
       Vector density(N*(N+1)/2); 
       
       for (i = 0; i < NV; i++) { density(i) = 0.; }

       for (n1 = Limits[n]; n1 < Limits[n+1]; n1++) {
         if (n < Nb) {
           for (i = 0; i < N; i++) {
             for (j = 0; j <i; j++) { 
	      density(i*(i+1)/2+j) += 2*GemD[n1]*(alphaCoefficients(n1,i)*alphaCoefficients(n1,j)+betaCoefficients(n1,i)*betaCoefficients(n1,j));
	     }
	     density(i*(i+3)/2) += GemD[n1]*(alphaCoefficients(n1,i)*alphaCoefficients(n1,i)+betaCoefficients(n1,i)*betaCoefficients(n1,i));
           }
	 } else {
	    for (i = 0; i < N; i++) {
             for (j = 0; j <i; j++) { 
	       density(i*(i+1)/2+ j) += 2*alphaCoefficients(n1,i)*alphaCoefficients(n1,j);
	     }
	     density(i*(i+3)/2) += alphaCoefficients(n1,i)*alphaCoefficients(n1,i);
           }
	 }
   
       }
       
       //std::cout<<" Debug. Density matrix check for geminal "<<n+1<<"\n";
       //for(i=0;i<N;i++){for(j=0;j<i+1;j++){std::cout<<i<<","<<j<<" "<<density(i*(i+1)/2+ j)<<"\n";}}

       // copying from normal triangular to scanned-rows ordering, storing in list of densities
       Vector* vector(new Vector(N*(N+1)/2));
       m_densityVectors.append(vector);

     for (i = 0, n1 = 0; i < N; ++i) {
         for (j = i; j < N; ++j, ++n1) {
           (*vector)[n1] = density(j*(j+1)/2+i);
         }
      }
   }
   
   // While we are here we set up the GeminalOrbitalProperties for coloring the
   // surfaces.
   initGeminalOrbitalProperties();

   //delete [] GemL; 
   delete [] GemD; 
 
}


// This sets up the orbital SpatialProperties objects for evaluating the
// orbitals on a surface.
void GeminalOrbitals::initGeminalOrbitalProperties()
{
   if (!m_molecule) {
      QLOG_WARN() << "Molecule uninitialized in initOrbitalProperties()";
      return;
   }

   unsigned i;
   for (i = 0; i < nAlpha(); ++i) {
       m_molecule->addProperty(new GeminalOrbitalProperty(m_geminalOrbitals, i));
   }
   for (i = 0; i < nBeta(); ++i) {
       m_molecule->addProperty(new GeminalOrbitalProperty(m_geminalOrbitals, i+nAlpha()));
   }
}


bool GeminalOrbitals::computeDensityGrids(Data::GridDataList& grids)
{
   if (grids.isEmpty()) return false;;

   QTime time;
   time.start();

   // Check that the grids are all of the same size
   Data::GridData* g0(grids.first());
   Data::GridDataList::iterator iter;

   // Ensure the required density matrices are available
   computeDensityVectors();

   // This will contain a list of pointers to the appropriate density vectors 
   // for each of the requested grids, in order.
   QList<Vector*> densityVectorPointers;

   for (iter = grids.begin(); iter != grids.end(); ++iter) {
       QLOG_DEBUG() << "Computing grid" << (*iter)->surfaceType().toString() ;
       (*iter)->size().dump();
       if ( ((*iter)->size() != g0->size()) ) {
          QLOG_ERROR() << "Different sized grids found in geminal calculator";
          return false;
       }
       if ( !(*iter)->surfaceType().isDensity()) {
          QLOG_ERROR() << "Incorrect grid type found in geminal calculator";
          QLOG_ERROR() << (*iter)->surfaceType().toString(); 
          return false;
       }
       unsigned index((*iter)->surfaceType().index());
       densityVectorPointers.append(m_densityVectors[index-1]);
   }

   unsigned nx, ny, nz;
   g0->getNumberOfPoints(nx, ny, nz);
   Vec delta(g0->delta());
   Vec origin(g0->origin());
   unsigned nDensities(grids.size());
  
   unsigned totalProgress(nx);
   unsigned progress(0);
   unsigned i, j, k;
   double   x, y, z;

   QProgressDialog progressDialog("Calculating density grid data", "Cancel", 0, 
       totalProgress, QApplication::activeWindow());
   progressDialog.setValue(progress);
   progressDialog.setWindowModality(Qt::WindowModal);
   progressDialog.show();
   
 
//    k=0;  std::cout<<" Debug of grids.  Assume only one is computed at a time \n";
//    for (i = 0; i < 6; ++i) {
//        std::cout<<i<<" "<<i<<" "<<(*densityVectorPointers[0])[k]<<"\n";
//        ++k;
//        for (j = i+1; j < 6; ++j, ++k) {
//            std::cout<<i<<" "<<j<<" "<<(*densityVectorPointers[0])[k]<<"\n";
//        }
//    }

   for (i = 0, x = origin.x; i < nx; i += 1, x += delta.x) {
       for (j = 0, y = origin.y; j < ny; j += 1, y += delta.y) {
           for (k = 0, z = origin.z; k < nz; k += 1, z += delta.z) {
               Vec gridPoint(x, y, z);
               computeShellPairs(gridPoint);

               for (unsigned den = 0; den < nDensities; ++den) {
                   (*grids.at(den))(i, j, k) = sqrt(fabs(
                       inner_prod(*densityVectorPointers[den], m_shellPairValues)       //;
						    ));
               }

           }
       }

       ++progress;
       progressDialog.setValue(progress);
       if (progressDialog.wasCanceled()) return false;
   }

   double t = time.elapsed() / 1000.0;
   QLOG_INFO() << "Time to compute density grid data:" << t << "seconds";

   return true;
}


void GeminalOrbitals::computeShellPairs(Vec const& gridPoint)
{
   Data::ShellList const& shells(m_geminalOrbitals.shellList());

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


// ------------------------------------------------------------------------
//  m_geminalOrerty SpatialProperty
// ------------------------------------------------------------------------

GeminalOrbitalProperty::GeminalOrbitalProperty(Data::GeminalOrbitals const& geminalOrbitals, 
   unsigned const index)
   : m_geminalOrbitals(geminalOrbitals), m_alpha(geminalOrbitals.alphaCoefficients()),  
   m_beta(geminalOrbitals.betaCoefficients()),m_geminals(geminalOrbitals.geminalCoefficients()),
   m_index(index)
   
{
  
  //QList<unsigned> const& Limits(geminalOrbitals.geminalOrbitalLimits());
  
  if(index < geminalOrbitals.nAlpha()){
   setText("Geminal Alpha " + QString::number(index+1));
  } else {
    setText("Geminal Beta " + QString::number(index+1-geminalOrbitals.nAlpha()));
 }
  m_function = boost::bind(&GeminalOrbitalProperty::orbital, this, _1, _2, _3);
}


double GeminalOrbitalProperty::orbital(double const x, double const y, double const z) const
{
  
   Vec gridPoint(x,y,z);
   unsigned offset(0),i,s;
   double*  values;
   double   val(0.0);

   QList<unsigned> const& Limits(m_geminalOrbitals.geminalOrbitalLimits());
   Data::ShellList const& shells(m_geminalOrbitals.shellList());
   Data::ShellList::const_iterator shell;
   unsigned NA = m_geminalOrbitals.nAlpha();
   
   //Limits[6] m_alpha(i,j) m_index

   if( m_index < NA ){ 
     for (shell = shells.begin(); shell != shells.end(); ++shell) {
       if ( (values = (*shell)->evaluate(gridPoint)) ) {
          for ( s = 0; s < (*shell)->nBasis(); ++s) {
	    for (i = Limits[m_index]; i < Limits[m_index+1]; i++) {
               val += m_alpha(i,offset) * m_geminals[i] * values[s];
	    }
            ++offset;
          }
       } else {
          offset += (*shell)->nBasis();
       }
     }
   } else {
     for (shell = shells.begin(); shell != shells.end(); ++shell) {
       if ( (values = (*shell)->evaluate(gridPoint)) ) {
          for ( s = 0; s < (*shell)->nBasis(); ++s) {
	    for (i = Limits[m_index-NA]; i < Limits[m_index+1-NA]; i++) {
               val += m_beta(i,offset) * m_geminals[i] * values[s];
	    }
            ++offset;
          }
       } else {
          offset += (*shell)->nBasis();
       }
     }
   }

   return val;
}


} } // end namespace IQmol::Layer
