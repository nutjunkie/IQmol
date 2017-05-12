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

#include "Constraint.h"

#include <QDebug>


namespace IQmol {
namespace Data {


void Constraint::dump() const
{
   qDebug() << " - value:" << m_value;
}


// --------------- Position Constraint ---------------

PositionConstraint::PositionConstraint(unsigned const atomIndex, 
   qglviewer::Vec const& position)
{
   m_atomIndices << atomIndex;
   m_position = position;
}


void PositionConstraint::dump() const
{
  if (m_atomIndices.size() == 1) {
  qDebug() << "Constraint:" << m_atomIndices[0] << "Position:" 
            << m_position.x << m_position.y << m_position.z;
  }else {
     qDebug() << "Invalid position constraint";
  }
}


// --------------- Distance Constraint ---------------

DistanceConstraint::DistanceConstraint(unsigned const atomIndexA, unsigned const atomIndexB, 
   double const distance)
{
   m_atomIndices << atomIndexA << atomIndexB;
   m_value = distance;
}


void DistanceConstraint::dump() const
{
   if (m_atomIndices.size() == 2) {
      qDebug() << "Constraint:" << m_atomIndices[0] << m_atomIndices[1] 
               << "Distance:" << m_value;
   }else {
     qDebug() << "Invalid distance constraint";
   }
}


// --------------- Angle Constraint ---------------

AngleConstraint::AngleConstraint(unsigned const atomIndexA, unsigned const atomIndexB, 
   unsigned const atomIndexC, double const angle)
{
   m_atomIndices << atomIndexA << atomIndexB << atomIndexC;
   m_value = angle;
}


void AngleConstraint::dump() const
{
   if (m_atomIndices.size() == 3) {
      qDebug() << "Constraint:" << m_atomIndices[0] << m_atomIndices[1] 
               << m_atomIndices[2] << "Angle:" << m_value;
   }else {
     qDebug() << "Invalid angle constraint";
   }
}


// --------------- Torsion Constraint ---------------

TorsionConstraint::TorsionConstraint(unsigned const atomIndexA, unsigned const atomIndexB, 
   unsigned const atomIndexC, unsigned const atomIndexD, double const torsion)
{
   m_atomIndices << atomIndexA << atomIndexB << atomIndexC << atomIndexD;
   m_value = torsion;
}


void TorsionConstraint::dump() const
{
   if (m_atomIndices.size() == 4) {
      qDebug() << "Constraint:" << m_atomIndices[0] << m_atomIndices[1] 
               << m_atomIndices[2] << m_atomIndices[3] << "Torsion:" << m_value;
   }else {
     qDebug() << "Invalid torsion constraint";
   }
}

} } // end namespace IQmol::Data
