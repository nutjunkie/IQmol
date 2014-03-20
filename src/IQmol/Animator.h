#ifndef IQMOL_ANIMATOR_H
#define IQMOL_ANIMATOR_H
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

#include "GLObjectLayer.h"
#include "SurfaceLayer.h"
#include "Geometry.h"
#include <QObject>
#include <QList>


namespace IQmol {
namespace Animator {

   enum Waveform { Square, Ramp, Sigmoidal, Triangle, Sinusoidal };

   /// Class controlling the animation of GLObjects from one frame to
   /// another.  Several different waveforms are available to provide 
   /// different effects (actually they all look the same, so that was 
   /// a waste of time).
   class Base : public QObject {

      Q_OBJECT

      public:
         // If cycles < 0.0 then it animates until stopped
         Base(double const cycles, double const speed, Waveform const waveform) : 
            m_waveform(waveform), m_cycles(cycles), m_speed(speed), m_active(true), 
            m_time(0.0) { }
         virtual ~Base() { }

         double getSpeed() const { return m_speed; }
         bool   isActive() const { return m_active; }

      public Q_SLOTS:
         void setCycles(double const cycles) { m_cycles = cycles; }
         void setSpeed(double const speed) { m_speed = speed; }
         void setWavefunction(Waveform const waveform) { m_waveform = waveform; }

         void step();
         virtual void reset();

      Q_SIGNALS:
         void finished();
         void finished(Animator::Base*);

      protected:
         virtual void update(double const time, double const amplitude) = 0;

      private:
         double amplitude(double const t);

         Waveform m_waveform;
         double   m_cycles;  // < 0 => forever
         double   m_speed;
         bool     m_active;
         double   m_time;
   };



   // ABC for animators that move (translate and/or rotate) an object
   class Movement : public Base {

      Q_OBJECT

      public:
         Movement(Layer::GLObject* object, double const cycles, double const speed, 
            Waveform const waveform) : Base(cycles, speed, waveform), m_object(object)
         {
            m_beginFrame = m_object->getFrame();
         }
         virtual ~Movement() { }

      public Q_SLOTS:
         virtual void reset() {
            Base::reset();
            m_object->setFrame(m_beginFrame);
         }

      protected:
         Layer::GLObject* m_object;
         qglviewer::Frame m_beginFrame;
   };



   class Vibration : public Movement {

      Q_OBJECT

      public:
         Vibration(Layer::GLObject* object, qglviewer::Vec const& displacement, 
            double const speed, double const scaleAmplitude, double cycles = -1.0) :
            Movement(object, cycles, speed, Sinusoidal), m_displacement(displacement), 
            m_scaleAmplitude(scaleAmplitude) { }
         ~Vibration() { reset(); }
            
         void update(double const time, double const amplitude);
         void setScale(double const scaleAmplitude) { m_scaleAmplitude = scaleAmplitude; }
         double getScale() const { return m_scaleAmplitude; }

      private:
         qglviewer::Vec m_displacement;
         double m_scaleAmplitude;
   };



   class Move : public Movement {

      Q_OBJECT

      public:
         Move(Layer::GLObject* object, qglviewer::Frame const& endFrame, 
            double const speed = 0.05);
         Move(Layer::GLObject* object, qglviewer::Vec const& endPoint, 
            double const speed = 0.05);

         void update(double const time, double const amplitude);

      private:
         qglviewer::Frame m_endFrame;
   };



   class Path : public Movement {

      Q_OBJECT

      public:
         Path(Layer::GLObject* object, QList<qglviewer::Vec> const& waypoints, 
           double const speed, bool const bounce = false);
           
         void update(double const time, double const amplitude);
         void setBounceMode(bool bounce) { m_bounce = bounce; }

      private:
         QList<qglviewer::Vec> m_waypoints;
         bool m_bounce;
   };



   // This works a little differently from the other animators.  We must first
   // generate a list of surfaces and this class is repsonsible for determining
   // which one needs to be visible at a given time.

   class Combo : public Base {

      Q_OBJECT

      public:
         class Data {
            friend class Combo;
            public:
               Data(IQmol::Data::Geometry const& geometry, Layer::Surface* surface) 
                : m_geometry(geometry), m_surface(surface) { }
            protected:
               IQmol::Data::Geometry m_geometry;
               Layer::Surface* m_surface;
         };

         typedef QList<Data*> DataList;

         Combo(Layer::Molecule*, DataList const& frames, int const interpolationFrames, 
            double const speed, bool const bounce = false);
         ~Combo();
           
         void update(double const time, double const amplitude);
         void setBounceMode(bool bounce) { m_bounce = bounce; }
         void stepForward();
         void stepBack();
         void reset();
         void setAlpha(double const);
         void setDrawMode(Layer::Surface::DrawMode const);

      private:
         void setCurrentIndex(int const n);
         Layer::Molecule* m_molecule;
         DataList m_frames;
         bool m_bounce;
         int  m_interpolationFrames;
         int  m_currentIndex;
   };



}

typedef QList<Animator::Base*> AnimatorList;

} // end namespace IQmol::Animator

#endif
