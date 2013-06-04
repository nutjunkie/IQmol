#ifndef IQMOL_ALIGN_H
#define IQMOL_ALIGN_H
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

#include "qglviewer/Vec.h"
#include "qglviewer/Quaternion.h"


namespace IQmol {

   /// Wrapper class to the QCPRotation package
   class Align {

      public:
         Align(QList<qglviewer::Vec> const& reference, QList<qglviewer::Vec> const& current,
            QList<double> const& weights = QList<double>());
         ~Align();

         bool computeAlignment();
         qglviewer::Quaternion& rotation();
         qglviewer::Vec& translation();

      private:
         double** initMatrix(int const rows, int const cols);
         void destroyMatrix(double*** matrix);
         void print3x3Matrix(double* matrix);
         void printCoords(double **coords);
         qglviewer::Vec centerCoords(double **coords);
 
         int      m_nAtoms;
         double*  m_weights;
         double** m_reference;
         double** m_current;
         bool     m_aligned;

         qglviewer::Vec m_translation;
         qglviewer::Quaternion m_rotation;
   };


} // end namespace IQmol

#endif
