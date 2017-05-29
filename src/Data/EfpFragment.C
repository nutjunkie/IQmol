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

#include "EfpFragment.h"
#include "EfpFragmentLibrary.h"
#include "EulerAngles.h"
#include "Align.h"
#include "openbabel/mol.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID List<EfpFragment>::TypeID = Type::EfpFragmentList;


EfpFragment::EfpFragment(QString const& name, Vec const& position, double const alpha, 
   double const beta, double const gamma) : m_name(name), m_position(position)
{
   setEulerAngles(alpha, beta, gamma);
}
            

void EfpFragment::setEulerAngles(double const alpha, double const beta, double const gamma)
{ 
   m_orientation = Util::EulerAngles::toQuaternion(alpha, beta, gamma);
}


bool EfpFragment::align(QList<Vec> const& coordinates)
{
   EfpFragmentLibrary& library(EfpFragmentLibrary::instance());
   bool defined(library.defined(m_name) || library.add(m_name));
   if (!defined) return false;

   Geometry const& geometry(library.geometry(m_name));
   if (geometry.nAtoms() != (unsigned)coordinates.size()) return false;
   
   QList<double> weights;
   for (unsigned i = 0; i < geometry.nAtoms(); ++i) {
       weights.append(OpenBabel::etab.GetMass(geometry.atomicNumber(i))); 
   }

   Util::Align align(geometry.coordinates(), coordinates, weights);
   if (!align.computeAlignment()) return false;

   m_position = align.translation();
   m_orientation = align.rotation();

   return true;
}



void EfpFragment::dump() const
{
   qDebug() << "EFP Fragment   " << m_name;
   qDebug() << "    Position   " << m_position.x << m_position.y << m_position.z;
   qDebug() << "    Orientation" << m_orientation[0] << m_orientation[1] 
                                 << m_orientation[2] << m_orientation[3];
}

} } // end namespace IQmol::Data
