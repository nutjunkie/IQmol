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

#include "EFPFragmentListConfigurator.h"
#include "EFPFragmentListLayer.h"


namespace IQmol {
namespace Configurator {

EFPFragmentList::EFPFragmentList(Layer::EFPFragmentList* efpFragmentList) : 
   m_efpFragmentList(efpFragmentList)
{
   m_efpFragmentListConfigurator.setupUi(this);
   on_resetButton_clicked(true);
}


void EFPFragmentList::on_resetButton_clicked(bool)
{
   m_efpFragmentListConfigurator.tubes->setChecked(true);
   on_tubes_clicked(true);
   m_efpFragmentListConfigurator.atomRadiusScale->setValue(10);
   m_efpFragmentListConfigurator.bondRadiusScale->setValue(10);
}


void EFPFragmentList::on_ballsAndSticks_clicked(bool)
{
   m_efpFragmentList->setDrawMode(Layer::Primitive::BallsAndSticks);
   m_efpFragmentListConfigurator.bondRadiusScale->setEnabled(true);
}


void EFPFragmentList::on_tubes_clicked(bool)
{
   m_efpFragmentList->setDrawMode(Layer::Primitive::Tubes);
   m_efpFragmentListConfigurator.atomRadiusScale->setValue(
      m_efpFragmentListConfigurator.bondRadiusScale->value());
   m_efpFragmentListConfigurator.bondRadiusScale->setEnabled(true);
}


void EFPFragmentList::on_spaceFilling_clicked(bool)
{
   m_efpFragmentList->setDrawMode(Layer::Primitive::SpaceFilling);
   m_efpFragmentListConfigurator.bondRadiusScale->setEnabled(false);
}


void EFPFragmentList::on_wireFrame_clicked(bool)
{
   m_efpFragmentList->setDrawMode(Layer::Primitive::WireFrame);
   m_efpFragmentListConfigurator.atomRadiusScale->setValue(
      m_efpFragmentListConfigurator.bondRadiusScale->value());
   m_efpFragmentListConfigurator.bondRadiusScale->setEnabled(true);
}


void EFPFragmentList::on_atomRadiusScale_valueChanged(int value)
{
   Layer::Primitive::DrawMode drawMode(m_efpFragmentList->m_drawMode);

   if (drawMode == Layer::Primitive::Tubes || 
       drawMode == Layer::Primitive::WireFrame) {
      m_efpFragmentListConfigurator.bondRadiusScale->setValue(
         m_efpFragmentListConfigurator.atomRadiusScale->value());
      m_efpFragmentList->setBondScale(value/20.0);
   }
   m_efpFragmentList->setAtomScale(value/20.0);
}


void EFPFragmentList::on_bondRadiusScale_valueChanged(int value)
{
   Layer::Primitive::DrawMode drawMode(m_efpFragmentList->m_drawMode);

   if (drawMode == Layer::Primitive::Tubes || 
       drawMode == Layer::Primitive::WireFrame) {
      m_efpFragmentListConfigurator.atomRadiusScale->setValue(
         m_efpFragmentListConfigurator.bondRadiusScale->value());
      m_efpFragmentList->setAtomScale(value/20.0);
   }
   m_efpFragmentList->setBondScale(value/20.0);
}



} } // end namespace IQmol::Configurator
