#ifndef IQMOL_UNDOCOMMANDS_H
#define IQMOL_UNDOCOMMANDS_H
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

#include "Animator.h"
#include "DataLayer.h"
#include "FragmentLayer.h"
#include "QGLViewer/vec.h"
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
}

namespace Command {

   /// Allows Primitives to be added and/or removed from a Molecule in one Command
   class EditPrimitives : public QUndoCommand {
      public:
         EditPrimitives(QString const& text, Layer::Molecule* molecule, 
            PrimitiveList const& removed, PrimitiveList const& added)
          : QUndoCommand(text), m_molecule(molecule), m_removed(removed),
            m_added(added), m_deleteRemoved(true) { }
            
         ~EditPrimitives();
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         PrimitiveList m_removed;
         PrimitiveList m_added;

		 // Determines which primitive list needs to be deleted when the
		 // Command goes out of scope:
         //   true  => delete removed Primitives
         //   false => delete added Primitives (set if undo is called);
         bool m_deleteRemoved;
   };


   // !!! DEPRECATE
   /// Adds a list of primitves to a Molecule.  If the last call was to undo
   /// when the Command goes out of scope then the primtives are deleted.
   class AddPrimitives : public QUndoCommand {
      public:
         AddPrimitives(Layer::Molecule* molecule, PrimitiveList const& primitiveList, 
            QString const& text = "Add atoms/bonds") : QUndoCommand(text), 
            m_molecule(molecule), m_primitiveList(primitiveList), m_deletePrimitives(false) { }
         ~AddPrimitives();
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         PrimitiveList m_primitiveList;
         bool m_deletePrimitives;
   };


   // !!! DEPRECATE
   /// Removes a list of primitives from a single Molecule.  The destruction of
   /// the Primitives is delayed until the Command is destroyed.
   class RemovePrimitives : public QUndoCommand {
      public:
         RemovePrimitives(Layer::Molecule* molecule, PrimitiveList const& primitiveList,
            QString const& text = "Remove atoms/bonds") : QUndoCommand(text), 
            m_molecule(molecule), m_primitiveList(primitiveList), m_deletePrimitives(false) { }
            
         ~RemovePrimitives();
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         PrimitiveList m_primitiveList;
         bool m_deletePrimitives;
   };


   // Note that in the following class the Molecule object is simply a handle
   // for updating the Viewer.  The GLObjectList can contain objects from other
   // Molecules as well.
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

      private:
         void loadCoordinates(QList<qglviewer::Vec> const& coordinates);
         void saveCoordinates(QList<qglviewer::Vec>& coordinates);
         QList<qglviewer::Vec> m_initialCoordinates;
         QList<qglviewer::Vec> m_finalCoordinates;
         Layer::Molecule* m_molecule;
         GLObjectList m_objectList;
         bool m_finalStateSaved;
         bool m_animate;
         QString m_msg;
         AnimatorList m_animatorList;
   };


   class AddHydrogens : public QUndoCommand {
      public:
         AddHydrogens(Layer::Molecule* molecule, PrimitiveList const& primitives);
         ~AddHydrogens();
            
         void redo();
         void undo();

      private:
         Layer::Molecule* m_molecule;
         PrimitiveList m_primitives;
         AnimatorList  m_animatorList;
   };


   // Specialized cases of the above

   class AddCharges: public AddPrimitives {
      public:
         AddCharges(Layer::Molecule* molecule, PrimitiveList const& chargeList)
            : AddPrimitives(molecule, chargeList, "Add charges") { }
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


   class ApplyConstraint : public MoveObjects {
      public:
         ApplyConstraint(Layer::Molecule* molecule)
            : MoveObjects(molecule, "Apply constraint", true) { }
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
         AppendData(Layer::Molecule* molecule, DataList const& dataList, 
            QString const& text) : QUndoCommand(text), m_molecule(molecule), 
            m_dataList(dataList) { }
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         DataList  m_dataList;
   };


   class RemoveData : public QUndoCommand {
      public:
         RemoveData(Layer::Molecule* molecule, DataList const& dataList, 
            QString const& text) : QUndoCommand(text), m_molecule(molecule), 
            m_dataList(dataList) { }
            
         virtual void redo();
         virtual void undo();

      private:
         Layer::Molecule* m_molecule;
         DataList  m_dataList;
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
