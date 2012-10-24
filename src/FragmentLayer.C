/*******************************************************************************

  Copyright (C) 2011 Andrew Gilbert

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

#include "FragmentLayer.h"
#include "MoleculeLayer.h"
#include "QMsgBox.h"
#include <QtDebug>


using namespace qglviewer;

namespace IQmol {
namespace Layer {

int Fragment::s_count = 1;

Fragment::Fragment(PrimitiveList const& primitives) 
 : Primitive("Fragment " + QString::number(s_count))
{
   setFlags(Qt::ItemIsSelectable |  Qt::ItemIsEnabled | Qt::ItemIsEditable);
   group(primitives);
   ++s_count;
   connect(newAction("Ungroup Fragment"), SIGNAL(triggered()), this, SLOT(ungroupAction()));
}


void Fragment::ungroupAction()
{
   MoleculeList parents(findLayers<Molecule>(Parents));
   if (!parents.isEmpty()) {
      parents.first()->ungroupSelection(); 
   }else {
      qDebug() << "No parent molecule found for ungrouping fragment";
   }
}


void Fragment::select()
{
   m_selected = true;
   PrimitiveList primitives(findLayers<Primitive>(Children));
   PrimitiveList::iterator primitive;

   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       (*primitive)->select();
   }
}


void Fragment::deselect()
{
   m_selected = false;
   PrimitiveList primitives(findLayers<Primitive>(Children));
   PrimitiveList::iterator primitive;
   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       (*primitive)->deselect();
   }
}


void Fragment::renumberAtomsFrom(int& index) 
{
   PrimitiveList primitives(findLayers<Primitive>(Children));
   PrimitiveList::iterator primitive;

   Atom* atom;
   Fragment* frag;

   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       if ( (frag = qobject_cast<Fragment*>(*primitive)) ) {
          frag->renumberAtomsFrom(index);
       }else if ( (atom = qobject_cast<Atom*>(*primitive)) ) {
          if (index == 0) {
             atom->setReorderIndex(0);
          }else {
             atom->setReorderIndex(index++);
          }
       }
   }
}


void Fragment::group(PrimitiveList const& primitives)
{
   Atom* atom;
   Bond* bond;
   Charge* charge;
   Fragment* fragment;

   PrimitiveList::const_iterator primitive;
   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       if ( (atom = qobject_cast<Atom*>(*primitive)) ) {
          if (!m_atomList.hasChildren())  appendRow(&m_atomList);
          m_atomList.appendRow(*primitive);
       
       }else if ( (bond = qobject_cast<Bond*>(*primitive)) ) {
          if (!m_bondList.hasChildren())  appendRow(&m_bondList);
          m_bondList.appendRow(*primitive);
   
       }else if ( (charge = qobject_cast<Charge*>(*primitive)) ) {
          if (!m_chargeList.hasChildren()) appendRow(&m_chargeList);
          m_chargeList.appendRow(*primitive);

       }else if ( (fragment = qobject_cast<Fragment*>(*primitive)) ) {
          appendRow(*primitive);
      
       } else {
          QMsgBox::warning(0, "IQmol", "Attempt to add unknown primitive type to fragment");
       }
       (*primitive)->setInGroup(true);
   }
}


PrimitiveList Fragment::ungroup() 
{
   Atom* atom;
   Bond* bond;
   Charge* charge;
   Fragment* fragment;

   PrimitiveList primitives(findLayers<Primitive>(Children));
   PrimitiveList::iterator primitive;

   for (primitive = primitives.begin(); primitive != primitives.end(); ++primitive) {
       if ( (atom = qobject_cast<Atom*>(*primitive)) ) {
          m_atomList.takeRow((*primitive)->row());
       
       }else if ( (bond = qobject_cast<Bond*>(*primitive)) ) {
          m_bondList.takeRow((*primitive)->row());
   
       }else if ( (charge = qobject_cast<Charge*>(*primitive)) ) {
          m_chargeList.takeRow((*primitive)->row());

       }else if ( (fragment = qobject_cast<Fragment*>(*primitive)) ) {
          takeRow((*primitive)->row());
      
       } else {
          QMsgBox::warning(0, "IQmol", "Attempt to add unknown primitive type to fragment");
       }
       (*primitive)->setInGroup(false);
   }

   qDebug() << "Fragment::ungroup returning primitive list of length" << primitives.size();
   return primitives;
}


void Fragment::draw(qglviewer::Vec const& cameraPosition)
{
   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   PrimitiveList primitives(findLayers<Primitive>(Children));

   PrimitiveList::iterator iter;
   for (iter = primitives.begin(); iter != primitives.end(); ++iter) {
       (*iter)->draw(cameraPosition);    
   }

   if (m_frame.grabsMouse()) {
      for (iter = primitives.begin(); iter != primitives.end(); ++iter) {
          (*iter)->drawSelected(cameraPosition);    
      }
   }
   glPopMatrix();
}


void Fragment::drawSelected(qglviewer::Vec const& cameraPosition)
{
   glPushMatrix();
   glMultMatrixd(m_frame.matrix());

   PrimitiveList primitives(findLayers<Primitive>(Children));

   PrimitiveList::iterator iter;
   for (iter = primitives.begin(); iter != primitives.end(); ++iter) {
       (*iter)->drawSelected(cameraPosition);    
   }

   glPopMatrix();
}

void Fragment::drawFast(qglviewer::Vec const& cameraPosition)
{
   draw(cameraPosition);
}


} } // end namespace IQmol::Layer
