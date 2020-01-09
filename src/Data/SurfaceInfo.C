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

#include "SurfaceInfo.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID SurfaceInfoList::TypeID = Type::SurfaceInfoList;


SurfaceInfo::SurfaceInfo(SurfaceType const& type, unsigned const quality, 
   double const isovalue, QColor const& positive, QColor const& negative, bool isSigned,
   bool simplifyMesh, double const opacity, bool const isovalueIsPercent) 
 : m_surfaceType(type), m_quality(quality), m_isovalue(isovalue),  
   m_positiveColor(positive), m_negativeColor(negative), 
   m_isSigned(isSigned), m_simplifyMesh(simplifyMesh), m_opacity(opacity), 
   m_isovalueIsPercent(isovalueIsPercent)
{
}


QString SurfaceInfo::toString() const
{
    return m_surfaceType.toString();
}


bool SurfaceInfo::isSigned() const
{
  return m_isSigned || m_surfaceType.isSigned();
}


void SurfaceInfo::dump() const
{
   m_surfaceType.dump();
   qDebug() << " Quality: " << m_quality << "Isovalue" << m_isovalue;
}
            

} } // end namespace IQmol::Data
