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

#include "OrbitalsLayer.h"
#include "Orbitals.h"
#include "MoleculeLayer.h"
#include "GridInfoDialog.h"
#include "MarchingCubes.h"
#include "MeshDecimator.h"
#include "BoundingBoxDialog.h"
#include "SurfaceType.h"
#include "SurfaceInfo.h"
#include "SurfaceLayer.h"
#include "QMsgBox.h"
#include "Density.h"
#include "QGLViewer/vec.h"
#include "Surface.h"
#include "QsLog.h"
#include "MolecularGridEvaluator.h"
#include "DensityEvaluator.h"
#include "GridData.h"
#include "Matrix.h"

#include "Function.h"

#include <QTime>
#include <QProgressDialog>
#include <cmath>
#include <set>
#include <QDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {

Orbitals::Orbitals(Data::Orbitals& orbitals)
 : Base(orbitals.label()), 
   m_orbitals(orbitals),
   m_configurator(*this), 
   m_molecularGridEvaluator(0),
   m_progressDialog(0)
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

   m_orbitals.shellList().resize();
 
   m_orbitals.boundingBox(m_bbMin, m_bbMax);
   QLOG_WARN() << "Attempt to append surface";
   //appendSurfaces(m_orbitals.surfaceList());
}



void Orbitals::appendSurfaces(Data::SurfaceList& surfaceList)
{
   Data::SurfaceList::const_iterator iter;
   for (iter = surfaceList.begin(); iter != surfaceList.end(); ++iter) {
       Layer::Surface* surfaceLayer(new Layer::Surface(*(*iter)));
       connect(surfaceLayer, SIGNAL(updated()), this, SIGNAL(softUpdate()));
       appendLayer(surfaceLayer);
   }
}



void Orbitals::setMolecule(Layer::Molecule* molecule)
{
   m_molecule = molecule;
   connect(this, SIGNAL(updated()), m_molecule, SIGNAL(updated()));
   connect(this, SIGNAL(softUpdate()), m_molecule, SIGNAL(softUpdate()));
}


unsigned Orbitals::nAlpha() const 
{ 
   return m_orbitals.nAlpha(); 
}


unsigned Orbitals::nBeta() const 
{ 
   return m_orbitals.nBeta(); 
}


unsigned Orbitals::nBasis() const 
{ 
   return m_orbitals.nBasis(); 
}


unsigned Orbitals::nOrbitals() const 
{ 
   return m_orbitals.nOrbitals(); 
}


void Orbitals::addToQueue(Data::SurfaceInfo const& info) 
{ 
   m_surfaceInfoQueue.append(info); 
}


void Orbitals::clearSurfaceQueue()
{ 
   m_surfaceInfoQueue.clear(); 
}


void Orbitals::showGridInfo()
{
   GridInfoDialog dialog(&m_availableGrids, m_molecule->text(), 
       m_molecule->coordinatesForCubeFile());
   dialog.exec();
}


void Orbitals::editBoundingBox()
{
   BoundingBoxDialog dialog(&m_bbMin, &m_bbMax);
   dialog.exec();
}


Data::GridData* Orbitals::findGrid(Data::SurfaceType const& type, 
   Data::GridSize const& size, Data::GridDataList const& gridList)
{
   Data::GridData* grid(0);
   Data::GridDataList::const_iterator iter;

   for (iter = gridList.begin(); iter != gridList.end(); ++iter) {
       if ( type == (*iter)->surfaceType() && size == (*iter)->size() ) {
          grid = (*iter);
          break;
       }
   }

   if (grid) {
      QLOG_TRACE() << "Reusing existing grid data" << type.toString();
   }else {
      QLOG_TRACE() << "Grid data not found" << type.toString();
   }

   return grid;
}



// The default description is appropriate for localized orbitals
QString Orbitals::description(Data::SurfaceInfo const& info, bool const tooltip)
{
   Data::SurfaceType const& type(info.type());
   QString label(type.toString());

   if (type.isOrbital()) {
      Data::SurfaceType::Kind kind(type.kind());

      unsigned nElectrons = kind == Data::SurfaceType::AlphaOrbital ? nAlpha() : nBeta();
      unsigned index(type.index());

      if (index <= nElectrons) {
         label += " (occ)";
      }
   }

   if (tooltip) label += "\nIsovalue = " + QString::number(info.isovalue(), 'f', 3);
 
   return label;
}



