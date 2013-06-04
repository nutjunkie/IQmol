#ifndef IQMOL_DATA_GEOMETRICPROPERTY_H
#define IQMOL_DATA_GEOMETRICPROPERTY_H
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

#include "Data.h"


namespace IQmol {
namespace Data {

   /// Base class for properties associated with a particular geometry such as
   /// dipole moments and energy.
   class GeometricProperty : public Base {
      public:
         Type::ID typeID() const { return Type::GeometricProperty; }
   };


   class Energy : public GeometricProperty {

      friend class boost::serialization::access;

      public:
         enum Units { Hartree, KJMol, KCalMol };
         Energy(double const value = 0.0, Units const units = Hartree) : m_value(value), 
            m_units(units) { }

         double value() const { return m_value; }
         Units  units() const { return m_units; }
         void dump() const;

         void setValue(double const value, Units const units = Hartree) {
             m_value = value;
             m_units  = units;
         }

         void serialize(InputArchive& ar, unsigned int const /* version */) {
            ar & m_value;
            ar & m_units;
         }
         void serialize(OutputArchive& ar, unsigned int const /* version */) {
            ar & m_value;
            ar & m_units;
         }

      private:
         double m_value;
         Units  m_units;
   };


   class ScfEnergy : public Energy {
      public:
         Type::ID typeID() const { return Type::ScfEnergy; }
   };


   class TotalEnergy : public Energy {  // Post-HF energy
      public:
         Type::ID typeID() const { return Type::TotalEnergy; }
   };


   class ForceFieldEnergy : public Energy {
      public:
         Type::ID typeID() const { return Type::ForceFieldEnergy; }
   };


   class DipoleMoment : public GeometricProperty {

      friend class boost::serialization::access;

      public:
         DipoleMoment(qglviewer::Vec const& dipole = qglviewer::Vec()) : m_dipole(dipole) { }
         Type::ID typeID() const { return Type::DipoleMoment; }
         void setValue(double const x, double const y, double const z) { 
            m_dipole.setValue(x,y,z); 
         } 

         void dump() const;
         qglviewer::Vec const& value() const { return m_dipole; }

         void serialize(InputArchive& ar, unsigned int const /* version */) {
            ar & m_dipole;
         }
         void serialize(OutputArchive& ar, unsigned int const /* version */) {
            ar & m_dipole;
         }

      private:
         qglviewer::Vec m_dipole;
   };



} } // end namespace IQmol::Data

#endif
