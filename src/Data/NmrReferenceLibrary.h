#ifndef IQMOL_DATA_NMRREFERENCELIBRARY_H
#define IQMOL_DATA_NMRREFERENCELIBRARY_H
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

#include <QString>

namespace IQmol {
namespace Data {

   class NmrReference;

   // Note that this is a singleton class and does not derive from the
   // Data::Base class.
   class NmrReferenceLibrary {

      public:
         static NmrReferenceLibrary& instance();

         void addReference(NmrReference const&);
         QStringList availableIsotopes() const;

         QList<NmrReference const*> filter(QString const& element, 
            QString const& system = QString(), QString const& method = QString());

         void dump() const;

      private:
         static NmrReferenceLibrary* s_instance;
         NmrReferenceLibrary();
         explicit NmrReferenceLibrary(NmrReferenceLibrary const&) { }
         ~NmrReferenceLibrary() { }

         static void load();
         static void cleanup();

         static QList<NmrReference*> s_library;
   };

} } // end namespace IQmol::Data

#endif
