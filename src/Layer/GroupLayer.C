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

#include "GroupLayer.h"
#include "AtomLayer.h"
#include "BondLayer.h"
#include "Align.h"
#include "QMsgBox.h"
#include "ParseFile.h"
#include "LayerFactory.h"
#include <QFileInfo>

#include <QtDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {


Group::Group(QString const& label) : Primitive(label)
{
   setFlags(Qt::ItemIsSelectable |  Qt::ItemIsEnabled | Qt::ItemIsEditable);
}


Group::Group(PrimitiveList const& primitives, QString const& label) : Primitive(label)
{
   setFlags(Qt::ItemIsSelectable |  Qt::ItemIsEnabled | Qt::ItemIsEditable);
   addPrimitives(primitives);
}


Group::~Group()
{
   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       delete (*atom);
   }

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       delete (*bond);
   }
}


void Group::dump() const
{
   qDebug() << "Dumping atom coordinates in Group:";
   AtomList::const_iterator atom;
   Vec v;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       v = (*atom)->getPosition();
       qDebug() << "(x,y,z) = " << v.x << v.y << v.z;
   }
}


void Group::loadFromFile(QString const& filePath)
{
   QFileInfo info(filePath);
   QString name(info.completeBaseName());
   setText(name);

   setFlags(Qt::ItemIsSelectable |  Qt::ItemIsEnabled);

   Parser::ParseFile parser(filePath);
   parser.start();
   parser.wait();

   QStringList errors(parser.errors());
   if (!errors.isEmpty()) {
      QMsgBox::warning(0, "IQmol", errors.join("\n"));
   }

   IQmol::Data::Bank& bank(parser.data());
   Factory& factory(Factory::instance()); 
   List data(factory.toLayers(bank));
   List::iterator iter;

   for (iter = data.begin(); iter != data.end(); ++iter) {
       addAtoms((*iter)->findLayers<Atom>(Children));
       addBonds((*iter)->findLayers<Bond>(Children));
  }
}


void Group::addPrimitives(PrimitiveList const& primitives)
{
   AtomList atoms;
   BondList bonds;
   Atom* atom;
   Bond* bond;

   PrimitiveList::const_iterator primitive;
   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       if (atom = dynamic_cast<Atom*>(*primitive)) {
          atoms.append(atom);
       }else if (bond = dynamic_cast<Bond*>(*primitive)) {
          bonds.append(bond);
       }   
   }   

   addAtoms(atoms);
   addBonds(bonds);
}


void Group::addAtoms(AtomList const& atoms)
{
   AtomList::const_iterator atom;
   for (atom = atoms.begin(); atom != atoms.end(); ++atom) {
       (*atom)->setReferenceFrame(&m_frame);
       m_atoms.append(*atom);
   }
}


void Group::addBonds(BondList const& bonds)
{
   BondList::const_iterator bond;
   for (bond = bonds.begin(); bond != bonds.end(); ++bond) {
       (*bond)->setReferenceFrame(&m_frame);
       m_bonds.append(*bond);
   }
}


void Group::align(QList<Vec> const& current)
{
   AtomList atoms(getAtoms());
   int nAtoms(atoms.size());
   if (current.size() != nAtoms) {
      QMsgBox::warning(0, "IQmol", "Coordinate mismatch in Group alignment");
      return;
   }

   QList<double> weights;
   QList<Vec> reference;
   AtomList::iterator atom;
   for (atom = atoms.begin(); atom != atoms.end(); ++atom) {
       reference.append((*atom)->getTranslation());
       weights.append((*atom)->getMass());
   }

   Util::Align align(reference, current, weights);
   if (!align.computeAlignment()) {
      QMsgBox::warning(0, "IQmol", "Alignment failed in Group::align");
      return;
   }

   setRotation(align.rotation());
   setTranslation(align.translation());
}



PrimitiveList Group::ungroup()
{
   PrimitiveList primitives;

   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       Vec pos((*atom)->getPosition());
       (*atom)->setReferenceFrame(0);
       (*atom)->setPosition(pos);
       (*atom)->orphanLayer();
       primitives.append(*atom);
   }
   m_atoms.clear();

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->setReferenceFrame(0);
       (*bond)->orphanLayer();
       primitives.append(*bond);
   }

   m_bonds.clear();

   return primitives;
}


void Group::draw()
{
   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       (*atom)->draw();    
   }

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->draw();    
   }

   glPopMatrix();
}


void Group::drawSelected()
{
   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       (*atom)->drawSelected();    
   }

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->drawSelected();    
   }

   glPopMatrix();
}


void Group::drawFast()
{
   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       (*atom)->drawFast();    
   }

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->drawFast();    
   }

   glPopMatrix();
}


void Group::select()
{
   setProperty(Selected);

   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       (*atom)->select();    
   }

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->select();    
   }
}


void Group::deselect()
{
   unsetProperty(Selected);

   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       (*atom)->deselect();    
   }

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->deselect();    
   }
}


void Group::setAtomScale(double const scale)
{
   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       (*atom)->setScale(scale);    
   }
}


void Group::setBondScale(double const scale)
{
   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->setScale(scale);    
   }
}


void Group::setDrawMode(DrawMode const mode)
{
   AtomList::iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       (*atom)->setDrawMode(mode);    
   }

   BondList::iterator bond;
   for (bond = m_bonds.begin(); bond != m_bonds.end(); ++bond) {
       (*bond)->setDrawMode(mode);    
   }
}


Atom* Group::rootAtom() const
{
   Atom* atom(0);
   if (!m_atoms.isEmpty()) atom = m_atoms.first();
   return atom;
}

} } // end namespace IQmol::Layer
