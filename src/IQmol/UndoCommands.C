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

#include "UndoCommands.h"
#include "MoleculeLayer.h"
#include "AtomLayer.h"
#include "BondLayer.h"
#include "GroupLayer.h"
#include "QsLog.h"
#include "ConstraintLayer.h"
#include "QVariantPointer.h"


using namespace qglviewer;

namespace IQmol {
namespace Command {


// --------------- AddHydrogens ---------------
AddHydrogens::AddHydrogens(Layer::Molecule* molecule, Layer::PrimitiveList const& primitives)
  : QUndoCommand("Add hydrogens"), m_molecule(molecule), m_primitives(primitives)
{
   Layer::Atom *begin, *end, *atom;
   Layer::Bond *bond;
   Vec finalPosition;
   Vec bit(0.0, 0.0, 0.000001);  // avoids normalizing a null vector
   int index;

   AtomList hydrogens;
   BondList bonds;

   Layer::PrimitiveList::iterator iter;
   for (iter = m_primitives.begin(); iter != m_primitives.end(); ++iter) {
       if ( (bond = qobject_cast<Layer::Bond*>(*iter)) ) {
          bonds.append(bond);
       }else if  ( (atom = qobject_cast<Layer::Atom*>(*iter)) ) {
          hydrogens.append(atom);
       }
   }

   BondList::iterator bondIter;
   for (bondIter = bonds.begin(); bondIter != bonds.end(); ++bondIter) {
       begin = (*bondIter)->beginAtom();
       end   = (*bondIter)->endAtom();

       index = hydrogens.indexOf(begin);
       if (index >= 0) {
          finalPosition = hydrogens[index]->getPosition();
          hydrogens[index]->setPosition(end->getPosition() + bit);
          m_animatorList.append(new Animator::Move(hydrogens[index], finalPosition));
       }

       index = hydrogens.indexOf(end);
       if (index >= 0) {
          finalPosition = hydrogens[index]->getPosition();
          hydrogens[index]->setPosition(begin->getPosition() + bit);
          m_animatorList.append(new Animator::Move(hydrogens[index], finalPosition));
       }
   }
}


AddHydrogens::~AddHydrogens()
{
   if (m_molecule) m_molecule->popAnimators(m_animatorList); 
   AnimatorList::iterator iter;
   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       delete (*iter);
   }
}


void AddHydrogens::redo()
{
   AnimatorList::iterator iter;
   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       (*iter)->reset();
   }
   m_molecule->appendPrimitives(m_primitives);
   m_molecule->pushAnimators(m_animatorList); 
}


void AddHydrogens::undo()
{
   m_molecule->takePrimitives(m_primitives);
   m_molecule->updated();
}


// --------------- EditPrimitives ---------------
EditPrimitives::~EditPrimitives()
{
   Layer::PrimitiveList::iterator iter;
   if (m_deleteRemoved) {
      for (iter = m_removed.begin(); iter != m_removed.end(); ++iter) delete *iter;
   }else {
      for (iter = m_added.begin(); iter != m_added.end(); ++iter) delete *iter;
   }
}


void EditPrimitives::redo()
{
   m_deleteRemoved = true;
   m_molecule->takePrimitives(m_removed);
   m_molecule->appendPrimitives(m_added);
}


void EditPrimitives::undo()
{
   m_deleteRemoved = false;
   m_molecule->takePrimitives(m_added);
   m_molecule->appendPrimitives(m_removed);
   m_molecule->updated();
}



// --------------- MoveObjects ---------------
MoveObjects::MoveObjects(Layer::Molecule* molecule, QString const& text, bool const animate) 
   : QUndoCommand(text), m_molecule(molecule), m_finalStateSaved(false), m_animate(animate)
{ 
/*
   AtomList atomList(m_molecule->findLayers<Layer::Atom>(Layer::Children));
   AtomList::iterator atom;
   for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
       m_objectList.append(*atom);
   }

   GroupList groupList(m_molecule->findLayers<Layer::Group>(Layer::Children));
   GroupList::iterator group;
   for (group = groupList.begin(); group != groupList.end(); ++group) {
       m_objectList.append(*group);
   }
*/
   m_objectList = m_molecule->findLayers<Layer::GLObject>(Layer::Children);
   saveFrames(m_initialFrames);
}


MoveObjects::MoveObjects(GLObjectList const& objectList, QString const& text, 
   bool const animate) : QUndoCommand(text), m_molecule(0), m_objectList(objectList), 
   m_finalStateSaved(false), m_animate(animate)
{ 
   // Need a Molecule handle for the Viewer update (yugh)
   MoleculeList parents;
   int i(0);

   while (!m_molecule && i < m_objectList.size()) {
      parents = m_objectList[i]->findLayers<Layer::Molecule>(Layer::Parents);
      if (!parents.isEmpty()) m_molecule = parents.first();
      ++i;
   }

   if (!m_molecule) { QLOG_ERROR() << "MoveObjects constructor called with no molecule"; }
   saveFrames(m_initialFrames);
}


