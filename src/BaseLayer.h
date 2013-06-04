#ifndef IQMOL_BASELAYER_H
#define IQMOL_BASELAYER_H
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

#include "BaseConfigurator.h"
#include "QVariantPointer.h"
#include "QsLog.h"
#include <QStandardItem>
#include <QObject>
#include <QString>
#include <QAction>


namespace IQmol {
namespace Layer {

   /// Custom Layer property flags.  Do not confuse these with the flags for
   /// QStandardItem.
   enum PropertyFlag { 
      RemoveWhenChildless = 0x001,
      Selected            = 0x002 
   };


   /// Flags that control the behavior of the findLayers() member function.
   enum FindFlag { 
      Visible      = 0x001, 
      Nested       = 0x002, 
      Children     = 0x004, 
      Parents      = 0x008, 
      IncludeSelf  = 0x010,
      SelectedOnly = 0x020
   };

      

   /// Model item for the ViewerModel class.  
   /// Layers can be thought of as nodes of the tree which is represented in
   /// the 'Model View' window.  This allows a heirarchical data structure to be
   /// built up where, for example, Atom Layers are children of an AtomList Layer
   /// which is in turn a child of a Molecule Layer.  The visibility of a Layer,
   /// and all its children, is controlled by the checkbox that can be added with
   /// the Qt::ItemIsUserCheckable flag.
   ///
   /// Layers can also have an associated Configurator dialog which is activated 
   /// by double-clicking the Layer in the ModelView window.  This allows custom
   /// options to be set for each Layer.
   class Base : public QObject, public QStandardItem {

      Q_OBJECT

      public:

         explicit Base(QString const& text = QString(), QObject* parent = 0)
          : QObject(parent), QStandardItem(text), m_configurator(0), m_persistentParent(0),
            m_propertyFlags(0)
         { 
            setFlags(Qt::ItemIsEnabled);
            setData(QVariantPointer<Base>::toQVariant(this));
         }

         virtual ~Base() 
         { 
            deleted();
            QList<QAction*>::iterator iter;
            for (iter = m_actions.begin(); iter != m_actions.end(); ++iter) {
                delete *iter;
            } 
         }

		 /// Allows custom property flags to be set for the Layer.
         void setProperty(PropertyFlag const flag)   { m_propertyFlags |= flag; }
         void unsetProperty(PropertyFlag const flag) { m_propertyFlags &= ~flag; }
         bool hasProperty(PropertyFlag const flag) const { return m_propertyFlags & flag; }


		 /// Appends a child Layer to the current Layer.  If the current Layer
		 /// is childless and the RemoveWhenChildless flag is set, then it will
		 /// be reparented automatically.  This function should be used in
		 /// preference to QStandardItem::appendRow()
         virtual void appendLayer(Base* child)
         {
            child->setPersistentParent(this);
            if (!hasChildren() && (m_propertyFlags & RemoveWhenChildless)) adopt();
            appendRow(child);
         }

		 /// Removes a child Layer from this, if it exists.  Note that deletion
		 /// of the Layer must be handled explicitly.
         virtual void removeLayer(Base* child)
         {
            child->orphan();
            if (!hasChildren() && (m_propertyFlags & RemoveWhenChildless)) orphan();
         }

         virtual void orphanLayer()
         {
            if (m_persistentParent) {
               m_persistentParent->removeLayer(this);
               m_persistentParent = 0;
            }
         }


         template <class T>
         QList<T*> findLayers(unsigned int flags = (Nested | Children))
         {
            bool appendSelf(flags & IncludeSelf);
            
            if (flags & SelectedOnly) {
               appendSelf = appendSelf && hasProperty(Selected);
            }
            if (flags & Visible && isCheckable()) {
               appendSelf = appendSelf && (checkState() == Qt::Checked);
            }

            T* t;
            QList<T*> hits;    
            if ( appendSelf && (t = dynamic_cast<T*>(this)) ) { 
               hits.append(t);
               if (!(flags & Nested)) return hits;
            }

			// We can't go both ways else we'd end up with the whole family around
            if (flags & Children) {
               findChildren<T>(hits, flags);
            }else if (flags & Parents)  {
               findParents<T>(hits, flags);
            }
            
            return hits;
         }



