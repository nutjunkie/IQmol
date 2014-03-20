#ifndef IQMOL_LAYER_FREQUENCIES_H
#define IQMOL_LAYER_FREQUENCIES_H
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

#include "Layer.h"
#include "Animator.h"
#include "FrequenciesConfigurator.h"
#include "QGLViewer/vec.h"
#include "openbabel/math/vector3.h"
#include <vector>


typedef std::vector<OpenBabel::vector3> Eigenvectors;

namespace OpenBabel {
   class OBVibrationData;
}

namespace IQmol {

namespace Data {
   class Frequencies;
}

namespace Layer {

   class Mode;

   /// Data Layer representing the vibrational frequencies of a molecule.
   /// The individual Modes are children of this item.
   class Frequencies : public Base {

      Q_OBJECT 

      public:
         Frequencies(Data::Frequencies const&);

         //void load(OpenBabel::OBVibrationData const&);
         void fromData(Data::Frequencies const&);

         void setMolecule(Molecule*);
         void setPlay(bool const play);
         void setLoop(bool const loop);
         void setPlay() { setPlay(m_play); }
         void setSpeed(double const speed);
         void setScale(double const scale);

      Q_SIGNALS:
         void pushAnimators(AnimatorList const&);
         void popAnimators(AnimatorList const&);
         void update();

      public Q_SLOTS:
         void configure();
         void setActiveMode(Mode const& mode);
         void playMode(Mode const& mode); 
         void clearActiveMode();

      private:
         Data::Frequencies const& m_frequencies;
         Configurator::Frequencies m_configurator;
         AnimatorList m_animatorList;
         bool m_play;
         double m_loop;  // -1.0 => loop forever
         double m_speed;
         double m_scale;
   };


   class Mode : public Base {
       Q_OBJECT

       public:
          Mode(double const frequency, Eigenvectors const& eigenvectors, 
             double const intensity);

          Mode(double const frequency, QList<qglviewer::Vec> const& eigenvectors, 
             double const intensity);
             
          double frequency() const { return m_frequency; } 
          double intensity() const { return m_intensity; } 
          int nAtoms() const { return m_eigenvectors.size(); }
          qglviewer::Vec const& eigenvector(unsigned int const atom) const;

       Q_SIGNALS:
          void playMode(Mode const&);

       public Q_SLOTS:
          void configure() { playMode(*this); }

       private:
          static QList<qglviewer::Vec> m_emptyList;
          double m_frequency;
          double m_intensity;
          QList<qglviewer::Vec>  m_eigenvectors;
   };


} } // end namespace IQmol::Layer

#endif
