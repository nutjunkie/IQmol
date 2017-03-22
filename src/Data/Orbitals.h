#ifndef IQMOL_DATA_ORBITALS_H
#define IQMOL_DATA_ORBITALS_H
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

#include "Data.h"


namespace IQmol {
namespace Data {

   /// Base class for different orbital types
   class Orbitals : public Base {

      friend class boost::serialization::access;

      public:
         enum OrbitalType { Undefined = 0, 
                            Canonical, 
                            Localized, 
                            NaturalTransition, 
                            NaturalBond 
                          };

         Orbitals() : m_orbitalType(Undefined) { }

// This should be fixed so that the orbitalType function is declared virtual
         //OrbitalType orbitalType() const = 0;
         void setOrbitalType(OrbitalType const& orbitalType) { m_orbitalType = orbitalType; }
         OrbitalType orbitalType() const { return m_orbitalType; }

         static QString toString(OrbitalType const);

         QString const& label() const { return m_label; }
         void setLabel(QString const& label) { m_label = label; }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) 
         {
            ar & m_label;
         }

// Virtual
         OrbitalType m_orbitalType;
         QString m_label;
   };

} } // end namespace IQmol::Data

#endif