MoveObjects::~MoveObjects()
{
   if (m_molecule) m_molecule->popAnimators(m_animatorList); 
   AnimatorList::iterator iter;
   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       delete (*iter);
   }
}


void MoveObjects::redo() 
{
   if (!m_finalStateSaved) {
      saveFrames(m_finalFrames);
      m_finalStateSaved = true;
      if (m_animate) {   
         for (int i = 0; i < m_objectList.size(); ++i) {
             m_objectList[i]->setFrame(m_initialFrames[i]);
             m_animatorList.append(new Animator::Move(m_objectList[i],
                 m_finalFrames[i]));
         }
      }
   }

   if (m_animate && m_molecule) {
      AnimatorList::iterator iter;
      for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
          (*iter)->reset();
      }
      m_molecule->pushAnimators(m_animatorList); 
      m_molecule->setReferenceFrame(m_finalFrames.last());
   }else {
      loadFrames(m_finalFrames);
   }

   if (m_molecule) m_molecule->postMessage(m_msg);
}


void MoveObjects::undo() 
{
   loadFrames(m_initialFrames);
   if (m_molecule) m_molecule->postMessage("");
}


void MoveObjects::saveFrames(QList<Frame>& frames)
{
   frames.clear();
   for (int i = 0; i < m_objectList.size(); ++i) {
       frames.append(m_objectList[i]->getFrame());
   }
   frames.append(m_molecule->getReferenceFrame());
}


void MoveObjects::loadFrames(QList<Frame> const& frames)
{
   for (int i = 0; i < m_objectList.size(); ++i) {
       m_objectList[i]->setFrame(frames[i]);
   }
   m_molecule->setReferenceFrame(frames.last());
}



// --------------- ApplyConstraint ---------------
ApplyConstraint::ApplyConstraint(Layer::Molecule* molecule, Layer::Constraint* constraint)
   : MoveObjects(molecule, "Apply constraint", true), m_deleteConstraint(false), 
     m_constraint(constraint) 
{ 
}


ApplyConstraint::~ApplyConstraint()
{
   if (m_deleteConstraint) delete m_constraint;
}


void ApplyConstraint::undo()
{
   m_deleteConstraint = true;
   MoveObjects::undo();
}


void ApplyConstraint::redo()
{
   m_deleteConstraint = false;
   MoveObjects::redo();
}



// --------------- ChangeAtomType ---------------
ChangeAtomType::ChangeAtomType(Layer::Atom* atom) : QUndoCommand("Change atom type"),
   m_atom(atom), m_finalStateSaved(false), m_molecule(0)
{
   MoleculeList parents(atom->findLayers<Layer::Molecule>(Layer::Parents));
   //MoleculeList parents(atom->findParents<Layer::Molecule>());
   if (parents.size() != 1) {
      QLOG_ERROR() << "Could not determine parent Molecule";
   }else {
      m_molecule = parents.first();
   }
   m_oldAtomicNumber = m_atom->getAtomicNumber();
}


void ChangeAtomType::redo()
{
   if (m_finalStateSaved) {
      m_atom->setAtomicNumber(m_newAtomicNumber);
   }else {
      m_newAtomicNumber = m_atom->getAtomicNumber();
      m_finalStateSaved = true;
   }

   if (m_molecule) {
      m_molecule->updateInfo();
      m_molecule->softUpdate();
      m_molecule->reindexAtomsAndBonds();
   }
}


void ChangeAtomType::undo()
{
   m_atom->setAtomicNumber(m_oldAtomicNumber);
   if (m_molecule) {
      m_molecule->updateInfo();
      m_molecule->softUpdate();
      m_molecule->reindexAtomsAndBonds();
   }
}



// --------------- ChangeBondOrder ---------------
ChangeBondOrder::ChangeBondOrder(Layer::Bond* bond) : QUndoCommand("Change bond order"),
   m_bond(bond), m_finalStateSaved(false), m_molecule(0)
{
   // Need a Molecule handle for the Viewer update (yugh)
   MoleculeList parents(bond->findLayers<Layer::Molecule>(Layer::Parents));
   if (parents.size() != 1) {
      QLOG_ERROR() << "Could not determine parent molecule";
   }else {
      m_molecule = parents.first();
   }

   m_oldOrder = m_bond->getOrder();
}


void ChangeBondOrder::redo()
{
   if (m_finalStateSaved) {
      m_bond->setOrder(m_newOrder);
   }else {
      m_newOrder = m_bond->getOrder();
      m_finalStateSaved = true;
   }
   if (m_molecule) m_molecule->softUpdate();
}


