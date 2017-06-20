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

#include "GeometryLayer.h"
#include "DipoleMoment.h"
#include "AtomicProperty.h"
#include "Geometry.h"
#include "Energy.h"


namespace IQmol {
namespace Layer {

Geometry::Geometry(Data::Geometry& geometry) : m_geometry(geometry)
{
   setText(QString::number(energy(),'f', 6));
   setToolTip(label());
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}


double Geometry::energy() const
{
   Data::Energy& energy(m_geometry.getProperty<Data::TotalEnergy>());
   return energy.value();
}


QString Geometry::label() const
{
   Data::Energy& energy(m_geometry.getProperty<Data::TotalEnergy>());
   return energy.label();
}


qglviewer::Vec Geometry::dipole() const
{
   Data::DipoleMoment& dipole(m_geometry.getProperty<Data::DipoleMoment>());
   return dipole.value();
}


unsigned Geometry::nAtoms() const
{
    return m_geometry.nAtoms();
}


qglviewer::Vec Geometry::atomicPosition(unsigned i) const
{
    return m_geometry.position(i);
}


double Geometry::atomicCharge(unsigned i) const
{
    return m_geometry.getAtomicProperty<Data::MullikenCharge>(i).value();
}


double Geometry::atomicSpin(unsigned i) const
{
    return m_geometry.getAtomicProperty<Data::SpinDensity>(i).value();
}

} } // end namespace IQmol::Layer
