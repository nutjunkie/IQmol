#ifndef IQMOL_DATA_FACTORY_H
#define IQMOL_DATA_FACTORY_H
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

   class Base;

   /// Singleton factory class for creation of Data classes.  Note that the
   /// implementation is basic - derived Data classes must be explicitly added to
   /// create() before they will work with the DataFactory.
   class Factory {
      public:
         static Factory& instance();
         Base* create(Type::ID const);

      private:
         static void destroy();
         explicit Factory(Factory const&) { }
         Factory() { }
         ~Factory() { }

         static Factory* s_instance;
   };

} } // end namespace IQmol::Data

#endif
