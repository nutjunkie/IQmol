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

#include "MultipoleExpansion.h"
#include "Constants.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID MultipoleExpansionList::TypeID = Type::MultipoleExpansionList;


MultipoleExpansion& MultipoleExpansion::operator+=(MultipoleExpansion const& that)
{
   if (m_order < that.m_order) expandMultipoleVector(that.m_order);
   int n(that.m_multipoles.size());
   for (int i = 0; i < n; ++i) {
       m_multipoles[i] += that.m_multipoles[i];
   }
   return *this;
}


MultipoleExpansion& MultipoleExpansion::addCharge(double const charge)
{
   if (m_order < 0) expandMultipoleVector(0);
   m_multipoles[Q] += charge;
   return *this;
}


MultipoleExpansion& MultipoleExpansion::addDipole(double const x, double const y, 
   double const z)
{
   if (m_order < 1) expandMultipoleVector(1);
   m_multipoles[X] += x;
   m_multipoles[Y] += y;
   m_multipoles[Z] += z;
   return *this;
}


MultipoleExpansion& MultipoleExpansion::addQuadrupole(double const xx, double const xy, 
   double const xz, double const yy, double const yz, double const zz) 
{
   if (m_order < 2) expandMultipoleVector(2);
   m_multipoles[XX] += xx;
   m_multipoles[XY] += xy;
   m_multipoles[XZ] += xz;
   m_multipoles[YY] += yy;
   m_multipoles[YZ] += yz;
   m_multipoles[ZZ] += zz;
   return *this;
}


MultipoleExpansion& MultipoleExpansion::addOctopole(double const xxx, double const xxy, 
   double const xxz, double const xyy, double const xyz, double const xzz, double const yyy, 
   double const yyz, double const yzz, double const zzz)
{  
   if (m_order < 3) expandMultipoleVector(3);
   m_multipoles[XXX] += xxx;
   m_multipoles[XXY] += xxy;
   m_multipoles[XXZ] += xxz;
   m_multipoles[XYY] += xyy;
   m_multipoles[XYZ] += xyz;
   m_multipoles[XZZ] += xzz;
   m_multipoles[YYY] += yyy;
   m_multipoles[YYZ] += yyz;
   m_multipoles[YZZ] += yzz;
   m_multipoles[ZZZ] += zzz;
   return *this;
}


void MultipoleExpansion::expandMultipoleVector(int const order)
{
   m_order = order;
   int size((m_order+1)*(m_order+2)*(m_order+3)/6);
   while (m_multipoles.size() < size) m_multipoles.append(0.0);
}


void MultipoleExpansion::dump() const
{
   qDebug() << "Multipole Expansion order:" << m_order;
   if (m_order < 0) return;

   qDebug() << "  charge    " << m_multipoles[Q];
   if (m_order < 1) return;

   qDebug() << "  dipole    " << m_multipoles[X] << m_multipoles[Y] << m_multipoles[Z];
   if (m_order < 2) return;

   qDebug() << "  quadrupole" << m_multipoles[XX] << m_multipoles[XY] << m_multipoles[XZ];
   qDebug() << "            " << m_multipoles[YY] << m_multipoles[YZ] << m_multipoles[ZZ];
   if (m_order < 3) return;

   qDebug() << "  octopole  "  << m_multipoles[XXX] << m_multipoles[XXY] << m_multipoles[XXZ];
   qDebug() << "            "  << m_multipoles[XYY] << m_multipoles[XYZ] << m_multipoles[XZZ];
   qDebug() << "            "  << m_multipoles[YYY] << m_multipoles[YYZ] << m_multipoles[YZZ];
   qDebug() << "            "  << m_multipoles[ZZZ];
}


/* gcc on Windows doesn't like this

template<>
void MultipoleExpansionList::dump() const
{
   qDebug() << "Total Multipole values";
   double q, Q(0);
   double x(0), y(0), z(0);
   for (int i = 0; i < size(); ++i) {
       q  = value(i)->moment(MultipoleExpansion::Q);
       x += q*value(i)->position().x*Constants::AngstromToBohr;
       y += q*value(i)->position().y*Constants::AngstromToBohr;
       z += q*value(i)->position().z*Constants::AngstromToBohr;
       Q += q;
   }
   qDebug() << "   Charge:" << Q;

   for (int i = 0; i < size(); ++i) {
       x += value(i)->moment(MultipoleExpansion::X);
       y += value(i)->moment(MultipoleExpansion::Y);
       z += value(i)->moment(MultipoleExpansion::Z);
   }
   qDebug() << "   Dipole:" << x << y << z;
}
*/

} } // end namespace IQmol::Data
