#ifndef IQMOL_DATA_BANK_H
#define IQMOL_DATA_BANK_H
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
#include "DataFactory.h"
#include <boost/serialization/split_member.hpp>


namespace IQmol {
namespace Data {

   /// Class representing a group of Data objects.  Basically this class
   /// manages the serialization of several different objects (of the same
   /// or different classes) to the one file.  The load functions makes 
   /// use of the Data::Factory to create the required objects based on 
   /// the saved TypeIDs.  Note that the Bank assumes ownership of the
   /// objects and deletes them in the dtor.  To avoid this the the objects
   /// should be taken using the QList::take*() functions.
   class Bank : public Base, public QList<Base*> {

	  friend class boost::serialization::access;

	  public: 
         Bank() { }
         ~Bank();
         Type::ID typeID() const { return Type::Bank; }

         void merge(Bank* bank);

         template <class T>
         QList<T*> findData() {
            QList<T*> list;
            T* t(0);
            for (int i = 0; i < size(); ++i) {
                if ( (t = dynamic_cast<T*>(value(i))) ) list.append(t);
            }
            return list;
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            int n(size());
            int id;
            ar & n;
            for (int i = 0; i < n; ++i) {
                id = at(i)->typeID();
                ar & id;
                at(i)->serialize(ar, version);
            }
         }

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            Type::ID typeID;
            Base* base;
            int n;
            ar & n;
            for (int i = 0; i < n; ++i) {
                ar & typeID;
                base = Factory::instance().create(typeID);
                base->serialize(ar, version);
                append(base);
            }
         }

         void dump() const;

      private:
         Bank(Bank const&);

         BOOST_SERIALIZATION_SPLIT_MEMBER();
   };

} } // end namespace IQmol::Data

#endif
