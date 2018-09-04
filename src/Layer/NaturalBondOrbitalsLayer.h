#ifndef IQMOL_LAYER_NATURALBONDORBITALS_H
#define IQMOL_LAYER_NATURALBONDORBITALS_H
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

#include "OrbitalsLayer.h"
#include "NaturalBondOrbitals.h"


namespace IQmol {

namespace Configurator {
   class Orbitals;
}

namespace Layer {

   class NaturalBondOrbitals : public Orbitals {

      Q_OBJECT

      friend class Configurator::Orbitals;

      public:
         NaturalBondOrbitals(Data::NaturalBondOrbitals&);
         ~NaturalBondOrbitals() { }
         
      protected:
         QString description(Data::SurfaceInfo const&, bool const tooltip);

      private:
         // Cast of Orbitals::m_orbitals
         Data::NaturalBondOrbitals& m_naturalBondOrbitals;  
   };

} } // End namespace IQmol::Layer 
#endif
