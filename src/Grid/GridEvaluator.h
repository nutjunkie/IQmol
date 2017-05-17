#ifndef IQMOL_GRID_GRIDEVALUATOR_H
#define IQMOL_GRID_GRIDEVALUATOR_H
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

#include "Task.h"
#include "Function.h"


namespace IQmol {

   namespace Data {
      class GridData;
   }

   class GridEvaluator : public Task {

      Q_OBJECT

      public:
         GridEvaluator(Data::GridData& grid, Function3D const& function);

      protected:
         void run();

      private:
         Data::GridData&   m_grid;
         Function3D const& m_function;
   };


   /// GridEvaluator for cases where it is more efficient to generate multiple
   /// grid data at a time.  For example, several molecular orbitals requiring
   /// only one evaluation of the shell data at each point
   class MultiGridEvaluator : public Task {

      Q_OBJECT

      public:
         // Note we don't check for size consistency between the number of 
         // grids and the return on the MultiFunction3D object.
         MultiGridEvaluator(QList<Data::GridData*> grids, MultiFunction3D const& function,
            double const thresh, bool const coarseGrain = true);

      protected:
         void run();

      private:
         void runCoarseGrain();

         QList<Data::GridData*> m_grids;
         MultiFunction3D const& m_function;
         double m_thresh;
         bool m_coarseGrain;
   };

} // end namespace IQmol

#endif
