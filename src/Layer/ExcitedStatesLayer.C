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

#include "ExcitedStatesLayer.h"
#include "ExcitedStatesConfigurator.h"
#include "ExcitedStates.h"


namespace IQmol {
namespace Layer {


ExcitedStates::ExcitedStates(Data::ExcitedStates const& states) : Base("Excited States"),
   m_excitedStates(states), m_configurator(0)
{
}


ExcitedStates::~ExcitedStates()
{
   if (m_configurator) delete m_configurator;
}


void ExcitedStates::configure()
{
   if (!m_configurator) {
      m_configurator = new Configurator::ExcitedStates(*this);
      if (!m_configurator) return;
      m_configurator->load(m_excitedStates);
   }

   if (m_configurator) m_configurator->display();
}


} } // end namespace IQmol::Layer
