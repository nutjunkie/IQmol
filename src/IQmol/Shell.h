#ifndef IQMOL_SHELL_H
#define IQMOL_SHELL_H
/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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

#include "QGLViewer/vec.h"
#include <QList>
#include <vector>


namespace IQmol {

   /// Basic implementation of a shell of \em contracted gaussian basis functions. 
   class Shell {

      public:
         enum AngMom { S, P, D5, D6, F7, F10 };

         Shell(AngMom angMom, qglviewer::Vec const& position, QList<double> const& exponents, 
            QList<double> const& contractionCoefficients);
         ~Shell() { };

         /// The number of functions in the shell, e.g. 3 for a p shell
         unsigned int size() const { return m_size; }  

         /// Returns a raw array of shell values at the gridPoint
         double* evaluate(qglviewer::Vec const& gridPoint);

		 /// Returns the (-1,-1,-1) and (1,1,1) octant corners of a rectangular
		 /// box that encloses the significant region of the Shell where 
         /// significance is determined by s_thresh.  
         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max);
         static double thresh() { return s_thresh; }

         static std::vector<double> evaluateShellPairs(QList<Shell*> const& shells,
            int const nBasis, qglviewer::Vec const& gridPoint);

         // setThresh is probably unncessary and dump is just for debug
         static void setThresh(double const thresh) { s_thresh = thresh; }
         void dump() const;

      private:
		 /// Value used for Shell cutoffs.  Note that for surfaces this value
		 /// is rather large to ensure the bounding box as small as possible. 
         /// The value should be consistent with the minimum isosurface value
         /// in the SurfaceConfigurator class.  Note that a large value may 
         /// not be appropriate for other wavefunction analyses.
		 static double s_thresh; 

		 /// Static storage for returning the shell values at a grid point.
		 /// This avoids reallocating the space all the time.  The space is
		 /// sufficient for up to 10 cartesian f functions.
         static double s_values[];

         double computeSignificantRadius(double const thresh);
         void normalize();

         // Data
         AngMom m_angMom;
         qglviewer::Vec m_position;
         QList<double>  m_exponents;
         QList<double>  m_contractionCoefficients;
         double m_significantRadius2;
         unsigned int m_size;

         /// Angular momentum of the shell
         int m_L; 
   };

   //typedef QList<Shell*> ShellList;

} // end namespace IQmol


#endif
