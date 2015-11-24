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

#include "NmrConfigurator.h"
#include "NmrLayer.h"
#include "NmrData.h"
#include "MoleculeLayer.h"


namespace IQmol {
namespace Layer {

Nmr::Nmr(Data::Nmr& data) : Base("NMR"), m_data(data), m_configurator(0)
{
}


Nmr::~Nmr()
{
   if (m_configurator) delete m_configurator;
}


void Nmr::configure()
{
   if (!m_configurator) {
      m_configurator = new Configurator::Nmr(*this, m_data);
      if (m_molecule) {
         connect(m_configurator, SIGNAL(selectAtoms(QList<int> const&)), 
            m_molecule, SLOT(selectAtoms(QList<int> const&)));
      }
   }
   m_configurator->display();
}

} } // end namespace IQmol::Layer
