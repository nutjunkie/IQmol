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
#include "QsLog.h"
#define _USE_MATH_DEFINES
#include <cmath>


using namespace qglviewer;

namespace IQmol {
namespace Animator {

// if cycles < 0.0 then it animates until manually stopped
Base::Base(Layer::GLObject* object, double const cycles, double const speed, 
   Waveform const waveform) : m_object(object), m_cycles(cycles), m_speed(speed),
   m_time(0.0), m_active(true)
{
   m_originalPosition = m_object->getPosition();
   setWavefunction(waveform);
}


void Base::setWavefunction(Waveform const waveform)
{
   switch (waveform) {
      case Square:      wavefunction = &square;      break;
      case Ramp:        wavefunction = &ramp;        break;
      case Sigmoidal:   wavefunction = &sigmoidal;   break;
      case Triangle:    wavefunction = &triangle;    break;
      case Sinusoidal:  wavefunction = &sinusoidal;  break;
   }
}


void Base::step()
{
   if (!m_active) return;
   m_time += m_speed;

   if (m_cycles > 0.0 && m_time > m_cycles) {
      m_time = m_cycles;
      m_active = false;
   }

   update(m_time, wavefunction(m_time));

   if (!m_active) {
       finished(this);
       finished();
   }
}


void Base::reset()
{
   QLOG_TRACE() << "Animator: Base::reset() called, setting to original position";
   m_time = 0;
   m_active = true;
   m_object->setPosition(m_originalPosition);
}


double Base::signum(double const t)
{
   return (t > 0.0) ? 1.0 : -1.0; 
}


double Base::square(double const t)
{
   return signum(std::sin(2.0*M_PI*t));
}


double Base::ramp(double const t)
{
   return t - std::floor(t);
}


double Base::sigmoidal(double const t)
{
   double s(std::sin(0.5*M_PI*t));
   return s*s;
}



double Base::triangle(double const t)
{
   return std::abs(2.0 - std::fmod(t, 4.0)) - 1.0;
}


double Base::sinusoidal(double const t)
{
   return std::sin(2.0*M_PI*t);
}



// --------------- Vibration ---------------
void Vibration::update(double const time, double const amplitude)
{
   Q_UNUSED(time);
   m_object->setPosition(m_originalPosition + m_scale*amplitude*m_displacement);
}


// --------------- Path ---------------
void Path::update(double const time, double const amplitude)
{
   int n(m_waypoints.size()-1);
   int index = m_bounce ? (int(time) % (2*n)) : (int(time) % n);

   if (index >= n) {
      index = 2*n - index - 1;
      Vec d(m_waypoints[index]-m_waypoints[index+1]);
      m_object->setPosition(m_waypoints[index+1]+amplitude*d);
   }else {
      Vec d(m_waypoints[index+1]-m_waypoints[index]);
      m_object->setPosition(m_waypoints[index]+amplitude*d);
   }
}


// --------------- Move ---------------
void Move::reset()
{
   QLOG_DEBUG() << "Animator: Move::reset() called";
   //m_object->setPosition(m_endPoint);
}


void Move::update(double const time, double const amp)
{
   Q_UNUSED(time);
   m_object->setPosition( (1.0-amp)*m_originalPosition + amp*m_endPoint);
}

} } // end namespace IQmol::Animator
