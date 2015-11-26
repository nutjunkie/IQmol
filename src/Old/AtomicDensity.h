#ifndef IQMOL_ATOMICDENSITY_H
#define IQMOL_ATOMICDENSITY_H
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
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "Mesh.h"


namespace IQmol {
   namespace AtomicDensity {

      /// Base class that represent atomic densities within a molecule.
      /// Note that in many cases the densities are limited to atoms up to
      /// argon; this can be checked with the isAvailable member function;
      class Base{
   
         public:
            Base(unsigned atomicNumber);
            virtual ~Base() { }

            void setCharge(double const charge) { m_charge = charge; }
   
            virtual bool isAvailable() const = 0;
   
            virtual double density(qglviewer::Vec const& position) const = 0;
   
            unsigned atomicNumber() const { return m_atomicNumber; }
   
            virtual double computeSignificantRadius(double const thresh) const;
          
         protected:
            unsigned m_atomicNumber;
            double   m_charge;
   
         private:
            double binarySearch(double const r1, double const r2, double const y1, 
               double const y2, double const thresh) const;
      };


      class AtomShellApproximation : public Base {
   
         public:
            AtomShellApproximation(unsigned Z);
   
            bool isAvailable() const { return (m_atomicNumber <= s_maxAtomicNumber); }
   
            double density(qglviewer::Vec const&) const;
   
         private:
            static const unsigned s_maxAtomicNumber;
            static const unsigned s_expansionLength[];
            static const double* s_parameters[];
   
            static double const s_atom01[];  static double const s_atom02[];
            static double const s_atom03[];  static double const s_atom04[];
            static double const s_atom05[];  static double const s_atom06[];
            static double const s_atom07[];  static double const s_atom08[];
            static double const s_atom09[];  static double const s_atom10[];
            static double const s_atom11[];  static double const s_atom12[];
            static double const s_atom13[];  static double const s_atom14[];
            static double const s_atom15[];  static double const s_atom16[];
            static double const s_atom17[];  static double const s_atom18[];
            static double const s_atom19[];  static double const s_atom20[];
            static double const s_atom21[];  static double const s_atom22[];
            static double const s_atom23[];  static double const s_atom24[];
            static double const s_atom25[];  static double const s_atom26[];
            static double const s_atom27[];  static double const s_atom28[];
            static double const s_atom29[];  static double const s_atom30[];
            static double const s_atom31[];  static double const s_atom32[];
            static double const s_atom33[];  static double const s_atom34[];
            static double const s_atom35[];  static double const s_atom36[];
   
            unsigned m_nBasis;
            const double* m_data;
      };
   
   
      class SuperpositionIonicDensities : public Base {
   
         public:
            SuperpositionIonicDensities(unsigned Z);

            bool isAvailable() const { return (m_atomicNumber <= s_maxAtomicNumber); }

            double density(qglviewer::Vec const&) const;
   
         private:
            static const unsigned s_maxAtomicNumber;
            static const unsigned s_expansionLength[];
            static const double*  s_parameters[];
            static const double   s_stepSize;
   
            static double const s_atom01a[], s_atom01n[], s_atom01c[];
            static double const s_atom02a[], s_atom02n[], s_atom02c[];
            static double const s_atom03a[], s_atom03n[], s_atom03c[];
            static double const s_atom04a[], s_atom04n[], s_atom04c[];
            static double const s_atom05a[], s_atom05n[], s_atom05c[];
            static double const s_atom06a[], s_atom06n[], s_atom06c[];
            static double const s_atom07a[], s_atom07n[], s_atom07c[];
            static double const s_atom08a[], s_atom08n[], s_atom08c[];
            static double const s_atom09a[], s_atom09n[], s_atom09c[];
            static double const s_atom10a[], s_atom10n[], s_atom10c[];
            static double const s_atom11a[], s_atom11n[], s_atom11c[];
            static double const s_atom12a[], s_atom12n[], s_atom12c[];
            static double const s_atom13a[], s_atom13n[], s_atom13c[];
            static double const s_atom14a[], s_atom14n[], s_atom14c[];
            static double const s_atom15a[], s_atom15n[], s_atom15c[];
            static double const s_atom16a[], s_atom16n[], s_atom16c[];
            static double const s_atom17a[], s_atom17n[], s_atom17c[];
            static double const s_atom18a[], s_atom18n[], s_atom18c[];
   
            const double* m_neutralData;
            const double* m_chargedData;
            unsigned m_nNeutralData;
            unsigned m_nChargedData;
      };
   
   
      class VanDerWaals : public Base {

         public:
            VanDerWaals(unsigned Z, qglviewer::Vec const& center = qglviewer::Vec(0.0,0.0,0.0),
            double const scale = 1.0, double const solventRadius = 0.0);

            bool isAvailable() const { return true; }

            double density(qglviewer::Vec const&) const;

            double computeSignificantRadius(double const) const;
   
            Data::Mesh generateMesh(int nDivisions, QList<VanDerWaals*> const& atoms);
            
         private:
            static double   s_vertices[12][3];
            static unsigned s_indices[20][3];
   
            void generateTriangle(Data::Mesh& mesh, 
               Data::OMMesh::VertexHandle const& Av, Data::OMMesh::VertexHandle const& Bv, 
               Data::OMMesh::VertexHandle const& Cv, int div);
               
            double m_radius;
            int    m_filterLevel;
            qglviewer::Vec m_center;
      };
   
   }  // end namespace AtomicDensity
} // end namespace IQmol

#endif
