#ifndef IQMOL_DATA_REMSECTIONDATA_H
#define IQMOL_DATA_REMSECTIONDATA_H
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

   class RemSection: public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::RemSection; }

         QString value(QString const& key) const; 
         void insert(QString const& key, QString const& value);
         bool isSet(QString const& key) const;
         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned int const) {
            ar & m_rem;
         }

         QMap<QString, QString> m_rem;
   };


} } // end namespace IQmol::Data

#endif
