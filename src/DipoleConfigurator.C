/*******************************************************************************

  Copyright (C) 2011-2013 Andrew Gilbert

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

#include "IQmol.h"
#include "DipoleConfigurator.h"
#include "DipoleLayer.h"
#include <QColorDialog>


namespace IQmol {
namespace Configurator { 

Dipole::Dipole(Layer::Dipole* dipole) : m_dipole(dipole)
{
   m_dipoleConfigurator.setupUi(this);
}


void Dipole::sync()
{
   QString s(QString::number(m_dipole->m_value, 'f', 3));
   m_dipoleConfigurator.dipoleLabel->setText(s+" D");
   SetButtonColor(*(m_dipoleConfigurator.colorButton), m_dipole->m_color);
}


void Dipole::on_colorButton_clicked(bool)
{
   m_dipole->m_color = QColorDialog::getColor(m_dipole->m_color, this);
   SetButtonColor(*(m_dipoleConfigurator.colorButton), m_dipole->m_color);
   m_dipole->updated();
}


void Dipole::on_scaleSlider_valueChanged(int value)
{
   m_dipole->m_scale = value/100.0; 
   m_dipole->updated();
}

} } // end namespace IQmol::Configurator
