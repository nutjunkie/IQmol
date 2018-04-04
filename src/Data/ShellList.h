#ifndef IQMOL_DATA_SHELLLIST_H
#define IQMOL_DATA_SHELLLIST_H
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
#include "DataList.h"
#include "Matrix.h"
#include "Shell.h"


namespace IQmol {
namespace Data {

    class Geometry;

   // This is a POD structure to accumulate the data in the parsers (taken from
   // FormattedCheckpoingParser).
   struct ShellData {
      QList<int>      shellTypes;
      QList<unsigned> shellToAtom;
      QList<unsigned> shellPrimitives;
      QList<double>   exponents;
      QList<double>   contractionCoefficients;
      QList<double>   contractionCoefficientsSP;
      QList<double>   overlapMatrix;
   };

   class ShellList : public List<Shell> {

      friend class boost::serialization::access;

      public:
         ShellList() { }

         ShellList(ShellData const& shellData, Geometry const& geometry);

         /// Returns the (-1,-1,-1) and (1,1,1) octant corners of a rectangular
         /// box that encloses the significant region of the Shells where 
         /// significance is determined by thresh.  
         void boundingBox(qglviewer::Vec& min, qglviewer::Vec& max, 
            double const thresh = 0.001);

         unsigned nBasis() const;

         Vector const& overlapMatrix() const { return m_overlapMatrix; }

         void setOverlapMatrix(QList<double> const& overlapMatrix) {
            unsigned nElements(overlapMatrix.size());
            m_overlapMatrix.resize(nElements);
            for (unsigned i = 0; i < nElements; ++i) {
                m_overlapMatrix[i] = overlapMatrix[i];
            }
         }

         /// Allocates the memory for evaluating the shells/shell pairs on a grid
         /// point.  This should be called after the last Shell has been appended
         /// to the list and before shellValues or shellPairValues is called.
         void resize();

         Vector const& shellValues(qglviewer::Vec const& gridPoint);
         // Returns the vectorized upper triangular array of unique shell 
         // values at the grid point pairs.
         Vector const& shellPairValues(qglviewer::Vec const& gridPoint);

         // Shell offset for each atom
         QList<unsigned> shellAtomOffsets() const;

         // Basis offset for each atom
         QList<unsigned> basisAtomOffsets() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            serializeList(ar, version);
         }  
         
         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            serializeList(ar, version);
         }  

         void dump() const;

      private:
         unsigned m_nBasis;
         Vector   m_shellValues;
         Vector   m_shellPairValues;
         Vector   m_overlapMatrix;   // upper triangular
   };

} } // end namespace IQmol::Data

#endif
