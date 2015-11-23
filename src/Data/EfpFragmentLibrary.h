#ifndef IQMOL_DATA_EFPFRAGMENTLIBRARY_H
#define IQMOL_DATA_EFPFRAGMENTLIBRARY_H
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

   class Geometry;

   /// Singleton class that manages a library of EfpFragments.  These may come
   /// from either the internal library or from an input/output file.
   class EfpFragmentLibrary : Base {

      public:
         static EfpFragmentLibrary& instance();

         Type::ID typeID() const { return Type::EfpFragmentLibrary; }

         // Adds a custom fragment in to the library
         void add(QString const& fragmentName, Geometry*, QString const& params = QString());

		 // Attempts to load a fragment from the library, returning true if
		 // successful or if the fragment already exists.
         bool add(QString const& fragmentName);

         QStringList availableFragments() const;
         Geometry const& geometry(QString const& fragmentName);
         QString const& parameters(QString const& fragmentName) const;
         QString getFilePath(QString const& fragmentName);

		 // Returns true if the fragment is in the library, or can be
		 // successfully loaded from the standard library.
         bool defined(QString const& fragmentName);

         // Returns true if the fragment is currently in the library.
         bool isLoaded(QString const& fragmentName) const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void dump() const;

      private:
         static void checkOut();

         static EfpFragmentLibrary* s_instance;
         static QMap<QString, Geometry*> s_geometries;
		 // Only parameters for fragments not in the standard library are
		 // stored, so an empty parameter string indicates a library fragment
         static QMap<QString, QString> s_parameters;

         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & s_geometries;
            ar & s_parameters;
         }

         EfpFragmentLibrary() { }
         explicit EfpFragmentLibrary(EfpFragmentLibrary const&) : Base() { }
         ~EfpFragmentLibrary() { }
   };

} } // end namespace IQmol::Data

#endif
