#ifndef IQMOL_DATA_MOLECULE_H
#define IQMOL_DATA_MOLECULE_H
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
#include "Bank.h"


namespace IQmol {
namespace Data {

   /// Essentially a container for Data objects pertaining to a single
   /// molecule.  
   class Molecule : public Bank {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::Molecule; }
   };

   typedef Data::List<Data::Molecule> MoleculeList;

} } // end namespace IQmol::Data

#endif
