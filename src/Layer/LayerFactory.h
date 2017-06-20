#ifndef IQMOL_LAYER_FACTORY_H
#define IQMOL_LAYER_FACTORY_H
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

#include "Layer.h"

namespace IQmol {

namespace Data {
   class Base;
   class Bank;
   class FileList;
   class Geometry;
   class Frequencies;
   class OrbitalsList;
   class GeometryList;
   class PointChargeList;
   class EfpFragmentList;
}

namespace Layer {

   /// Singleton factory class for creation of Layer classes from their
   /// associated Data  namespace structures.  The toLayers() member function
   /// must be updated to handle new Data classes.
   class Factory {
      public:
         static Factory& instance();
         Layer::List toLayers(Data::Base&);

      private:
         Layer::List convert(Data::Bank&);
         Layer::List convert(Data::Geometry&);
         Layer::List convert(Data::FileList&);
         Layer::List convert(Data::GeometryList&);
         Layer::List convert(Data::OrbitalsList&);
         Layer::List convert(Data::EfpFragmentList&);
         Layer::List convert(Data::PointChargeList&);

         static void destroy();
         explicit Factory(Factory const&) { }
         Factory() { }
         ~Factory() { }

         static Factory* s_instance;
   };

} } // end namespace IQmol::Data

#endif
