#ifndef IQMOL_UNDOCOMMANDS_H
#define IQMOL_UNDOCOMMANDS_H
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

#include "Animator.h"
#include "Layer.h"
#include "QGLViewer/vec.h"
#include "PrimitiveLayer.h"
#include <QUndoCommand>
#include <QString>
#include <QList>


class QStandardItem;

namespace qglviewer {
   class Vec;
}

namespace IQmol {

namespace Layer {
   class Atom;
   class Bond;
   class Molecule;
   class Constraint;
}

namespace Command {

   /// Allows Primitives to be added and/or removed from a Molecule in one Command
   class EditPrimitives : public QUndoCommand {
      public:
         EditPrimitives(QString const& text, Layer::Molecule* molecule)
          : QUndoCommand(text), m_molecule(molecule), m_deleteRemoved(true) { }

         template <class T>
         EditPrimitives& add(T const& added) 
         {
            typename T::const_iterator iter;
            for (iter = added.begin(); iter != added.end(); ++iter) {
                m_added.append(*iter);
            }
            return *this;
         }

         template <class T>
         EditPrimitives& remove(T const& removed) 
         {
            typename T::const_iterator iter;
            for (iter = removed.begin(); iter != removed.end(); ++iter) {
                m_removed.append(*iter);
            }
            return *this;
         }

         ~EditPrimitives();
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         Layer::PrimitiveList m_removed;
         Layer::PrimitiveList m_added;

		 // Determines which primitive list needs to be deleted when the
		 // Command goes out of scope:
         //   true  => delete removed Primitives
         //   false => delete added Primitives (set if undo is called);
         bool m_deleteRemoved;
   };


   class MoveObjects : public QUndoCommand {
      public:
         MoveObjects(Layer::Molecule*, QString const& text = "Move items", 
            bool const animiate = false);
         MoveObjects(GLObjectList const&, QString const& text = "Move items", 
            bool const animiate = false);
         ~MoveObjects();
           
         virtual void redo();
         virtual void undo();
         void setMessage(QString const& msg) { m_msg = msg; }

      protected:
         Layer::Molecule* m_molecule;

      private:
         void loadFrames(QList<qglviewer::Frame> const& frames);
         void saveFrames(QList<qglviewer::Frame>& frames);
         QList<qglviewer::Frame> m_initialFrames;
         QList<qglviewer::Frame> m_finalFrames;

         GLObjectList m_objectList;
         bool m_finalStateSaved;
         bool m_animate;
         QString m_msg;
         AnimatorList m_animatorList;
   };



   class AddHydrogens : public QUndoCommand {
      public:
         AddHydrogens(Layer::Molecule* molecule, Layer::PrimitiveList const& primitives);
         ~AddHydrogens();
            
         void redo();
         void undo();

      private:
         Layer::Molecule* m_molecule;
         Layer::PrimitiveList m_primitives;
         AnimatorList  m_animatorList;
   };


   // Specialized cases of the above

   class AddCharges: public EditPrimitives {
      public:
         AddCharges(Layer::Molecule* molecule, Layer::PrimitiveList const& chargeList)
            : EditPrimitives("Add Charges", molecule) {
            add(chargeList);
         }
   };


   class MinimizeStructure: public MoveObjects {
      public:
         MinimizeStructure(Layer::Molecule* molecule) 
            : MoveObjects(molecule, "Minimize energy", true) { }
   };
 

   class SymmetrizeStructure : public MoveObjects {
      public:
         SymmetrizeStructure(Layer::Molecule* molecule)
            : MoveObjects(molecule, "Symmetrize structure", true) { }
   };


   class AddConstraint : public QUndoCommand {

      public:
         AddConstraint(Layer::Molecule*, Layer::Constraint*);
         ~AddConstraint();

         void redo();
         void undo();

      private:
		 // Determines if the Constraint needs to be deleted when the command
         // goes out of scope.
         bool m_deleteConstraint;
         Layer::Molecule*   m_molecule;
         Layer::Constraint* m_constraint;
   };


   // Commands associated with changes to primitives

   class ChangeAtomType : public QUndoCommand {
      public:
         ChangeAtomType(Layer::Atom* bond);
         void redo();
         void undo();

      private:
         Layer::Atom* m_atom;
         bool m_finalStateSaved;
         int m_oldAtomicNumber;
         int m_newAtomicNumber;
         Layer::Molecule* m_molecule;
   };


   class ChangeBondOrder : public QUndoCommand {
      public:
         ChangeBondOrder(Layer::Bond* bond);
         void redo();
         void undo();

      private:
         Layer::Bond* m_bond;
         bool  m_finalStateSaved;
         int   m_oldOrder;
         int   m_newOrder;
         Layer::Molecule* m_molecule;
   };


   // Add Molecule

   class AddMolecule: public QUndoCommand {
      public:
         AddMolecule(Layer::Molecule* molecule, QStandardItem* parent);
         ~AddMolecule();
         void redo();
         void undo();
      private:
         Layer::Molecule* m_molecule;
         QStandardItem* m_parent;
         bool m_deleteMolecule;
   };


   class RemoveMolecule: public QUndoCommand {
      public:
         RemoveMolecule(Layer::Molecule* molecule, QStandardItem* parent);
         ~RemoveMolecule();
         void redo();
         void undo();
      private:
         Layer::Molecule* m_molecule;
         QStandardItem* m_parent;
         bool m_deleteMolecule;
   };


   class AppendData : public QUndoCommand {
      public:
         AppendData(Layer::Molecule* molecule, Layer::List const& dataList, 
            QString const& text) : QUndoCommand(text), m_molecule(molecule), 
            m_dataList(dataList) { }
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         Layer::List m_dataList;
   };


   class RemoveData : public QUndoCommand {
      public:
         RemoveData(Layer::Molecule* molecule, Layer::List const& dataList, 
            QString const& text) : QUndoCommand(text), m_molecule(molecule), 
            m_dataList(dataList) { }
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         Layer::List  m_dataList;
   };



/*
   // Groupig and ungrouping Fragments
   class GroupPrimitives : public QUndoCommand {
      public:
         GroupPrimitives(Layer::Molecule* molecule, PrimitiveList const& primitiveList)
           : QUndoCommand("Create Fragment"),  m_molecule(molecule), 
             m_primitiveList(primitiveList) { }
           
         ~GroupPrimitives() { }
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         PrimitiveList m_primitiveList;
         PrimitiveList m_fragment;
   };


   class UngroupFragments: public QUndoCommand {
      public:
         UngroupFragments(Layer::Molecule* molecule, FragmentList const& fragmentList)
           : QUndoCommand("Ungroup Fragment"),  m_molecule(molecule), 
             m_fragmentList(fragmentList) { }
           
         ~UngroupFragments() { }
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         FragmentList m_fragmentList;
   };
*/


} } // end namespace IQmol::Command

#endif
