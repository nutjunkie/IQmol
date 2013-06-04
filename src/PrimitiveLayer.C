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

#include "PrimitiveLayer.h"
#define _USE_MATH_DEFINES
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace Layer {


double Primitive::distance(Primitive* A, Primitive* B) 
{
   return (A->getPosition() - B->getPosition()).norm();
}


double Primitive::angle(Primitive* A, Primitive* B, Primitive* C) 
{
  if ( (A == B) || (B == C)) return 0.0;
  Vec u(B->getPosition() - A->getPosition());
  Vec v(B->getPosition() - C->getPosition());
  double theta(u * v / (u.norm() * v.norm())); 
  return std::acos(theta) * 180.0 / M_PI; 
}


double Primitive::torsion(Primitive* A, Primitive* B, Primitive* C , Primitive* D) 
{
   Vec a(B->getPosition() - A->getPosition());
   Vec b(C->getPosition() - B->getPosition());
   Vec c(D->getPosition() - C->getPosition());
   double x((b.norm() * a) * cross(b,c));
   double y(cross(a,b) * cross(b,c));
   return std::atan2(x,y) * 180.0 / M_PI;
}

} } // end namespace IQmol::Layer