void ChangeBondOrder::undo()
{
   m_bond->setOrder(m_oldOrder);
   if (m_molecule) m_molecule->softUpdate();
}



// --------------- AddMolecule ---------------
AddMolecule::AddMolecule(Layer::Molecule* molecule, QStandardItem* parent) 
   : m_molecule(molecule), m_parent(parent), m_deleteMolecule(false)
{ 
   QString s;
   if (m_molecule->fileName().isEmpty()) {
      s = "New molecule";
   }else {
      s = "Load file " + m_molecule->fileName();
   }
   setText(s);
}


AddMolecule::~AddMolecule()
{
   if (m_deleteMolecule) {
      QLOG_DEBUG() << "Deleting molecule" << m_molecule->text() << m_molecule;
      // The following causes a crash
      //delete m_molecule;  
   }
}


void AddMolecule::redo()
{
   m_deleteMolecule = false;
   QLOG_INFO() << "Adding molecule" << m_molecule->text() << m_molecule;
   m_parent->appendRow(m_molecule);
   m_molecule->updated();
}


void AddMolecule::undo()
{
   m_deleteMolecule = true;
   QLOG_INFO() << "Removing molecule" << m_molecule->text() << m_molecule;
   m_parent->takeRow(m_molecule->row());
   m_molecule->updated();
}


// --------------- RemoveMolecule ---------------
RemoveMolecule::RemoveMolecule(Layer::Molecule* molecule, QStandardItem* parent) 
   : m_molecule(molecule), m_parent(parent), m_deleteMolecule(false)
{ 
   QString s;
   if (m_molecule->fileName().isEmpty()) {
      s = "Remove molecule";
   }else {
      s = "Remove " + m_molecule->fileName();
   }
   setText(s);
}


RemoveMolecule::~RemoveMolecule()
{
   if (m_deleteMolecule) {
      QLOG_DEBUG() << "Deleting Molecule" << m_molecule->text() << m_molecule;
      // The following causes a crash
      //delete m_molecule;  
   }
}


void RemoveMolecule::redo()
{
   m_deleteMolecule = true;
   QLOG_INFO() << "Removing molecule" << m_molecule->text() << m_molecule;
   m_parent->takeRow(m_molecule->row());
   m_molecule->updated();
}


void RemoveMolecule::undo()
{
   m_deleteMolecule = false;
   QLOG_INFO() << "Adding molecule" << m_molecule->text() << m_molecule;
   m_parent->appendRow(m_molecule);
   m_molecule->updated();
}


// --------------- AppendData ---------------
void AppendData::redo()
{
   m_molecule->appendData(m_dataList);
   m_molecule->updated();
}

void AppendData::undo()
{
   m_molecule->removeData(m_dataList);
   m_molecule->updated();
}



// --------------- RemoveData ---------------
void RemoveData::redo()
{
   m_molecule->removeData(m_dataList);
   m_molecule->updated();
}

void RemoveData::undo()
{
   m_molecule->appendData(m_dataList);
   m_molecule->updated();
}




/*
// --------------- GroupPrimitives ---------------
void GroupPrimitives::redo()
{
qDebug() << "Group Primitives 0";
   m_molecule->takePrimitives(m_primitiveList);
qDebug() << "Group Primitives 1";
   m_fragment << new Layer::Fragment(m_primitiveList);
qDebug() << "Group Primitives 2";
   m_molecule->appendPrimitives(m_fragment);
qDebug() << "Group Primitives 3 - finished";
}


void GroupPrimitives::undo()
{
qDebug() << "ungroup Primitives 0";
   Layer::Fragment* fragment = qobject_cast<Layer::Fragment*>(m_fragment.first());
qDebug() << "ungroup Primitives 1";
   m_molecule->takePrimitives(m_fragment);
qDebug() << "ungroup Primitives 2";
   m_molecule->appendPrimitives(fragment->ungroup());
qDebug() << "ungroup Primitives 3";
   //delete fragment;
   m_fragment.clear();
qDebug() << "ungroup Primitives end";
}


// --------------- UnroupPrimitives ---------------
void UngroupFragments::redo()
{
   m_molecule->takePrimitives(m_fragmentList);
   // At this point the Molecule has given us a list of only the top most
   // fragments
   FragmentList::iterator iter;
   QStandardItem* parent
   for (iter = m_fragmentList.begin(); iter != fragment.end(); ++iter) {
       parent parent((*iter)->parent());
       
   }

   m_molecule->appendPrimitives(fragment->ungroup());
   m_molecule->appendPrimitives(m_fragment);
   delete fragment;
   m_fragment.clear();
}


void UngroupFragments::undo()
{
   Layer::Fragment* fragment = qobject_cast<Layer::Fragment*>(m_fragment.first());
   m_molecule->takePrimitives(m_fragment);
   m_fragment << new Layer::Fragment(m_primitiveList);
}

*/

} } // end namespace IQmol::Command

