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

#include "MeshLayer.h"
#include <QtOpenGL>
#include <QtDebug>
#include <cmath>


namespace IQmol {
namespace Configurator {

Mesh::Mesh(Layer::Mesh* mesh) : m_mesh(mesh)
{
   m_meshConfigurator.setupUi(this);
}


void Mesh::sync()
{
   m_meshConfigurator.xyCheckBox->setChecked (m_mesh->m_xy);
   m_meshConfigurator.xzCheckBox->setChecked (m_mesh->m_xz);
   m_meshConfigurator.yzCheckBox->setChecked (m_mesh->m_yz);
   m_meshConfigurator.polarButton->setChecked(m_mesh->m_polar);
   m_meshConfigurator.stepButton->setChecked (m_mesh->m_useStepSize);
   m_meshConfigurator.ticksSpinBox->setValue (m_mesh->m_ticks);
   m_meshConfigurator.stepSpinBox->setValue  (m_mesh->m_stepSize);
}


void Mesh::on_xyCheckBox_clicked(bool tf) 
{ 
   m_mesh->m_xy = tf; 
   m_mesh->updated();
}


void Mesh::on_xzCheckBox_clicked(bool tf) 
{
   m_mesh->m_xz = tf; 
   m_mesh->updated();
}


void Mesh::on_yzCheckBox_clicked(bool tf) 
{
   m_mesh->m_yz = tf; 
   m_mesh->updated();
}


void Mesh::on_ticksSpinBox_valueChanged(int value)
{
   m_mesh->m_ticks = value;
   m_mesh->updated();
}


void Mesh::on_stepSpinBox_valueChanged(double value)
{
   m_mesh->m_stepSize = value;
   m_mesh->updated();
}


void Mesh::on_polarButton_clicked(bool tf)
{
   m_mesh->m_polar = tf;
   m_mesh->updated();
}

void Mesh::on_cartesianButton_clicked(bool tf)
{
   m_mesh->m_polar = !tf;
   m_mesh->updated();
}

void Mesh::on_totalButton_clicked(bool tf)
{
   m_mesh->m_useStepSize = !tf;
   m_mesh->updated();
}

void Mesh::on_stepButton_clicked(bool tf)
{
   m_mesh->m_useStepSize = tf;
   m_mesh->updated();
}

} } // end namespace IQmol::Layer
