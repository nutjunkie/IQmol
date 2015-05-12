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

#include "DataList.h"
#include "QGLViewer/vec.h"


namespace IQmol {
namespace Data {

   /// Data class representing molecule with a particular geometry.  
   class ElectronicTransition : public Base {

      friend class boost::serialization::access;

      public:
         ElectronicTransition(double const energy = 0.0, double const strength = 0.0,
            qglviewer::Vec const transitionMoment = qglviewer::Vec()) : 
            m_energy(energy), m_strength(strength), m_transitionMoment(transitionMoment) { }

         Type::ID typeID() const { return Type::ElectronicTransition; }
         
         double energy()   const { return m_energy; }
         double strength() const { return m_strength; }
         qglviewer::Vec const& transitionMoment() { return m_transitionMoment; }

         void setEnergy(double const energy) { m_energy = energy; }
         void setStrength(double const strength) { m_strength = strength; }
         void setTransitionMoment(qglviewer::Vec const& vec) { m_transitionMoment = vec; }

         bool operator<(ElectronicTransition const& that) {
            return (m_energy < that.m_energy);
         }

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
            ar & m_transitionMoment;
         }

         double m_energy;
         double m_strength;
         qglviewer::Vec m_transitionMoment;
        // Mulliken charges or something to show the movement of the electron
   };

   typedef Data::List<Data::ElectronicTransition> ElectronicTransitionList;

} } // end namespace IQmol::Data

#endif
