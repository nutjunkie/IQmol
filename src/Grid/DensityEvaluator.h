#ifndef IQMOL_GRID_DENSITY_EVALUATOR_H
#define IQMOL_GRID_DENSITY_EVALUATOR_H
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

#include "GridData.h"
#include "Function.h"
#include "Matrix.h"
#include "Task.h"


namespace IQmol {

   class MultiGridEvaluator;

   namespace Data {
      class ShellList;
   }

   class DensityEvaluator : public Task {

      Q_OBJECT

      public:
         // Note the list of densities can be a superset of the requred densities
         // for the grid calcualation.
         DensityEvaluator(Data::GridDataList& grids, Data::ShellList& shellList, 
            QList<Vector*> const& densities);

      Q_SIGNALS:
         void progress(int);

      protected:
         void run();

      private Q_SLOTS:
         void evaluatorFinished();

      private:
		 // Fills the m_returnValues vector with the value of each requested
		 // orbital at the given point.
         Vector const& evaluate(double const x, double const y, double const z);
         
         MultiFunction3D     m_function;
         Data::GridDataList  m_grids;
         Data::ShellList&    m_shellList;
         QList<Vector*>      m_densities;
         Vector              m_returnValues;
         MultiGridEvaluator* m_evaluator;
   };

} // end namespace IQmol

#endif
