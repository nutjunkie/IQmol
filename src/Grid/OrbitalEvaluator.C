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

#include "OrbitalEvaluator.h"
#include "GridEvaluator.h"
#include "ShellList.h"
#include "QsLog.h"
#include <QApplication>


using namespace qglviewer;

namespace IQmol {

OrbitalEvaluator::OrbitalEvaluator(Data::GridDataList& grids, Data::ShellList& shellList, 
   Matrix const& coefficients, QList<int> indices) : m_grids(grids), m_shellList(shellList),
   m_coefficients(coefficients), m_indices(indices)
{
   m_returnValues.resize(m_indices.size());
   m_function = boost::bind(&OrbitalEvaluator::evaluate, this, _1, _2, _3);

   double thresh(0.001);
   m_evaluator = new MultiGridEvaluator(m_grids, m_function, thresh);
   connect(m_evaluator, SIGNAL(progress(int)), this, SIGNAL(progress(int)));
   connect(m_evaluator, SIGNAL(finished()), this, SLOT(evaluatorFinished()));

   m_totalProgress = m_evaluator->totalProgress();
}



void OrbitalEvaluator::run()
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


void OrbitalEvaluator::evaluatorFinished()
{
   qDebug() << "OrbitalEvaluator finished";
//   m_evaluator->deleteLater();
//   m_evaluator = 0;
   finished();
}



Vector const& OrbitalEvaluator::evaluate(double const x, double const y, double const z)
{
   Vector const& s1(m_shellList.shellValues(Vec(x,y,z)));
   unsigned size(m_indices.size()); 

   for (unsigned i = 0; i < size; ++i) {
       MatrixRow row(m_coefficients, m_indices[i]);
       m_returnValues[i] = inner_prod(row, s1);
   }  
    
   return m_returnValues;
}

} // end namespace IQmol
