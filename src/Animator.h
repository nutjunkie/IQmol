#ifndef IQMOL_ANIMATOR_H
#define IQMOL_ANIMATOR_H
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

#include "GLObjectLayer.h"
#include <QObject>
#include <QList>


namespace IQmol {
namespace Animator {

   enum Waveform { Square, Ramp, Sigmoidal, Triangle, Sinusoidal };

   /// Class controlling the animation of GLObjects from one frame to
   /// another.  Several different waveforms are available to provide 
   /// different effects (atually they all look the same, so that was 
   /// a waste of time).
   class Base : public QObject {

      Q_OBJECT

      public:
         Base(Layer::GLObject* object, double const cycles, double const speed, Waveform const);
         virtual ~Base() { }
         double getSpeed() const { return m_speed; }
         void setCycles(double const cycles) { m_cycles = cycles; }

      public Q_SLOTS:
         void setSpeed(double const speed) { m_speed = speed; }
         void setWavefunction(Waveform const);
         void step();
         virtual void reset();

      Q_SIGNALS:
         void finished();
         void finished(Animator::Base*);

      protected:
         Layer::GLObject* m_object;
         qglviewer::Vec m_originalPosition;
         virtual void update(double const time, double const amplitude) = 0;
         double m_time;
         bool   m_active;

      private:
         static double signum(double const t);
         static double square(double const t);
         static double ramp(double const t);
         static double sigmoidal(double const t);
         static double triangle(double const t);
         static double sinusoidal(double const t);

         double (*wavefunction)(double const);
         double m_cycles;  // < 0 => forever
         double m_speed;
   };


   class Vibration : public Base {

      Q_OBJECT

      public:
         Vibration(Layer::GLObject* object, qglviewer::Vec const& displacement, 
            double const speed, double const scale, double cycles = -1.0) 
              : Base(object, cycles, speed, Sinusoidal), 
            m_displacement(displacement), m_scale(scale) { }
         ~Vibration() { reset(); }
            
         void update(double const time, double const amplitude);
         void setScale(double const scale) { m_scale = scale; }
         double getScale() const { return m_scale; }

      private:
         qglviewer::Vec m_displacement;
         double m_scale;
   };


   class Path : public Base {

      Q_OBJECT

      public:
         Path(Layer::GLObject* object, QList<qglviewer::Vec> const& waypoints, 
           double const speed) : Base(object, -1.0, speed, Ramp), m_waypoints(waypoints), 
           m_bounce(false) { }
         ~Path() { reset(); }
           
         void update(double const time, double const amplitude);
         void addWaypoint(qglviewer::Vec const& displacement);
         void setBounceMode(bool bounce) { m_bounce = bounce; }
         bool getBounceMode() { return m_bounce; }

      private:
         QList<qglviewer::Vec> m_waypoints;
         bool m_bounce;
   };


   class Move : public Base {

      Q_OBJECT

      public:
         Move(Layer::GLObject* object, qglviewer::Vec const& endPoint, 
            double const speed = 0.05) : Base(object, 1.0, speed, Sigmoidal),
            m_endPoint(endPoint) { }
         void update(double const time, double const amplitude);

      public Q_SLOTS:
         void reset();

      private:
         qglviewer::Vec m_endPoint;
   };


   // Similar to Move, but transforms by Frames rather than just positions.
   // This is important for any object that has an orientation, such as Groups.
   class Transform : public Base {

      Q_OBJECT

      public:
         Transform(Layer::GLObject* object, qglviewer::Frame const& endPoint, 
            double const speed = 0.05);
         void update(double const time, double const amplitude);

      public Q_SLOTS:
         void reset();

      private:
         qglviewer::Frame m_startPoint;
         qglviewer::Frame m_endPoint;
   };

}

typedef QList<Animator::Base*> AnimatorList;

} // end namespace IQmol::Animator

#endif
