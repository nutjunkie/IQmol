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

#include "SymmetryLayer.h"


namespace IQmol {
namespace Layer {

Symmetry::Symmetry(Data::PointGroup const& pointGroup) : Base("Symmetry"), 
   m_configurator(*this), m_pointGroup(pointGroup)
{
   setConfigurator(&m_configurator);
}


void Symmetry::update()
{
   m_configurator.sync();
}


} } // end namespace IQmol::Layer
