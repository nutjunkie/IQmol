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

#include "MolecularGridEvaluator.h"
#include "DensityEvaluator.h"
#include "OrbitalEvaluator.h"
#include "ShellList.h"
#include "Density.h"
#include "QsLog.h"
#include <set>
#include <QApplication>


using namespace qglviewer;

namespace IQmol {


MolecularGridEvaluator::MolecularGridEvaluator(
   Data::GridDataList& grids, 
   Data::ShellList& shellList, 
   Matrix const& alphaCoefficients, 
   Matrix const& betaCoefficients, 
   QList<Data::Density*> const& densities) :
   m_grids(grids),
   m_shellList(shellList),
   m_alphaCoefficients(alphaCoefficients),
   m_betaCoefficients(betaCoefficients),
   m_densities(densities)
{
}


void MolecularGridEvaluator::run()
{
   Data::GridDataList::iterator iter;
   for (iter = m_grids.begin(); iter != m_grids.end(); ++iter) {
       (*iter)->surfaceType().dump();
   }

   // First obtain a list of the unique grid sizes.  This avoids calculating
   // shell(pair) data on the same grid points more than once.
   std::set<Data::GridSize> sizes;
   for (iter = m_grids.begin(); iter != m_grids.end(); ++iter) {
       sizes.insert((*iter)->size());    
   }

   // Second, determine what data are required for each grid size.  We do 
   // this loop so that we don't have to recompute shell and/or shell-pair
   // values over the same grid points.
   QLOG_TRACE() << "Computing data for" << m_grids.size() << "grids";
   QLOG_TRACE() << "There are" << sizes.size() << "different grid sizes";
   std::set<Data::GridSize>::iterator size;

   unsigned sizeCount(1);

   for (size = sizes.begin(); size != sizes.end(); ++size, ++sizeCount) {
(*size).dump();
       Data::GridDataList densityGrids;
       Data::GridDataList alphaGrids;
       Data::GridDataList betaGrids;

       QList<Vector*> densityVectors;
       QList<int> alphaOrbitals;
       QList<int> betaOrbitals;

       for (iter = m_grids.begin(); iter != m_grids.end(); ++iter) {
           if ((*iter)->size() == *size) {

              Data::SurfaceType type((*iter)->surfaceType());
              bool found(false);

              if (type.isDensity()) {
                 // Find the corresponding density matrix
                 for (int i = 0; i < m_densities.size(); ++i) {
                     if (m_densities[i]->surfaceType() == (*iter)->surfaceType()) {
                        densityGrids.append(*iter);
                        densityVectors.append(m_densities[i]->vector());
                        found = true;
                        break;
                     }
                 }

              }else if (type.kind() == Data::SurfaceType::AlphaOrbital) {
                 found = true;
                 alphaGrids.append(*iter);
                 alphaOrbitals.append(type.index()-1);
                 
              }else if (type.kind() == Data::SurfaceType::BetaOrbital) {
                 found = true;
                 betaGrids.append(*iter);
                 betaOrbitals.append(type.index()-1);

              }else if (type.kind() == Data::SurfaceType::Custom) {
                 for (int i = 0; i < m_densities.size(); ++i) {
                     if (type.label() == m_densities[i]->label()) {
qDebug() << "Pairing successful" << type.label() << m_densities[i]->label(); 
                        densityGrids.append(*iter);
                        densityVectors.append(m_densities[i]->vector());
                        found  = true;
                        break;
                     }
                 }

              }else  {
                 QLOG_WARN() << "Unknown Grid type found in processQueue";
              }

              if (!found) {
                 QLOG_WARN() << "Failed to pair data with grid";
                 type.dump();
              }
           }
       }

       if (!alphaOrbitals.isEmpty() && !m_terminate) {
          QString s("Computing alpha orbitals on grid ");
          s += QString::number(sizeCount);
          progressLabelText(s);

          QLOG_TRACE() << "MGE: Computing" << alphaOrbitals.size() << "alpha orbital grids";
          OrbitalEvaluator evaluator(alphaGrids, m_shellList, m_alphaCoefficients, 
             alphaOrbitals);

          progressMaximum(evaluator.totalProgress());
          progressValue(0);
          connect(&evaluator, SIGNAL(progress(int)), this, SIGNAL(progressValue(int)));  

          evaluator.start();
          while (evaluator.isRunning()) {
             msleep(100);
             QApplication::processEvents();
             if (m_terminate) {
                evaluator.stopWhatYouAreDoing();
                evaluator.wait();
             }
          }
          
          QLOG_TRACE() << "Time taken to compute orbital grids:" << evaluator.timeTaken();
       }

       if (!betaOrbitals.isEmpty() && !m_terminate) {
          QString s("Computing beta orbitals on grid ");
          s += QString::number(sizeCount);
          progressLabelText(s);

          OrbitalEvaluator evaluator(betaGrids, m_shellList, m_betaCoefficients, 
             betaOrbitals);

          progressMaximum(evaluator.totalProgress());
          progressValue(0);
          connect(&evaluator, SIGNAL(progress(int)), this, SIGNAL(progressValue(int)));  

          evaluator.start();
          while (evaluator.isRunning()) {
             msleep(100);
             QApplication::processEvents();
             if (m_terminate) {
                evaluator.stopWhatYouAreDoing();
                evaluator.wait();
             }
          }
 
          QLOG_TRACE() << "Time taken to compute orbital grids:" << evaluator.timeTaken();
       }

       if (!densityGrids.isEmpty() && !m_terminate) {
          QString s("Computing density on grid ");
          s += QString::number(sizeCount);
          progressLabelText(s);

          QLOG_TRACE() << "MGE: Computing" << densityVectors.size() << "density grids";
          DensityEvaluator evaluator(densityGrids, m_shellList, densityVectors);

          progressMaximum(evaluator.totalProgress());
          progressValue(0);
          connect(&evaluator, SIGNAL(progress(int)), this, SIGNAL(progressValue(int)));  

          evaluator.start();
          while (evaluator.isRunning()) {
             msleep(100);
             QApplication::processEvents();
             if (m_terminate) {
                evaluator.stopWhatYouAreDoing();
                evaluator.wait();
             }
          }
 
          QLOG_TRACE() << "Time taken to compute density grids:" << evaluator.timeTaken();
       }
   }
}

} // end namespace IQmol
