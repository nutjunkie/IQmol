#ifndef IQMOL_GRID_GRIDPRODUCT_H
#define IQMOL_GRID_GRIDPRODUCT_H
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

   class GridProduct: public Task {

      Q_OBJECT

      public:
         GridProduct(Vector& values, QList<Data::GridData const*>&  grids, 
            double const binSize = 0.1);

      Q_SIGNALS:
         void progressValue(int);

      protected:
         void run();

      private:
         Vector& m_values;
         QList<Data::GridData const*> m_grids;
         double  m_binSize;
   };

} // end namespace IQmol

#endif
