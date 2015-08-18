#ifndef IQMOL_DATA_ELECTRONICTRANSITION_H
#define IQMOL_DATA_ELECTRONICTRANSITION_H
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

#include "Spin.h"
#include "DataList.h"
#include "QGLViewer/vec.h"


namespace IQmol {
namespace Data {

   class Amplitude;

   /// Data class representing molecule with a particular geometry.  
   class ElectronicTransition : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::ElectronicTransition; }

         enum Multiplicity { Singlet, Doublet, Triplet, Quartet };

         ElectronicTransition(double const energy = 0.0, double const strength = 0.0,
            qglviewer::Vec const transitionMoment = qglviewer::Vec(), 
            double spinSquared = 0.0) : m_energy(energy), m_strength(strength), 
            m_transitionMoment(transitionMoment), m_spinSquared(spinSquared) { }
         
         double energy()   const { return m_energy; }
         double strength() const { return m_strength; }
         double spinSquared() const { return m_spinSquared; }
         qglviewer::Vec const& transitionMoment() { return m_transitionMoment; }

         bool addAmplitude(QStringList const&);
         QList<Amplitude>& amplitudes() { return m_amplitudes; }

/*
         bool operator<(ElectronicTransition const& that) {
            return (m_energy < that.m_energy);
         }
*/

         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & m_energy;
            ar & m_strength;
            ar & m_spinSquared;
            ar & m_transitionMoment;
            ar & m_amplitudes;
         }

         double m_energy;
         double m_strength;
         double m_spinSquared;
         qglviewer::Vec m_transitionMoment;
         QList<Amplitude> m_amplitudes;
         
        // Mulliken charges or something to show the movement of the electron
   };



   class Amplitude : public Base {

      friend class boost::serialization::access;
      friend class ExcitedStates;

      public: 
          Amplitude(Spin spin = Alpha, unsigned const i = 0, unsigned const a = 0, 
             double const amplitude = 0.0, double const ei = 0.0, double const ea = 0.0)
              : m_spin(spin), m_i(i), m_a(a), m_amplitude(amplitude), m_ei(ei), m_ea(ea) { } 

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void dump() const;

         Spin     m_spin;;
         unsigned m_i;
         unsigned m_a;
         double   m_amplitude;
         double   m_ei;
         double   m_ea;
      protected:

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & m_spin;
            ar & m_i;
            ar & m_a;
            ar & m_amplitude;
            ar & m_ei;
            ar & m_ea;
         }
   };

   typedef Data::List<Data::ElectronicTransition> ElectronicTransitionList;

} } // end namespace IQmol::Data

#endif
