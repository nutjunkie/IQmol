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
#include "MoleculeLayer.h"

#include <QDebug>


namespace IQmol {
namespace Layer {

Isotopes::Isotopes(AtomList const& atomList) : m_configurator(*this), m_accepted(false)
{
   setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

   m_configurator.loadTable(atomList);
   setConfigurator(&m_configurator);
}


void Isotopes::configure()
{
   if (m_configurator.exec() == QDialog::Accepted) {
      m_qchem = m_configurator.toString();
      m_accepted = true;
      updateLabels(m_configurator.makeMassList());
   }
}


void Isotopes::updateLabels()
{
   updateLabels(m_configurator.makeMassList());
}


void Isotopes::updateLabels(QMap<unsigned, double> const& masses)
{
   QList<Molecule*> list(findLayers<Molecule>(Parents));
   if (list.isEmpty()) return;
   Molecule* molecule(list.first());

   AtomList atoms(molecule->findLayers<Atom>());

   AtomList::iterator atom;
   for (atom = atoms.begin(); atom != atoms.end(); ++atom) {
       (*atom)->resetMass();
       unsigned index((*atom)->getIndex());
       if (masses.contains(index)) (*atom)->setMass(masses[index]);
   }
}

} } // end namespace IQmol::Layer