		 /// Returns a list of actions that should appear in the context menu.
         QList<QAction*> getActions() 
         {
            return m_actions; 
         }


         /// Creates a new action for the context menu, ensuring it gets added
         /// to the m_actions list.
         QAction* newAction(QString const& text) 
         { 
            QAction* action(new QAction(text, this));
            m_actions.append(action);
            return action;
         } 


      Q_SIGNALS:
         /// Signals sent when this Layer has changed.
         void updated(); 

         /// Signals sent just before this Layer is about to be deleted.
         void deleted();

         /// Signals sent when this Layer has been removed from its parent. 
         void orphaned();

         /// Signals sent when this Layer has been added to its parent. 
         void adopted();


      public Q_SLOTS:
         virtual void configure() { 
            if (m_configurator) m_configurator->display();
         }

         virtual void closeConfigurator() {
            if (m_configurator) m_configurator->close();
         }

		 /// The persistent parent exists even if the Layer has been removed
		 /// from the model.  This is useful for undo actions that need to know
		 /// where the Layer was so that it can be put back if redo is called.
         /// In most cases this does not need to be explicity called as it is 
         /// called by appendLayer, but if the RemoveWhenChildless flag is set
         /// on a Layer that hasn't been appended yet, it will need to be called.
         void setPersistentParent(Base* parent) 
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


      protected:
         void setConfigurator(Configurator::Base* configurator) {
            m_configurator = configurator; 
         }


      private Q_SLOTS:
         void persistentParentDeleted() { m_persistentParent = 0; }


      private:
		 /// Removes this Layer from the Model but does not delete it.  The
		 /// Layer can linger (in a QUndoAction for example) and later be 
		 /// added back to the Model if required using adopt.
         void orphan()
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


		 /// Returns this Layer to the model after being orphaned.  This should 
		 /// only be called for Layers that have had a persistent parent set.
         void adopt()
         {
            QStandardItem* p(QStandardItem::parent());
            if (p) {
               QLOG_DEBUG() << "Attempt to adopt Layer" << text() 
                           << "with exisiting parent" << p->text();
            }else {
               if (m_persistentParent) {
                  m_persistentParent->appendLayer(this);
                  adopted();
               }else {
                  QLOG_DEBUG() << "Layer " << text() << "has no persistent parent";
               }
            }
         }


         /// Returns a list of the child Layers of a given type, excluding itself.
         template <class T>
         void findChildren(QList<T*>& children, unsigned int flags)
         {
            if (flags & Visible && isCheckable() && checkState() == Qt::Unchecked) return;

            for (int i = 0; i < rowCount(); ++i) {
                Base* ptr(QVariantPointer<Base>::toPointer(child(i)->data()));

                bool append(true);
                if (flags & Visible && ptr->isCheckable()) {
                   append = append && (ptr->checkState() == Qt::Checked);
                }
 
                if (append) {
                   if (flags & SelectedOnly) append = append && ptr->hasProperty(Selected);
                   T* t;
                   if (append && (t = dynamic_cast<T*>(ptr)) ) {
                      children.append(t);
                      if (flags & Nested) ptr->findChildren<T>(children, flags);
                   }else {
                      ptr->findChildren<T>(children, flags);
                   }
                }
            }
         }


         /// Returns a list of the parent Layers of a given type, excluding itself.
         template <class T>
         void findParents(QList<T*>& parents, unsigned int flags) 
         {
            QStandardItem* p(QStandardItem::parent());
            if (!p) return;

            Base* ptr(QVariantPointer<Base>::toPointer(p->data()));
            if (!ptr) return;

            bool append(true);
            if (flags & SelectedOnly) {
               append = append && ptr->hasProperty(Selected);
            }
            if (flags & Visible && ptr->isCheckable()) {
               append = append && (ptr->checkState() == Qt::Checked);
            }
            T* t;
            if (append && (t = dynamic_cast<T*>(ptr))) {
               parents.append(t);
               if (!(flags & Nested)) return;
            }

            ptr->findParents<T>(parents, flags);
         }

         // Data members
         Configurator::Base* m_configurator;
         QList<QAction*> m_actions;
         Base* m_persistentParent;
         unsigned int m_propertyFlags;
   };


} } // end namespace IQmol::Layer

#endif
