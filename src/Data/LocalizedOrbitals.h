#ifndef IQMOL_DATA_LOCALIZEDORBITALS_H
#define IQMOL_DATA_LOCALIZEDORBITALS_H
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

#include "Orbitals.h"
#include "Surface.h"


namespace IQmol {
namespace Data {

   class LocalizedOrbitals : public Orbitals {

      friend class boost::serialization::access;

      public:
         enum LocalizationMethod { Undefined = 0, 
                                   Boys, 
                                   EdmistonRuedenberg, 
                                   PipekMezey };

         static QString toString(LocalizationMethod const);

         LocalizedOrbitals() : Orbitals() { }

         LocalizedOrbitals(unsigned const nAlpha, unsigned const nBeta, ShellList const& shells,
            QList<double> const& alphaCoefficients, QList<double> const& betaCoefficients,
            LocalizationMethod const, bool const restricted = false);
            
         Type::ID typeID() const { return Type::LocalizedOrbitals; }

      private:
         LocalizationMethod m_localizationMethod;
   };

} } // end namespace IQmol::Data

#endif
