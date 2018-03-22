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

#include "IsotopesConfigurator.h"
#include "IsotopesLayer.h"

#include <QComboBox>
#include <QSpinBox>
#include <QHeaderView>
#include "openbable/data.h"


namespace IQmol {
namespace Configurator {

Isotopes::Isotopes(Layer::Isotopes& isotopes) : m_isotopes(isotopes) 
{ 
   m_configurator.setupUi(this);

   QTableWidget* table(m_configurator.isotopeTable);

   table->setColumnCount( 3 );
   table->setRowCount( 1 );
   table->setCellWidget ( 0, 1, new QComboBox( table ) );
   table->setCellWidget ( 0, 2, new QSpinBox( table ) );
   table->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
   table->horizontalHeader()->setSectionResizeMode( 2, QHeaderView::ResizeToContents );
}


void Isotopes::loadMasses(unsigned Z, QComboBox* combo)
{
   openbabel::OBIsotopeTable obIso;
   
   for (unsigned i = 0; i < 10; ++i) {
       GetExactMass();

   }
}


} } // end namespace IQmol::Configurator
