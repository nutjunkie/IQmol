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

#include "MoleculeConfigurator.h"
#include "MoleculeLayer.h"
#include "SurfaceInfo.h"

#include<QtDebug>


namespace IQmol {
namespace Configurator {

Molecule::Molecule(Layer::Molecule& molecule) : m_molecule(molecule)
{
   m_moleculeConfigurator.setupUi(this);
   m_moleculeConfigurator.atomRadiusScale->setValue(10*m_molecule.m_atomScale);
   m_moleculeConfigurator.bondRadiusScale->setValue(10*m_molecule.m_bondScale);
   m_moleculeConfigurator.chargeRadiusScale->setValue(10*m_molecule.m_chargeScale);
   m_moleculeConfigurator.smallerHydrogens->setChecked(m_molecule.m_smallerHydrogens);
}


void Molecule::on_resetButton_clicked(bool)
{
   m_moleculeConfigurator.ballsAndSticks->setChecked(true);
   on_ballsAndSticks_clicked(true);
   m_drawMode = Layer::Primitive::BallsAndSticks;

   m_moleculeConfigurator.atomRadiusScale->setValue(10);
   m_moleculeConfigurator.bondRadiusScale->setValue(10);
   m_moleculeConfigurator.chargeRadiusScale->setValue(10);
}


void Molecule::on_ballsAndSticks_clicked(bool)
{
   m_drawMode = Layer::Primitive::BallsAndSticks;
   m_molecule.updateDrawMode(m_drawMode);
   m_moleculeConfigurator.atomRadiusScale->setEnabled(true);
   m_moleculeConfigurator.bondRadiusScale->setEnabled(true);
   m_moleculeConfigurator.smallerHydrogens->setEnabled(true);
}


void Molecule::on_tubes_clicked(bool)
{
   m_drawMode = Layer::Primitive::Tubes;
   m_molecule.updateDrawMode(m_drawMode);
   m_moleculeConfigurator.atomRadiusScale->setValue(
      m_moleculeConfigurator.bondRadiusScale->value());
   m_moleculeConfigurator.atomRadiusScale->setEnabled(true);
   m_moleculeConfigurator.bondRadiusScale->setEnabled(true);
   m_moleculeConfigurator.smallerHydrogens->setEnabled(false);
}


void Molecule::on_spaceFilling_clicked(bool)
{
   m_drawMode = Layer::Primitive::SpaceFilling;
   m_molecule.updateDrawMode(m_drawMode);
   m_moleculeConfigurator.atomRadiusScale->setEnabled(true);
   m_moleculeConfigurator.bondRadiusScale->setEnabled(false);
   m_moleculeConfigurator.smallerHydrogens->setEnabled(false);
}


void Molecule::on_wireFrame_clicked(bool)
{
   m_drawMode = Layer::Primitive::WireFrame;
   m_molecule.updateDrawMode(m_drawMode);
   m_moleculeConfigurator.atomRadiusScale->setValue(
      m_moleculeConfigurator.bondRadiusScale->value());
   m_moleculeConfigurator.atomRadiusScale->setEnabled(true);
   m_moleculeConfigurator.bondRadiusScale->setEnabled(true);
   m_moleculeConfigurator.smallerHydrogens->setEnabled(false);
}


void Molecule::on_plastic_clicked(bool)
{
   m_drawMode = Layer::Primitive::Plastic;
   m_molecule.updateDrawMode(m_drawMode);
/*
   m_moleculeConfigurator.atomRadiusScale->setEnabled(false);
   m_moleculeConfigurator.bondRadiusScale->setEnabled(false);
   m_moleculeConfigurator.smallerHydrogens->setEnabled(false);
*/
}


void Molecule::on_smallerHydrogens_clicked(bool tf)
{
   m_molecule.updateSmallerHydrogens(tf);
}


void Molecule::on_atomRadiusScale_valueChanged(int value)
{
   if (m_drawMode == Layer::Primitive::Tubes || 
       m_drawMode == Layer::Primitive::WireFrame) {
      m_moleculeConfigurator.bondRadiusScale->setValue(
         m_moleculeConfigurator.atomRadiusScale->value());
   }
   m_molecule.updateAtomScale(value/10.0);
}


void Molecule::on_bondRadiusScale_valueChanged(int value)
{
   if (m_drawMode == Layer::Primitive::Tubes || 
       m_drawMode == Layer::Primitive::WireFrame) {
      m_moleculeConfigurator.atomRadiusScale->setValue(
         m_moleculeConfigurator.bondRadiusScale->value());
      m_molecule.updateAtomScale(value/10.0);
   }
   m_molecule.updateBondScale(value/10.0);
}


void Molecule::on_chargeRadiusScale_valueChanged(int value)
{
   m_molecule.updateChargeScale(value/10.0);
}

} } // end namespace IQmol::Configurator
