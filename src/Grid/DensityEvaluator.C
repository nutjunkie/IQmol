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

#include "DensityEvaluator.h"
#include "GridEvaluator.h"
#include "ShellList.h"
#include "QsLog.h"
#include <QDebug>
#include <QApplication>

using namespace qglviewer;

namespace IQmol {


DensityEvaluator::DensityEvaluator(Data::GridDataList& grids, Data::ShellList& shellList, 
   QList<Vector*> const& densities) : m_grids(grids), m_shellList(shellList),
   m_densities(densities), m_evaluator(0)
{
   if (grids.isEmpty()) return;
   m_returnValues.resize(m_densities.size());
   m_function = boost::bind(&DensityEvaluator::evaluate, this, _1, _2, _3);

   double thresh(0.001);
   m_evaluator = new MultiGridEvaluator(m_grids, m_function, thresh);

   connect(m_evaluator, SIGNAL(progress(int)), this, SIGNAL(progress(int)));
   connect(m_evaluator, SIGNAL(finished()), this, SLOT(evaluatorFinished()));

   m_totalProgress = m_evaluator->totalProgress();
}



void DensityEvaluator::run()
{
   m_evaluator->start();
   while (m_evaluator->isRunning()) {
      msleep(100);
      QApplication::processEvents();
      if (m_terminate) {
         m_evaluator->stopWhatYouAreDoing();
         m_evaluator->wait();
      }
   }
}



void DensityEvaluator::evaluatorFinished()
{
   qDebug() << "DensityEvaluator finished";
//   m_evaluator->deleteLater();
//   m_evaluator = 0;
   finished();
}



Vector const& DensityEvaluator::evaluate(double const x, double const y, double const z)
{
   Vector const& s2(m_shellList.shellPairValues(Vec(x,y,z)));
   for (int i = 0; i < m_densities.size(); ++i) {
       m_returnValues[i] = inner_prod(*(m_densities[i]), s2);
   }  
    
   return m_returnValues;
}

} // end namespace IQmol