void Orbitals::processSurfaceQueue()
{
   // First, check to see if we are still computing data from a previous request.
   if (m_molecularGridEvaluator) {
      QMsgBox::warning(0, "IQmol", "Still processing previous grid data request");
      return;
   }

   // Second, determine what data the user has requested and check to see if we 
   // already have those data lying around.
   // TODO: If the user requests the same surface, but with different isovalues,
   //       I think, this will recompute the grid data multiple times.
   typedef QList<QPair<Data::SurfaceType, Data::GridSize> > GridQueue;
   GridQueue gridQueue;

   SurfaceInfoQueue::iterator iter;
   for (iter = m_surfaceInfoQueue.begin(); iter != m_surfaceInfoQueue.end(); ++iter) {
       Data::SurfaceType type((*iter).type());
       Data::GridSize size(m_bbMin, m_bbMax, (*iter).quality());
       Data::GridData* grid(findGrid(type, size, m_availableGrids));

       if (!grid) {
          // If the user requests an alpha, beta, spin or total density, we compute
          // the alpha and beta densities and combine them later for efficiency.
          if (type.isRegularDensity()) {
             type.setKind(Data::SurfaceType::AlphaDensity);
             gridQueue.append(qMakePair(type, size));
             type.setKind(Data::SurfaceType::BetaDensity);
             gridQueue.append(qMakePair(type, size));

             type.setKind(Data::SurfaceType::TotalDensity);
             gridQueue.append(qMakePair(type, size));
             type.setKind(Data::SurfaceType::SpinDensity);
             gridQueue.append(qMakePair(type, size));

          }else {
             gridQueue.append(qMakePair(type, size));
          }
       }
   }

   // Third, allocate the grids
   Data::GridDataList grids;
   GridQueue::const_iterator grid; 
   for (grid = gridQueue.begin(); grid != gridQueue.end(); ++grid) {
       grids.append(new Data::GridData(grid->second,grid->first));
   }

   // Fouth set up the (threaded) evaluator to do all the hard work.

   Data::ShellList& shellList(m_orbitals.shellList());

   m_molecularGridEvaluator = new MolecularGridEvaluator(grids, shellList, 
      m_orbitals.alphaCoefficients(),
      m_orbitals.betaCoefficients(),
      m_availableDensities);

   m_progressDialog = new QProgressDialog();
   m_progressDialog->setWindowModality(Qt::NonModal);
   m_progressDialog->show();

   connect(m_progressDialog, SIGNAL(canceled()), 
      this, SLOT(gridEvaluatorCanceled()));

   connect(m_molecularGridEvaluator, SIGNAL(progressLabelText(QString const&)), 
      m_progressDialog, SLOT(setLabelText(QString const&)));
   connect(m_molecularGridEvaluator, SIGNAL(progressMaximum(int)), 
      m_progressDialog, SLOT(setMaximum(int)));
   connect(m_molecularGridEvaluator, SIGNAL(progressValue(int)), 
      m_progressDialog, SLOT(setValue(int)));
   connect(m_molecularGridEvaluator, SIGNAL(finished()), 
      this, SLOT(gridEvaluatorFinished()));

   m_molecularGridEvaluator->start();
}


void Orbitals::gridEvaluatorCanceled()
{
   if (m_molecularGridEvaluator) {
      m_molecularGridEvaluator->stopWhatYouAreDoing();
      // deleting m_progressDialog  here causes a crash.
      m_progressDialog = 0;
      clearSurfaceQueue();
   }else {
      QLOG_WARN() << "MolecularGridEvaluator not found!";
   }
}


void Orbitals::gridEvaluatorFinished()
{
   qDebug() << "GridEvaluatorFinished() called";

   if (!m_molecularGridEvaluator) {
      QLOG_WARN() << "MolecularGridEvaluator not found!";
      return;
   }

   if (m_molecularGridEvaluator->status() == Task::Terminated) {
      qDebug() << "Evaluator terminated"; 
      Data::GridDataList grids(m_molecularGridEvaluator->getGrids());
      for (int i = 0; i < grids.size(); ++i) {
          delete grids[i];
      }
      delete m_molecularGridEvaluator;
      m_molecularGridEvaluator = 0;
   }else {
      // This should be deleted, but it triggers a crash if I do so
      if (m_progressDialog) m_progressDialog->hide();
      m_availableGrids += m_molecularGridEvaluator->getGrids();
      delete m_molecularGridEvaluator;
      m_molecularGridEvaluator = 0;
      calculateSurfaces(); 
   }
}



void Orbitals::calculateSurfaces()
{
   QProgressDialog* progressDialog = new QProgressDialog("Calculating surfaces", 
      "Cancel", 0, m_surfaceInfoQueue.count(), 0);

   progressDialog->setWindowModality(Qt::WindowModal);
   progressDialog->show();

   Qt::CheckState checked(Qt::Checked);
   int progress(0);
   SurfaceInfoQueue::iterator iter;
   for (iter = m_surfaceInfoQueue.begin(); iter != m_surfaceInfoQueue.end(); ++iter) {
       Data::Surface* surfaceData(generateSurface(*iter));

       if (surfaceData) {
          //m_orbitals.appendSurface(surfaceData);

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
       QApplication::processEvents();
       if (progressDialog->wasCanceled()) break;
   }

   // delete on progressDialog here causes a crash
    progressDialog->hide();

   clearSurfaceQueue();
   updated(); 
}



Data::Surface* Orbitals::generateSurface(Data::SurfaceInfo const& surfaceInfo)
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



void Orbitals::dumpGridInfo() const
{
   Data::GridDataList::const_iterator iter;
   for (iter = m_availableGrids.begin(); iter != m_availableGrids.end(); ++iter) {
       qDebug() << "Grid info:" << (*iter)->surfaceType().toString(); 
       (*iter)->size().dump(); 
   }
}

} } // end namespace IQmol::Layer
