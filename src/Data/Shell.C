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

#include "Shell.h"
#include <QDebug>
#include <cmath>


using qglviewer::Vec;

namespace IQmol {
namespace Data {

double Shell::s_values[15];
double Shell::s_zeroValues[15] = {  };
double Shell::s_thresh = 0.001;


Shell::Shell(AngularMomentum L, Vec const& position, QList<double> const& exponents, 
   QList<double> const& contractionCoefficients) : m_angularMomentum(L), m_position(position), 
   m_exponents(exponents), m_contractionCoefficients(contractionCoefficients)
{
   if (m_exponents.size() > m_contractionCoefficients.size()) {
      qDebug() << "Inconsistent array lengths in Shell constructor";
      while (m_exponents.size() > m_contractionCoefficients.size()) {
         m_contractionCoefficients.append(0.0);
      }
   }

   normalize();
   m_significantRadiusSquared  = computeSignificantRadius(s_thresh); 
   m_significantRadiusSquared *= m_significantRadiusSquared;
}


void Shell::normalize()
{
   static double Ns  = std::pow(2.0/M_PI, 0.75);
   double pf(0.0), ex(0.0);

   switch (m_angularMomentum) {
      case S:    pf =     Ns;  ex = 0.75;  break;
      case P:    pf = 2.0*Ns;  ex = 1.25;  break;
      case D5:   pf = 4.0*Ns;  ex = 1.75;  break;
      case D6:   pf = 4.0*Ns;  ex = 1.75;  break;
      case F7:   pf = 8.0*Ns;  ex = 2.25;  break;
      case F10:  pf = 8.0*Ns;  ex = 2.25;  break;
      case G9:   pf =16.0*Ns;  ex = 2.75;  break;
      case G15:  pf =16.0*Ns;  ex = 2.75;  break;
   }   

   for (int i = 0; i < m_exponents.size(); ++i) {
       m_contractionCoefficients[i] *= pf * std::pow(m_exponents[i], ex);
   }   
}

  
unsigned Shell::nBasis() const
{
    unsigned n(0);
    switch (m_angularMomentum) {
       case S:    n =  1;  break;
       case P:    n =  3;  break;
       case D5:   n =  5;  break;
       case D6:   n =  6;  break;
       case F7:   n =  7;  break;
       case F10:  n = 10;  break;
       case G9:   n =  9;  break;
       case G15:  n = 15;  break;
   }
   return n;
}


// Attempts to give an approximate size of the basis function for use in cutoffs
double Shell::computeSignificantRadius(double const thresh) 
{
   double radius(0.0);
   double alpha, coeff, r, f;
   double const step(0.5);
   int L(0);

   switch (m_angularMomentum) {
      case S:    L = 0;  break;
      case P:    L = 1;  break;
      case D5:   L = 2;  break;
      case D6:   L = 2;  break;
      case F7:   L = 3;  break;
      case F10:  L = 3;  break;
      case G9:   L = 4;  break;
      case G15:  L = 4;  break;
   }

   for (int i = 0; i < m_exponents.size(); ++i) {
       alpha = m_exponents[i];
       coeff = std::abs(m_contractionCoefficients[i]);
	   // This is the turning point of the primitive, things should all be
	   // downhill from here.
       r = std::sqrt(0.5*L/alpha);
       f = coeff * std::pow(r, L) * std::exp(-alpha*r*r);

	   // This is a little crusty, but to do things analytically 
	   // requires product logs
       while ((f > thresh) && (r < 100.0)) {
          r += step;
          f = coeff * std::pow(r, L) * std::exp(-alpha*r*r);
       }
       radius = std::max(radius, r);
   }

   return radius;
}


QString Shell::toString(AngularMomentum const L) const
{
   QString s;
   switch (L) {
      case S:    s = "S";    break;
      case P:    s = "P";    break;
      case D5:   s = "D5";   break;
      case D6:   s = "D6";   break;
      case F7:   s = "F7";   break;
      case F10:  s = "F10";  break;
      case G9:   s = "G9";   break;
      case G15:  s = "G15";  break;
   }
   return s;
}


void Shell::boundingBox(Vec& min, Vec& max) 
{
   double r(std::sqrt(m_significantRadiusSquared));
   Vec d(r, r, r);
   min = m_position - d;
   max = m_position + d;
}


// returns a null pointer if grid point is outside the significant radius
double* Shell::evaluate(Vec const& gridPoint) const
{
   static const double half   = 0.5;
   static const double quart  = 0.25;
   static const double eighth = 0.125;
   static const double rt3    = std::sqrt(3.0);
   static const double rt5    = std::sqrt(5.0);
   static const double rt7    = std::sqrt(7.0);
   static const double rt15   = std::sqrt(15.0);
   static const double rt35   = std::sqrt(35.0);
   static const double rt70   = std::sqrt(70.0);

   static const double rt3o8  = std::sqrt(3.0/8.0);
   static const double rt5o8  = std::sqrt(5.0/8.0);
   static const double rt35o3 = std::sqrt(35.0/3.0);
   static const double hrt3   = half*rt3;
   static const double hrt15  = half*rt15;

   // bail early if the basis function does not reach the grid point.
   Vec delta(gridPoint-m_position);
   double  r2(delta*delta);
   if (r2 > m_significantRadiusSquared) return s_zeroValues;

   double x(delta.x);
   double y(delta.y);
   double z(delta.z);
   double s(0.0);

   for (int i = 0; i < m_exponents.size(); ++i) {
       s += m_contractionCoefficients.at(i) * std::exp(-m_exponents.at(i) * r2);
   }

   switch (m_angularMomentum) {

      case S:
         s_values[0] = s;
         break;

      case P:
         // X Y Z
         s_values[0] = s * x;
         s_values[1] = s * y;
         s_values[2] = s * z;
         break;

      case D5:
         // 3ZZ-RR  XZ  YZ  XX-YY  XY
         s_values[0] = s * (3*z*z - r2) * half;
         s_values[1] = s * (x*z)        *  rt3;
         s_values[2] = s * (y*z)        *  rt3;
         s_values[3] = s * (x*x - y*y)  * hrt3;
         s_values[4] = s * (x*y)        *  rt3;
         break;

      case D6:
         // XX  YY  ZZ  XY  XZ  YZ
         s_values[0] = s * (x*x)      ;
         s_values[1] = s * (y*y)      ;
         s_values[2] = s * (z*z)      ;
         s_values[3] = s * (x*y) * rt3;
         s_values[4] = s * (x*z) * rt3;
         s_values[5] = s * (y*z) * rt3;
         break;

      case F7:
         // ZZZ-ZRR  XZZ-XRR  YZZ-YRR  XXZ-YYZ  XYZ  XXX-XYY  XXY-YYY
         s_values[0] = s * z * (5*z*z - 3*r2 ) * half ;
         s_values[1] = s * x * (5*z*z -   r2 ) * rt3o8;
         s_values[2] = s * y * (5*z*z -   r2 ) * rt3o8;
         s_values[3] = s * z * (  x*x -   y*y) * hrt15;
         s_values[4] = s * x*y*z               * rt15 ;
         s_values[5] = s * x * (  x*x - 3*y*y) * rt5o8;
         s_values[6] = s * y * (3*x*x -   y*y) * rt5o8;
      break;

      case F10:
         // XXX  YYY  ZZZ  XYY  XXY  XXZ  XZZ  YZZ  YYZ  XYZ
         s_values[0] = s * (x*x*x)       ;
         s_values[1] = s * (y*y*y)       ;
         s_values[2] = s * (z*z*z)       ;
         s_values[3] = s * (x*y*y) * rt5 ;
         s_values[4] = s * (x*x*y) * rt5 ;
         s_values[5] = s * (x*x*z) * rt5 ;
         s_values[6] = s * (x*z*z) * rt5 ;
         s_values[7] = s * (y*z*z) * rt5 ;
         s_values[8] = s * (y*y*z) * rt5 ;
         s_values[9] = s * (x*y*z) * rt15;
         break;

      case G9: {
         double x2(x*x), y2(y*y), z2(z*z);
         s_values[0] = s * (3*r2*r2 - 30*r2*z2 + 35*z2*z2) * eighth     ;
         s_values[1] = s *  x*z      * (7*z2 - 3*r2)       * rt5o8      ;
         s_values[2] = s *  y*z      * (7*z2 - 3*r2)       * rt5o8      ;
         s_values[3] = s * (x2 - y2) * (7*z2 -   r2)       * rt5*quart  ; 
         s_values[4] = s *  x*y      * (7*z2 -   r2)       * rt5*half   ; 
         s_values[5] = s *  x*z      * (  x2 - 3*y2)       * rt70*quart ;
         s_values[6] = s *  y*z      * (3*x2 -   y2)       * rt70*quart ;
         s_values[7] = s * (x2*x2 - 6*x2*y2 + y2*y2)       * rt35*eighth;
         s_values[8] = s *  x*y      * (  x2 -   y2)       * rt35*half  ;
      }  break;

      case G15:
         // XXXX YYYY ZZZZ XXXY XXXZ XYYY YYYZ ZZZX ZZZY XXYY XXZZ YYZZ XXYZ XYYZ XYZZ
         s_values[ 0] = s * (x*x*x*x)         ;
         s_values[ 1] = s * (y*y*y*y)         ;
         s_values[ 2] = s * (z*z*z*z)         ;
         s_values[ 3] = s * (x*x*x*y) * rt7   ;
         s_values[ 4] = s * (x*x*x*z) * rt7   ;
         s_values[ 5] = s * (x*y*y*y) * rt7   ;
         s_values[ 6] = s * (y*y*y*z) * rt7   ;
         s_values[ 7] = s * (x*z*z*z) * rt7   ;
         s_values[ 8] = s * (y*z*z*z) * rt7   ;
         s_values[ 9] = s * (x*x*y*y) * rt35o3;
         s_values[10] = s * (x*x*z*z) * rt35o3;
         s_values[11] = s * (y*y*z*z) * rt35o3;
         s_values[12] = s * (x*x*y*z) * rt35  ;
         s_values[13] = s * (x*y*y*z) * rt35  ;
         s_values[14] = s * (x*y*z*z) * rt35  ;
         break;
   }

   return s_values;
}


void Shell::dump() const
{
   qDebug() << "Shell data:";
   qDebug() << "   L =" << toString(m_angularMomentum) << "   K =" << m_exponents.size() 
            << "   r^2 =" << QString::number(m_significantRadiusSquared,'f',3)
            << "   at (" << m_position.x << "," << m_position.y << "," << m_position.z << ")";

   for (int i = 0; i < m_exponents.size(); ++i) {
       qDebug() << "      " <<  m_exponents[i] << "   " << m_contractionCoefficients[i];
   }
}

} } // end namespace IQmol::Data
