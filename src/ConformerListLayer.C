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

#include "ConformerListLayer.h"
#include "InfoLayer.h"
#include "IQmol.h"
#include "QsLog.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {


ConformerList::ConformerList(QList<Conformer*> const& conformers, Conformer* defaultConformer) 
  : Data("Geometries"), m_configurator(this), m_defaultConformer(defaultConformer), 
    m_speed(0.125), m_reperceiveBonds(false), m_nConformers(conformers.size()), m_bounce(false)
{
   QList<Layer::Conformer*>::const_iterator iter;
   for (iter = conformers.begin(); iter != conformers.end(); ++iter) {
       appendRow(*iter);
   }

   m_configurator.load();
   setConfigurator(&m_configurator);
}


void ConformerList::setMolecule(Molecule* molecule)
{
   QLOG_DEBUG() << "Setting ConformerList molecule to" << molecule;
   if (!molecule) return;
   m_molecule = molecule;

   connect(this, SIGNAL(pushAnimators(AnimatorList const&)),
      m_molecule, SIGNAL(pushAnimators(AnimatorList const&)));
   connect(this, SIGNAL(popAnimators(AnimatorList const&)),
      m_molecule, SIGNAL(popAnimators(AnimatorList const&)));
   connect(this, SIGNAL(update()), m_molecule, SIGNAL(softUpdate()));

   AtomList atomList(m_molecule->findLayers<Atom>(Children));
   QList<Conformer*> conformers(findLayers<Conformer>(Children));
   QLOG_DEBUG() << "Number of atoms and geometries" << atomList.size() << conformers.size();

   for (int i = 0; i < atomList.size(); ++i) {
       QList<Vec> waypoints;
       for (int j = 0; j < conformers.size(); ++j) {
           waypoints.append(conformers[j]->m_coordinates[i]);
       }
       m_animatorList.append(new Animator::Path(atomList[i], waypoints, m_speed)); 
   }

   //connect(m_animatorList.last(), SIGNAL(finished()), &m_configurator, SLOT(reset()));


   setDefaultConformer();
}


void ConformerList::setActiveConformer(Conformer const& conformer)
{
   if (!m_molecule) return;
   setPlay(false);
   AtomList atoms(m_molecule->findLayers<Atom>(Children));
   if (conformer.updateAtoms(atoms)) {
      m_molecule->energyAvailable(conformer.getEnergy(), Info::Hartree);
      m_molecule->dipoleAvailable(conformer.getDipole(), false);
      if (m_reperceiveBonds) {
         m_molecule->reperceiveBonds();
      }else {
         update(); 
      }
   }else {
      QLOG_ERROR() << "Setting conformer failed";
   }
}


void ConformerList::setDefaultConformer()
{ 
   if (m_defaultConformer) setActiveConformer(*m_defaultConformer); 
}


void ConformerList::setPlay(bool const play)
{
   QLOG_TRACE() << "Number of animators:" << m_animatorList.size();
   if (play) {
      pushAnimators(m_animatorList);
   }else {
      popAnimators(m_animatorList);
      AnimatorList::iterator iter;
      for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
          (*iter)->reset();
      }
      update();
   }
}


void ConformerList::setSpeed(double const speed)
{
   m_speed = speed;
   AnimatorList::iterator iter;
   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       (*iter)->setSpeed(m_speed);
   }
}


void ConformerList::setBounce(bool const bounce)
{
   m_bounce = bounce;
   AnimatorList::iterator iter;
   Animator::Path* pathAnimator;
   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       pathAnimator = qobject_cast<Animator::Path*>(*iter); 
       if (pathAnimator) pathAnimator->setBounceMode(bounce);
   }
}


void ConformerList::setLoop(bool const loop)
{
   AnimatorList::iterator iter;
   Animator::Path* pathAnimator;
   int cycles(loop ? -1.0 : m_nConformers-1);
   if (m_bounce) cycles *= 2;

   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       pathAnimator = qobject_cast<Animator::Path*>(*iter); 
       if (pathAnimator) pathAnimator->setCycles(cycles);
   }
}



void ConformerList::configure()
{
   m_configurator.reset();
   m_configurator.display();
}



// --------------- Conformer ---------------

Conformer::Conformer(AtomList const& atomList, double const energy)
{
   setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
   setEnergy(energy);
   AtomList::const_iterator atom;
   for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
       m_partialCharges.append((*atom)->getCharge());
       m_spinDensities.append((*atom)->getSpin()); 
       m_coordinates.append((*atom)->getPosition());
   }
}


Conformer::Conformer(QList<Vec> const& coordinates, double const energy) 
  : m_coordinates(coordinates)
{
   setEnergy(energy);
   setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

   for (int i = 0; i < m_coordinates.size(); ++i) {
       m_partialCharges.append(0.0);
       m_spinDensities.append(0.0);
   }
}


void Conformer::setCharges(QList<double> const& partialCharges,
   QList<double> const& spinDensities)
{
   if (m_coordinates.size() == partialCharges.size()) {
      m_partialCharges = partialCharges;
   }

   if (m_coordinates.size() == spinDensities.size()) {
      m_spinDensities = spinDensities;
   }
}


void Conformer::setEnergy(double const energy)
{ 
   m_energy = energy; 
   setText(QString::number(m_energy,'f', 6));
}


bool Conformer::updateAtoms(AtomList& atomList) const
{
   if (atomList.size() != m_coordinates.size()) {
      QLOG_DEBUG() << "Atom list size and coordinates don't match" 
                   << atomList.size() << "!=" << m_coordinates.size();
      return false;
   }

   for (int i = 0; i < atomList.size(); ++i) {
       atomList[i]->setPosition(m_coordinates[i]);
       atomList[i]->setCharge(m_partialCharges[i]);
       atomList[i]->setSpin(m_spinDensities[i]);
   }
   return true;
}


void Conformer::setMultipoles(QList<double> const& multipoles)
{
   int size(multipoles.size());

   if (size >= 1) {
      m_charge = multipoles[0];
   }

   if (size >= 3) {
      m_dipole = Vec(multipoles[1], multipoles[2], multipoles[3]);
   }
}


//! Computes the charge, dipole and quadrupole from the partial charges
void Conformer::computeMoments()
{
      
}

} } // end namespace IQmol::Layer
