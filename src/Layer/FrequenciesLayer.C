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

#include "FrequenciesLayer.h"
#include "MoleculeLayer.h"
#include "openbabel/generic.h"
#include "Animator.h"

#include "Frequencies.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {


Frequencies::Frequencies(Data::Frequencies const& frequencies) : Base("Frequencies"), 
   m_frequencies(frequencies), m_configurator(*this), m_play(false), m_loop(-1.0), 
   m_speed(0.0625), m_scale(0.25)
{
   fromData(m_frequencies);
   setConfigurator(&m_configurator);
}


void Frequencies::fromData(Data::Frequencies const& vibrationData) 
{        
   Data::VibrationalModeList::const_iterator iter;
   Data::VibrationalModeList const& modes(vibrationData.modes());
   double intensity, frequency;

   for (iter = modes.begin(); iter != modes.end(); ++iter) {
       intensity = (*iter)->intensity();
       frequency = (*iter)->frequency();
       Mode* mode = new Mode(frequency, (*iter)->eigenvector(), intensity);
       connect(mode, SIGNAL(playMode(Mode const&)), this, SLOT(playMode(Mode const&)));
       appendLayer(mode);
   }

   m_configurator.load();
}


void Frequencies::setMolecule(Molecule* molecule) 
{
   m_molecule = molecule;
   connect(this, SIGNAL(pushAnimators(AnimatorList const&)), 
      m_molecule, SIGNAL(pushAnimators(AnimatorList const&)));
   connect(this, SIGNAL(popAnimators(AnimatorList const&)), 
      m_molecule, SIGNAL(popAnimators(AnimatorList const&)));
   connect(this, SIGNAL(update()), m_molecule, SIGNAL(softUpdate()));
}


void Frequencies::configure()
{
   m_configurator.reset();
   m_configurator.display();
}


void Frequencies::setActiveMode(Mode const& mode)
{  
   clearActiveMode();

   if (!m_molecule) return;
   AtomList atoms(m_molecule->findLayers<Atom>(Children));
   if (atoms.size() != mode.nAtoms()) return;

   for (int i = 0; i < atoms.size(); ++i) {
       m_animatorList.append(
          new Animator::Vibration(atoms[i], mode.eigenvector(i), m_speed, m_scale, m_loop));
       atoms[i]->setDisplacement( mode.eigenvector(i) );
   }

   connect(m_animatorList.last(), SIGNAL(finished()), &m_configurator, SLOT(reset()));
}


void Frequencies::setPlay(bool const play) 
{
   m_play = play;

   if (m_play) {
      Atom::setDisplayVibrationVector(false);
      pushAnimators(m_animatorList);
   }else {
      popAnimators(m_animatorList);
      Atom::setDisplayVibrationVector(true);
      AnimatorList::iterator iter;
      for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
          (*iter)->reset();
      }
      update();
   }
}


void Frequencies::setLoop(bool const loop)
{
   m_loop = loop ? -1.0 : 1.0;
}


void Frequencies::playMode(Mode const& mode)
{
   setActiveMode(mode);
   setPlay(true);
}


void Frequencies::clearActiveMode()
{
   popAnimators(m_animatorList);
   AnimatorList::iterator iter;
   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       delete (*iter);
   }
   m_animatorList.clear();
   Atom::setDisplayVibrationVector(false);

   if (!m_molecule) return;
   AtomList atoms(m_molecule->findLayers<Atom>(Children));
   for (int i = 0; i < atoms.size(); ++i) {
       atoms[i]->setDisplacement( Vec(0.0, 0.0, 0.0) );
   }
}


void Frequencies::setSpeed(double const speed)
{
   m_speed = speed;

   AnimatorList::iterator iter;
   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       (*iter)->setSpeed(m_speed);
   }
}


void Frequencies::setScale(double const scale)
{
   m_scale = scale;
   Animator::Vibration* vibration;
   AnimatorList::iterator iter;

   for (iter = m_animatorList.begin(); iter != m_animatorList.end(); ++iter) {
       vibration = qobject_cast<Animator::Vibration*>(*iter);
       if (vibration) vibration->setScale(m_scale);
   }

   Atom::setVibrationVectorScale(4.0*scale);
   update();
}



// --------------- Mode ---------------

Mode::Mode(double const frequency, QList<qglviewer::Vec> const& eigenvectors, 
   double const intensity) : m_frequency(frequency), m_intensity(intensity),
   m_eigenvectors(eigenvectors)
{
   if (m_frequency < 0.0) {
      setText(QString("%1i").arg(-m_frequency, 8, 'f', 2));
    }else {
      setText(QString("%1 ").arg(m_frequency, 8, 'f', 2));
    }

    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}


Mode::Mode(double const frequency, Eigenvectors const& eigenvectors, double const intensity) 
 : m_frequency(frequency), m_intensity(intensity)
{
   double x, y, z;
   std::vector<OpenBabel::vector3>::const_iterator iter;
   for (iter = eigenvectors.begin(); iter != eigenvectors.end(); ++iter) {
       x = (*iter).x();
       y = (*iter).y();
       z = (*iter).z();
       m_eigenvectors.append( Vec(x,y,z) );
   }

   if (m_frequency < 0.0) {
      setText(QString("%1i").arg(-m_frequency, 8, 'f', 2));
    }else {
      setText(QString("%1 ").arg(m_frequency, 8, 'f', 2));
    }

    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}


Vec const& Mode::eigenvector(unsigned int atom) const
{
   static Vec zero(0.0, 0.0, 0.0);
   return ((int)atom > m_eigenvectors.size()-1) ? zero : m_eigenvectors[atom];
}

} } // end namespace IQmol::Layer
