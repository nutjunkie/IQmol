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

#include "Surface.h"
#include "SurfaceInfo.h"
#include "Preferences.h"
#include "QsLog.h"
#include <QDebug>
#include <climits>


namespace IQmol {
namespace Data {



Surface::Surface(SurfaceInfo const& info) : m_min(0.0), m_max(0.0)
{
   m_opacity = info.opacity();
   m_description = info.toString();
// add isovalue info to description
   m_isSigned = info.isSigned();
   m_colors << info.negativeColor();
   m_colors << info.positiveColor();
   m_isVisible = false;
}


Surface::Surface(Mesh const& mesh) : m_opacity(0.999), m_min(0.0), m_max(0.0)
{
   m_description = "Mesh";
   m_meshPositive = mesh;
// add isovalue info to description
   m_isSigned = false;
   m_colors << Preferences::NegativeSurfaceColor();
   m_colors << Preferences::PositiveSurfaceColor();
}


void Surface::computeSurfaceProperty(Function3D const& function)
{
   m_meshPositive.computeScalarField(function);
   if (m_isSigned) m_meshNegative.computeScalarField(function);
   computeSurfacePropertyRange();
}


void Surface::computeIndexProperty()
{
   m_meshPositive.computeIndexField();
   if (m_isSigned) m_meshNegative.computeIndexField();
   computeSurfacePropertyRange();
}


bool Surface::hasProperty() const 
{ 
   return m_meshPositive.hasProperty(Mesh::ScalarField) || 
          m_meshPositive.hasProperty(Mesh::ScalarField);
}


void Surface::clearSurfaceProperty()
{
   m_meshPositive.deleteProperty(Mesh::ScalarField);
   if (m_isSigned) m_meshNegative.deleteProperty(Mesh::ScalarField);
   computeSurfacePropertyRange();
}


bool Surface::propertyIsSigned() const
{
   return (hasProperty() && m_min < 0 && m_max > 0);
}


void Surface::getPropertyRange(double& min, double& max) const
{
   min = m_min;
   max = m_max;
}


void Surface::computeSurfacePropertyRange()
{
   m_meshPositive.getScalarFieldRange(m_min, m_max);

   if (m_isSigned) {
      double min, max;
      m_meshNegative.getScalarFieldRange(min, max);
      m_min = std::min(min, m_min);
      m_max = std::max(max, m_max);
    }
}


void Surface::setColors(QList<QColor> const& colors) 
{ 
   m_colors = colors; 
}


double Surface::area() const
{
   return m_meshPositive.surfaceArea() + m_meshNegative.surfaceArea();
}


void Surface::dump() const
{
   qDebug() << " --- Surface --- ";
   m_meshPositive.dump();   
   m_meshNegative.dump();   
   qDebug() << "Color list has" << m_colors.size() << "colors:";
   qDebug() << m_colors;
   qDebug() << "Surface is visible?" << m_isVisible;
}

} } // end namespace IQmol::Data
