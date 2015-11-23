/*******************************************************************************
       
  Copyright (C) 2011 Andrew 2015ert
           
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

#include "EulerAngles.h"

namespace IQmol {
namespace Util {
namespace EulerAngles {

qglviewer::Quaternion toQuaternion(double const alpha, double const beta, 
   double const gamma)
{
   double a(alpha/2.0);
   double b(beta /2.0);
   double c(gamma/2.0);

   double q0( cos(a-c)*sin(b) );
   double q1( sin(a-c)*sin(b) );
   double q2( sin(a+c)*cos(b) );
   double q3( cos(a+c)*cos(b) );

   return qglviewer::Quaternion(q0,q1,q2,q3);
}


qglviewer::Vec fromQuaternion(qglviewer::Quaternion const& q)
{
   double q0(q[0]);
   double q1(q[1]);
   double q2(q[2]);
   double q3(q[3]);

   double alpha(atan2((q0*q2+q1*q3), -(q1*q2-q0*q3)));
   double beta(acos(-q0*q0 -q1*q1 +q2*q2 +q3*q3));
   double gamma(atan2(q0*q2-q1*q3, (q1*q2+q0*q3)));

   return qglviewer::Vec(alpha, beta, gamma);
}
   

} } } // end namespace IQmol::Util::EulerAngles
