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

#include "Animator.h"
#include "MoleculeLayer.h"
#define _USE_MATH_DEFINES
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace Animator {


double Base::amplitude(double const t)
{
   double amp(0.0);
   switch (m_waveform) {
      case Square:      amp = (std::sin(2.0*M_PI*t)>0) ? 1.0 : -1.0;  break;
      case Ramp:        amp = t - std::floor(t);                      break; 
      case Sigmoidal:   amp = std::pow(std::sin(0.5*M_PI*t), 2);      break;
      case Triangle:    amp = std::abs(2.0-std::fmod(t, 4.0)) - 1.0;  break;
      case Sinusoidal:  amp = std::sin(2.0*M_PI*t);                   break;
   }
   return amp;
}


void Base::step()
{
   if (!m_active) return;
   m_time += m_speed;

   if (m_cycles > 0.0 && m_time > m_cycles) {
      m_time = m_cycles;
      m_active = false;
   }

   update(m_time, amplitude(m_time));

   if (!m_active) {
       finished(this);
       finished();
   }
}


void Base::reset()
{
   m_time = 0;
   m_active = true;
}



// --------------- Vibration ---------------

void Vibration::update(double const time, double const amplitude)
{
   Q_UNUSED(time);
   m_object->setPosition(m_beginFrame.position() + m_scaleAmplitude*amplitude*m_displacement);
}



// --------------- Move ---------------

Move::Move(Layer::GLObject* object, Frame const& endFrame, double const speed) :    
   Movement(object, 1.0, speed, Sigmoidal), m_endFrame(endFrame) 
{ 
}
            

Move::Move(Layer::GLObject* object, Vec const& endPoint, double const speed) : 
   Movement(object, 1.0, speed, Sigmoidal) 
{
   m_endFrame.setPosition(endPoint);
}
            

void Move::update(double const time, double const amp)
{
   Q_UNUSED(time);
   m_object->setPosition( (1.0-amp)*m_beginFrame.position() + amp*m_endFrame.position());
   m_object->setOrientation( Quaternion::slerp(m_beginFrame.orientation(),
       m_endFrame.orientation(),amp) );
}



// --------------- Path ---------------
Path::Path(Layer::GLObject* object, QList<Vec> const& waypoints, double const speed, 
   bool const bounce) : Movement(object, 1.0, speed, Ramp), m_waypoints(waypoints),  
   m_bounce(bounce)
{ 
}


void Path::update(double const time, double const amplitude)
{

  int nIntervals(m_waypoints.size()-1);

   if (m_bounce) {

      int interval = (int(time) % (2*nIntervals));
      if (interval >= nIntervals) {
         interval = 2*nIntervals - interval- 1;
         Vec d(m_waypoints[interval]-m_waypoints[interval+1]);
         m_object->setPosition(m_waypoints[interval+1]+amplitude*d);
      }else {
         Vec d(m_waypoints[interval+1]-m_waypoints[interval]);
         m_object->setPosition(m_waypoints[interval]+amplitude*d);
      }

   }else {

      int interval = (int(time) % (nIntervals+1));
      if (interval == nIntervals ) {
         m_object->setPosition(m_waypoints[interval]);
      }else {
         Vec d(m_waypoints[interval+1]-m_waypoints[interval]);
         m_object->setPosition(m_waypoints[interval]+amplitude*d);
      }

   }
}



// --------------- Combo ---------------

Combo::Combo(Layer::Molecule* molecule, DataList const& frames, int const interpolationFrames, 
   double const speed) : Base(1.0, speed, Ramp), m_molecule(molecule), m_frames(frames), 
   m_bounce(false), m_loop(false), m_interpolationFrames(interpolationFrames), 
   m_currentIndex(-1)
{ 
   m_referenceFrames = 1 + (frames.size()-1)/(m_interpolationFrames+1);
   int nCycles(m_bounce ? 2*m_referenceFrames-1 : m_referenceFrames-1);
   setCycles(nCycles);

   DataList::iterator iter;
   for (iter = m_frames.begin(); iter != m_frames.end(); ++iter) {
       (*iter)->m_surface->setCheckState(Qt::Unchecked);
   }

   setCurrentIndex(0);
}


Combo::~Combo()
{
   reset();
   DataList::iterator iter;
   for (iter = m_frames.begin(); iter != m_frames.end(); ++iter) {
       delete (*iter);
   }
}


void Combo::reset()
{
   Base::reset();
}


void Combo::setCurrentIndex(int const n)
{
   if (n < 0 || n >= m_frames.size() || n == m_currentIndex) return;
   m_frames[n]->m_surface->setCheckState(Qt::Checked);
   m_molecule->setGeometry(m_frames[n]->m_geometry);

   if (m_currentIndex >= 0 && m_currentIndex < m_frames.size()) {
      m_frames[m_currentIndex]->m_surface->setCheckState(Qt::Unchecked);
   }
   m_currentIndex = n;
}
           

void Combo::setLoopMode(bool loop)
{
   m_loop = loop;
   int nCycles(-1);
   if (!m_loop) nCycles = m_bounce ? 2*m_referenceFrames-1 : m_referenceFrames-1;
   setCycles(nCycles);
}


void Combo::setBounceMode(bool bounce) 
{ 
   m_bounce = bounce; 
   int nCycles(-1);
   if (!m_loop) nCycles = m_bounce ? 2*m_referenceFrames-1 : m_referenceFrames-1;
   setCycles(nCycles);
}


void Combo::stepForward()
{
   setCurrentIndex(m_currentIndex+1);
}


void Combo::stepBack()
{
   setCurrentIndex(m_currentIndex-1);
}


void Combo::update(double const time, double const amplitude)
{
   Q_UNUSED(amplitude);

   int n(m_frames.size());
   int index(time*(m_interpolationFrames+1));
   index = m_bounce ? (index % (2*n)) : (index % n);
   if (index >= n) index = 2*n - index - 1;

   setCurrentIndex(index);
}


void Combo::setAlpha(double const alpha)
{
   DataList::iterator iter;
   for (iter = m_frames.begin(); iter != m_frames.end(); ++iter) {
       (*iter)->m_surface->setAlpha(alpha);
   }
}


void Combo::setDrawMode(Layer::Surface::DrawMode const mode)
{
   DataList::iterator iter;
   for (iter = m_frames.begin(); iter != m_frames.end(); ++iter) {
       (*iter)->m_surface->setDrawMode(mode);
   }
}

} } // end namespace IQmol::Animator
