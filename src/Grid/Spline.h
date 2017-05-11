#ifndef IQMOL_GRID_SPLINE_H
#define IQMOL_GRID_SPLINE_H
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

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <exception>


namespace ublas = boost::numeric::ublas;

namespace IQmol {
namespace Math {
   /// Class for interpolating data using cubic splines.  Based on code from
   /// the following:
   ///    dynamo:- Event driven molecular dynamics simulator 
   ///    http://www.marcusbannerman.co.uk/dynamo
   ///    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>
   ///    (distributed under version 3 of the GPL)
   class Spline : private std::vector<std::pair<double, double> > {

      public:
         enum BoundaryConditions { FixedFirstDerivative, FixedSecondDerivative, ParabolicRunout };

         Spline() : m_valid(false), m_bcLow(FixedSecondDerivative), 
            m_bcHigh(FixedSecondDerivative), m_bcLowVal(0), m_bcHighVal(0) { }
	        
         typedef std::vector<std::pair<double, double> > base;
         typedef base::const_iterator const_iterator;

         const_iterator begin() const { return base::begin(); }
         const_iterator end() const { return base::end(); }

         void clear();
         size_t size() const { return base::size(); }
         size_t max_size() const { return base::max_size(); }
         size_t capacity() const { return base::capacity(); }
         bool empty() const { return base::empty(); }
         void addPoint(double x, double y);
         void setLowBC(BoundaryConditions bc, double val = 0);
         void setHighBC(BoundaryConditions bc, double val = 0);
         double operator()(double xval);


      private:
         struct SplineData { double x,a,b,c,d; };
         std::vector<SplineData> m_data;
         //Second derivative at each point
         ublas::vector<double> m_ddy;
         bool m_valid;
         BoundaryConditions m_bcLow, m_bcHigh; 
         double m_bcLowVal, m_bcHighVal;

         double splineCalc(std::vector<SplineData>::const_iterator i, double xval);
         double lowCalc(double xval);
         double highCalc(double xval);
         double x(size_t i) const { return operator[](i).first; }
         double y(size_t i) const { return operator[](i).second; }
         double h(size_t i) const { return x(i+1) - x(i); }

         template<class T>
         bool invertMatrix(ublas::matrix<T> A, ublas::matrix<T>& inverse);
         void generate();
   };

} }  // end namespace IQmol::Math

#endif
