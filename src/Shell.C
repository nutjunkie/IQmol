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

#include "Shell.h"
#include <algorithm>
#include <cmath>

#include <QtDebug>

using namespace qglviewer;

namespace IQmol {


double Shell::s_values[10];
double Shell::s_thresh = 0.001;

Shell::Shell(AngMom angMom, Vec const& position, QList<double> const& exponents, 
   QList<double> const& contractionCoefficients) : m_angMom(angMom), m_position(position), 
   m_exponents(exponents), m_contractionCoefficients(contractionCoefficients)
{
   if (exponents.size() != contractionCoefficients.size()) {
      qDebug() << "Inconsistent array lengths in Shell constructor";
   }else {

      switch (m_angMom) {
         case S:    m_size = 1;   m_L = 0;  break;
         case P:    m_size = 3;   m_L = 1;  break;
         case D5:   m_size = 5;   m_L = 2;  break;
         case D6:   m_size = 6;   m_L = 2;  break;
         case F7:   m_size = 7;   m_L = 3;  break;
         case F10:  m_size = 10;  m_L = 3;  break;
      }

      normalize();
      m_significantRadius2  = computeSignificantRadius(s_thresh); 
      m_significantRadius2 *= m_significantRadius2;
   }
}


void Shell::normalize()
{
   static double Ns  = std::pow(2.0/M_PI, 0.75);
   double pf(0.0), ex(0.0);

   switch (m_angMom) {
      case S:    pf =     Ns;  ex = 0.75;  break;
      case P:    pf = 2.0*Ns;  ex = 1.25;  break;
      case D5:   pf = 4.0*Ns;  ex = 1.75;  break;
      case D6:   pf = 4.0*Ns;  ex = 1.75;  break;
      case F7:   pf = 8.0*Ns;  ex = 2.25;  break;
      case F10:  pf = 8.0*Ns;  ex = 2.25;  break;
   }

   for (int i = 0; i < m_exponents.size(); ++i) {
       m_contractionCoefficients[i] *= pf * std::pow(m_exponents[i], ex);
   }
}


// Attempts to give an approximate size of the basis function for use in cutoffs
double Shell::computeSignificantRadius(double const thresh) 
{
   double radius(0.0);
   double alpha, coeff;
   double step(0.5), r, f;

   for (int i = 0; i < m_exponents.size(); ++i) {
       alpha = m_exponents[i];
       coeff = std::abs(m_contractionCoefficients[i]);
	   // This is the turning point of the primitive, things should all be
	   // downhill from here.
       r = std::sqrt(0.5*m_L/alpha);
       f = coeff * std::pow(r,m_L) * std::exp(-alpha*r*r);

	   // This is a little crusty, but to do things analytically 
	   // requires product logs
       while ((f > thresh) && (r < 100.0)) {
          r += step;
          f = coeff * std::pow(r,m_L) * std::exp(-alpha*r*r);
       }
       radius = std::max(radius, r);
   }

   return radius;
}


void Shell::boundingBox(Vec& min, Vec& max) 
{
   double r(std::sqrt(m_significantRadius2));
   Vec d(r, r, r);
   min = m_position - d;
   max = m_position + d;
}


// returns a null pointer if grid point is outside the significant radius
double* Shell::evaluate(Vec const& gridPoint)
{
   static const double rt3  = 1.0 / std::sqrt(3.0);
   static const double rt8  = 1.0 / std::sqrt(8.0);
   static const double rt15 = 1.0 / std::sqrt(15.0);

   // bail early if the basis function does not reach the grid point.
   Vec delta(gridPoint-m_position);
   double  r2(delta*delta);
   if (r2 > m_significantRadius2) return 0;

   double x(delta.x);
   double y(delta.y);
   double z(delta.z);
   double z2r2(z*z-r2);
   double x2y2(x*x-y*y);
   double tmp(0.0);

   for (int i = 0; i < m_exponents.size(); ++i) {
       tmp += m_contractionCoefficients.at(i) * std::exp(-m_exponents.at(i) * r2);
   }

   switch (m_angMom) {

      case S:
         s_values[0] = (tmp);
         break;

      case P:
         s_values[0] = (tmp * x);
         s_values[1] = (tmp * y);
         s_values[2] = (tmp * z);
         break;

      case D5:
         s_values[0] = (tmp * (z*z - r2)  * 0.5 * rt3);
         s_values[1] = (tmp * (x*z)                  );
         s_values[2] = (tmp * (y*z)                  );
         s_values[3] = (tmp * (x*x - y*y) * 0.5      );
         s_values[4] = (tmp * (x*y)                  );
         break;

      case D6:
         s_values[0] = (tmp * (x*x) * rt3);
         s_values[1] = (tmp * (y*y) * rt3);
         s_values[2] = (tmp * (z*z) * rt3);
         s_values[3] = (tmp * (x*y)      );
         s_values[4] = (tmp * (x*x)      );
         s_values[5] = (tmp * (y*z)      );
         break;

      case F7:
         s_values[0] = (tmp * (z*z2r2) * rt8);
         s_values[1] = (tmp * (x*z2r2) * rt8);
         s_values[2] = (tmp * (y*z2r2) * rt8);
         s_values[3] = (tmp * (z*x2y2) * 0.5);
         s_values[4] = (tmp * (x*y*z)       );
         s_values[5] = (tmp * (x*x2y2) * 0.5);
         s_values[6] = (tmp * (y*x2y2) * 0.5);
         break;

      case F10:
         s_values[0] = (tmp * (x*x*x) * rt15);
         s_values[1] = (tmp * (y*y*y) * rt15);
         s_values[2] = (tmp * (z*z*z) * rt15);
         s_values[3] = (tmp * (x*x*y) * rt3 );
         s_values[4] = (tmp * (x*y*y) * rt3 );
         s_values[5] = (tmp * (x*x*z) * rt3 );
         s_values[6] = (tmp * (x*z*z) * rt3 );
         s_values[7] = (tmp * (y*z*z) * rt3 );
         s_values[8] = (tmp * (y*y*z) * rt3 );
         s_values[9] = (tmp * (x*y*z)       );
         break;
   }

   return s_values;
}


void Shell::dump() const
{
   qDebug() << "Shell data:";
   qDebug() << "   AngMom" << m_angMom << "  (size =" << m_size << ")";
   qDebug() << "   radius" << m_significantRadius2;
   qDebug() << "   position (" << m_position.x << "," << m_position.y << "," 
                              << m_position.z << ")";
   qDebug() << "   Coefficients and exponents:";

   for (int i = 0; i < m_exponents.size(); ++i) {
       qDebug() << "      " << m_contractionCoefficients.at(i) << "   " << m_exponents.at(i);
   }
   qDebug();
}


//! Evaluates the basis function pairs at the gridPoint.  These can be
//! contracted with the (spin) density vectors to give a density.
std::vector<double> Shell::evaluateShellPairs(ShellList const& shells, int const nBasis,
   Vec const& gridPoint)
{
   std::vector<double> shellValues;
   std::vector<double> shellPairValues;

   int N(nBasis);
   shellValues.reserve(N);
   shellPairValues.reserve(N*(N-1)/2);

   double* values;
   ShellList::const_iterator shell;
   for (shell = shells.begin(); shell != shells.end(); ++shell) {
       if ( (values = (*shell)->evaluate(gridPoint)) ) {
          for (unsigned int j = 0; j < (*shell)->size(); ++j) {
              shellValues.push_back(values[j]);
          }
       }else {
          for (unsigned int j = 0; j < (*shell)->size(); ++j) {
              shellValues.push_back(0.0);
          }
       }
   }

   double xi, xj;

   for (int i = 0; i < N; ++i) {
       xi = shellValues[i];
       shellPairValues.push_back(xi*xi);
       for (int j = i+1; j < N; ++j) {
           xj = shellValues[j];
           shellPairValues.push_back(xi*xj);
       }
   }

   return shellPairValues;
}


} // end namespace IQmol
