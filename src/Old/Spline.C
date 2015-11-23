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

#include "Spline.h"


namespace IQmol {
namespace Math {

void Spline::clear() 
{ 
   m_valid = false; 
   base::clear(); 
   m_data.clear(); 
}


void Spline::addPoint(double x, double y) 
{
   m_valid = false;
   base::push_back(std::pair<double, double>(x,y)); 
}


double Spline::operator()(double xval) 
{
   if (!m_valid) generate();
   //Special cases when we're outside the range of the spline points
   if (xval <= x(0)) return lowCalc(xval);
   if (xval >= x(size()-1)) return highCalc(xval);

   //Check all intervals except the last one
   std::vector<SplineData>::const_iterator iPtr;
   for (iPtr = m_data.begin(); iPtr != m_data.end()-1; ++iPtr) {
	   if ((xval >= iPtr->x) && (xval <= (iPtr+1)->x)) {
          return splineCalc(iPtr, xval);
       }
   }
   return splineCalc(m_data.end() - 1, xval);
}


void Spline::setLowBC(BoundaryConditions bc, double val)
{ 
   m_bcLow = bc; 
   m_bcLowVal = val; 
   m_valid = false; 
}


void Spline::setHighBC(BoundaryConditions bc, double val)
{ 
   m_bcHigh = bc; 
   m_bcHighVal = val; 
   m_valid = false; 
}
 

//Function to calculate the value of a given spline at a point xval
double Spline::splineCalc(std::vector<SplineData>::const_iterator i, double xval)
{ 
   const double lx = xval - i->x;
   return ((i->a * lx + i->b) * lx + i->c) * lx + i->d;
}


double Spline::lowCalc(double xval)
{
   double lx = xval - x(0);
   double firstDeriv = (y(1)-y(0)) / h(0) - 2.0*h(0)*(m_data[0].b + 2.0*m_data[1].b) / 6.0;

   switch(m_bcLow) {
      case FixedFirstDerivative:
         return lx * m_bcLowVal + y(0);
      case FixedSecondDerivative:
         return lx * lx * m_bcLowVal + firstDeriv * lx + y(0);
      case ParabolicRunout:
        return lx * lx * m_ddy[0] + lx * firstDeriv  + y(0);
   }	
}


double Spline::highCalc(double xval)
{
   double lx = xval - x(size() - 1);
   double firstDeriv = 2.0 * h(size()-2) * (m_ddy[size()-2] + 2.0 * m_ddy[size() - 1]) / 6.0 
      + (y(size()-1) - y(size()-2)) / h(size()-2);
   switch(m_bcHigh) {
      case FixedFirstDerivative:
         return lx * m_bcHighVal + y(size() - 1);
      case FixedSecondDerivative:
         return lx * lx * m_bcHighVal + firstDeriv * lx + y(size() - 1);
      case ParabolicRunout:
         return lx * lx * m_ddy[size()-1] + lx * firstDeriv  + y(size() - 1);
   }
}


//Invert a arbitrary matrix using the boost ublas library
template<class T>
bool Spline::invertMatrix(ublas::matrix<T> A, ublas::matrix<T>& inverse) 
{
   using namespace ublas;
   // create a permutation matrix for the LU-factorization
 	permutation_matrix<std::size_t> pm(A.size1());
	
 	// perform LU-factorization
 	int res = lu_factorize(A,pm);
        if( res != 0 ) return false;
	
 	// create identity matrix of "inverse"
 	inverse.assign(ublas::identity_matrix<T>(A.size1()));
	
 	// backsubstitute to get the inverse
 	lu_substitute(A, pm, inverse);
	
 	return true;
      }


//This function will recalculate the spline parameters and store
//them in m_data, ready for spline interpolation
void Spline::generate()
{
   if (size() < 2) throw std::runtime_error("Spline requires at least 2 points");
	
   //If any spline points are at the same x location, we have to
   //just slightly seperate them
   { 
      bool testPassed(false);
	  while (!testPassed) {
         testPassed = true;
         std::sort(base::begin(), base::end());
	      
         for (base::iterator iPtr = base::begin(); iPtr != base::end() - 1; ++iPtr)
         if (iPtr->first == (iPtr+1)->first) {
		    if ((iPtr+1)->first != 0) {
               (iPtr+1)->first += (iPtr+1)->first * std::numeric_limits<double>::epsilon() * 10;
		    }else {
               (iPtr+1)->first = std::numeric_limits<double>::epsilon() * 10;
            }
            testPassed = false;
            break;
         }
      }
   }	    

   const size_t e = size() - 1;

   ublas::matrix<double> A(size(), size());
   for (size_t yv(0); yv <= e; ++yv) {
       for (size_t xv(0); xv <= e; ++xv) {
	       A(xv,yv) = 0;
       }
   }

   for (size_t i(1); i < e; ++i) {
       A(i-1,i) = h(i-1);
       A(i,i) = 2 * (h(i-1) + h(i));
       A(i+1,i) = h(i);
   }

   ublas::vector<double> C(size());
   for (size_t xv(0); xv <= e; ++xv) {
       C(xv) = 0;
   }

   for (size_t i(1); i < e; ++i) {
	  C(i) = 6.0 * ((y(i+1) - y(i)) / h(i) - (y(i) - y(i-1)) / h(i-1));
   }

   switch(m_bcLow) {
      case FixedFirstDerivative:
         C(0) = 6 * ((y(1) - y(0)) / h(0) - m_bcLowVal);
         A(0,0) = 2 * h(0);
         A(1,0) = h(0);
         break;
      case FixedSecondDerivative:
         C(0) = m_bcLowVal;
         A(0,0) = 1;
        break;
     case ParabolicRunout:
        C(0) = 0; A(0,0) = 1; A(1,0) = -1;
        break;
   }

   switch(m_bcHigh) {
      case FixedFirstDerivative:
         C(e) = 6 * (m_bcHighVal - (y(e) - y(e-1)) / h(e-1));
         A(e,e) = 2 * h(e - 1);
         A(e-1,e) = h(e - 1);
         break;
      case FixedSecondDerivative:
         C(e) = m_bcHighVal;
         A(e,e) = 1;
         break;
      case ParabolicRunout:
         C(e) = 0; A(e,e) = 1; A(e-1,e) = -1;	    
        break;
   }

   ublas::matrix<double> AInv(size(), size());
   invertMatrix(A,AInv);
	
   m_ddy = ublas::prod(C, AInv);

   m_data.resize(size()-1);
   for (size_t i(0); i < e; ++i) {
       m_data[i].x = x(i);
       m_data[i].a = (m_ddy(i+1) - m_ddy(i)) / (6 * h(i));
       m_data[i].b = m_ddy(i) / 2;
       m_data[i].c = (y(i+1) - y(i)) / h(i) - m_ddy(i+1) * h(i) / 6 - m_ddy(i) * h(i) / 3;
       m_data[i].d = y(i);
   }
   m_valid = true;
}



} } // end namespace IQmol
