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

#include "Layer.h"
#include "QsLog.h"


namespace IQmol {
namespace Layer {

Base::Base(QString const& text, QObject* parent) : QObject(parent), 
   QStandardItem(text), m_molecule(0), m_configurator(0), m_persistentParent(0), 
   m_propertyFlags(0)
{ 
   setFlags(Qt::ItemIsEnabled);
   setData(QVariantPointer<Base>::toQVariant(this));
}


Base::~Base() 
{ 
   deleted();
   QList<QAction*>::iterator iter;
   for (iter = m_actions.begin(); iter != m_actions.end(); ++iter) {
       delete *iter;
   } 
}


void Base::setMolecule(Molecule* molecule)
{
   m_molecule = molecule;
   QList<Base*> children(findLayers<Base>());
   QList<Base*>::iterator iter;
   for (iter = children.begin(); iter != children.end(); ++iter) {
       (*iter)->setMolecule(molecule);
   }
}


void Base::appendLayer(Base* child)
{
   child->setPersistentParent(this);
   if (!hasChildren() && (m_propertyFlags & RemoveWhenChildless)) adopt();
   appendRow(child);
}


void Base::removeLayer(Base* child)
{
   child->orphan();
   if (!hasChildren() && (m_propertyFlags & RemoveWhenChildless)) orphan();
}


void Base::orphanLayer()
{
   if (m_persistentParent) {
      m_persistentParent->removeLayer(this);
      m_persistentParent = 0;
   }
}


QAction* Base::newAction(QString const& text) 
{ 
   QAction* action(new QAction(text, this));
   m_actions.append(action);
   return action;
} 


void Base::setPersistentParent(Base* parent) 
{ 
   if (m_persistentParent == parent) return;

   if (m_persistentParent) {
      disconnect(m_persistentParent, SIGNAL(deleted()),
      this, SLOT(persistentParentDeleted()));
   }

   m_persistentParent = parent; 

   if (m_persistentParent) {
      connect(m_persistentParent, SIGNAL(deleted()),
      this, SLOT(persistentParentDeleted()));
   }
}


void Base::orphan()
{
   QStandardItem* p(QStandardItem::parent());
   if (p) {
      for (int i = 0; i < p->rowCount(); ++i) {
          if (this == p->child(i) ) {
             p->takeRow(i);
             orphaned();
             return;
          }
      }
   }
   QLOG_DEBUG() << "Failed to find parent for" << text() ;
}


void Base::adopt()
{
   QStandardItem* p(QStandardItem::parent());
   if (p) {
      QLOG_DEBUG() << "Attempt to adopt Layer" << text() << "with parent" << p->text();
   }else {
      if (m_persistentParent) {
         m_persistentParent->appendLayer(this);
         adopted();
      }else {
         QLOG_DEBUG() << "Layer " << text() << "has no persistent parent";
      }
   }
}

} } // end namespace IQmol::Layer
