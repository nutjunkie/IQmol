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

#include "AxesConfigurator.h"
#include "AxesLayer.h"
#include <QtOpenGL>
#include <QtDebug>
#include <cmath>


namespace IQmol {
namespace Configurator {

Axes::Axes(Layer::Axes& axes) : m_axes(axes)
{
   m_axesConfigurator.setupUi(this);
}


void Axes::sync()
{
   m_axesConfigurator.xCheckBox->setChecked(m_axes.m_xAxisOn);
   m_axesConfigurator.yCheckBox->setChecked(m_axes.m_yAxisOn);
   m_axesConfigurator.zCheckBox->setChecked(m_axes.m_zAxisOn);
   m_axesConfigurator.scaleSlider->setValue(100*m_axes.m_scale);
}


void Axes::on_xCheckBox_clicked(bool tf) 
{ 
   m_axes.m_xAxisOn = tf; 
   m_axes.updated();
}


void Axes::on_yCheckBox_clicked(bool tf) 
{ 
   m_axes.m_yAxisOn = tf; 
   m_axes.updated();
}


void Axes::on_zCheckBox_clicked(bool tf) 
{ 
   m_axes.m_zAxisOn = tf; 
   m_axes.updated();
}


void Axes::on_scaleSlider_valueChanged(int value) 
{
   m_axes.m_scale = value/100.0; 
   m_axes.updated();
}


void Axes::on_okButton_clicked(bool)
{
   accept();
}

} } // end namespace IQmol::Layer
