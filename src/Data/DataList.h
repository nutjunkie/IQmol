#ifndef IQMOL_DATA_LIST_H
#define IQMOL_DATA_LIST_H
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
#include <QList>


namespace IQmol {
namespace Data {

   /// Template class representing a list of Data object pointers.  Note that
   /// on destruction the List detetes the contained objects.  To avoid this the
   /// objects should be taken using the QList::take*() functions.
   template <class T>
   class List : public Base, public QList<T*> {

      friend class boost::serialization::access;

      public:
         ~List() { destroy(); }

         const static Type::ID TypeID;
         Type::ID typeID() const { return TypeID; }

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            load(ar, version);
         }
         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            save(ar, version);
         }

         void dump() const {
            for (int i = 0; i < this->size(); ++i) {
                this->at(i)->dump();
            }
         }


      protected:
         void destroy() {
            for (int i = 0; i < this->size(); ++i) {
                delete this->at(i);
            }
            this->clear();
         }


      private:
         template<class Archive>
         void save(Archive & ar, const unsigned int version) const
         {
            int n(this->size());
            ar & n;
            for (int i = 0; i < n; ++i) {
                this->at(i)->serialize(ar, version);
            }
         }

         template<class Archive>
         void load(Archive & ar, const unsigned int version)
         {
            int n;
            ar & n;
            for (int i = 0; i < n; ++i) {
                T* t(new T());
                t->serialize(ar, version);
                this->append(t);
            }
         }

         BOOST_SERIALIZATION_SPLIT_MEMBER();
   };

} } // end namepspace Data::IQmol

#endif
