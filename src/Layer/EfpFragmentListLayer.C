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

#include "EfpFragmentListLayer.h"
#include "EfpFragmentLayer.h"
#include "BondLayer.h"
#include "AtomLayer.h"
#include "Viewer.h"


namespace IQmol {
namespace Layer {


EfpFragmentList::EfpFragmentList(Layer::Base* parent) : Base("EFP Fragments"), m_atomScale(0.5),
   m_bondScale(0.5), m_drawMode(Primitive::Tubes), m_configurator(*this) 
{
   setConfigurator(&m_configurator);
   setPersistentParent(parent);
   setProperty(RemoveWhenChildless);
   setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
   setCheckState(Qt::Checked);
}


void EfpFragmentList::appendLayer(EfpFragment* efpFragment)
{
   efpFragment->setDrawMode(m_drawMode);
   efpFragment->setAtomScale(m_atomScale);
   efpFragment->setBondScale(m_bondScale);
   Base::appendLayer(efpFragment);
}


void EfpFragmentList::setDrawMode(Primitive::DrawMode const drawMode)
{
   m_drawMode = drawMode;
   QList<EfpFragment*> efps(findLayers<EfpFragment>());
   QList<EfpFragment*>::iterator iter;
   for (iter = efps.begin(); iter != efps.end(); ++iter) (*iter)->setDrawMode(m_drawMode);
   updated();
}


void EfpFragmentList::setAtomScale(double const scale)
{
   m_atomScale = scale;
   QList<EfpFragment*> efps(findLayers<EfpFragment>());
   QList<EfpFragment*>::iterator iter;
   for (iter = efps.begin(); iter != efps.end(); ++iter) (*iter)->setAtomScale(m_atomScale);
   updated();
}


void EfpFragmentList::setBondScale(double const scale)
{
   m_bondScale = scale;

   QList<EfpFragment*> efps(findLayers<EfpFragment>());
   QList<EfpFragment*>::iterator iter;
   for (iter = efps.begin(); iter != efps.end(); ++iter) (*iter)->setBondScale(m_bondScale);
   updated();
}


} } // end namespace IQmol::Layer
