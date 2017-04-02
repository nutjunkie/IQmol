#ifndef IQMOL_DATA_SHELL_H
#define IQMOL_DATA_SHELL_H
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

#include "QGLViewer/vec.h"
#include "Data.h"


namespace IQmol {
namespace Data {

   /// Basic Shell class representing a contracted shell of a particular
   /// angular momentum.  Note that we assume all the input values are in
   /// angstrom, not bohr, and that the coefficients are given for the
   /// normalized basis functions.
   class Shell : public Base {

      friend class boost::serialization::access;

      public:
         enum AngularMomentum { S, P, D5, D6, F7, F10, G9, G15 };

         Shell(AngularMomentum const angularMomentum = S, 
            qglviewer::Vec const& position = qglviewer::Vec(), 
            QList<double> const& exponents = QList<double>(), 
            QList<double> const& contractionCoefficients = QList<double>());

         Type::ID typeID() const { return Type::Shell; }

		 /// Returns the (-1,-1,-1) and (1,1,1) octant corners of a rectangular
		 /// box that encloses the significant region of the Shell where 
		 /// significance is determined by s_thresh.  
         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max);

		 // Returns a pointer to an array containing the values of the basis
		 // functions at the given position.
         double* evaluate(qglviewer::Vec const& position) const;
         AngularMomentum angularMomentum() const { return m_angularMomentum; }
         unsigned nBasis() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }  
         
         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version); 
         }  

         void dump() const;

         static double thresh() { return s_thresh; }

      private:
		 /// Value used for Shell cutoffs.  Note that for surfaces this value
		 /// is rather large to ensure the bounding box as small as possible. 
         /// The value should be consistent with the minimum isosurface value
         /// in the SurfaceConfigurator class.  Note that a large value may 
         /// not be appropriate for other wavefunction analyses.
		 static double s_thresh; 
		 /// Shell values are stored in this static array, the length of which
		 /// is sufficient for up to g angular momentum.
         /// This could cause problems if Shells are ever used in parallel
         static double s_values[15];
         static double s_zeroValues[15];

         double computeSignificantRadius(double const thresh);
         QString toString(AngularMomentum const) const;
         void normalize();

         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const) {
            ar & m_angularMomentum;
            ar & m_position;
            ar & m_exponents;
            ar & m_contractionCoefficients;
            ar & m_significantRadiusSquared;
         }

         AngularMomentum m_angularMomentum;
         qglviewer::Vec  m_position;
         QList<double>   m_exponents;
         QList<double>   m_contractionCoefficients;
         double          m_significantRadiusSquared;
   };


} } // end namespace IQmol::Data

#endif
