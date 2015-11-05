#ifndef IQMOL_DATA_NMRREFERENCE_H
#define IQMOL_DATA_NMRREFERENCE_H
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


namespace IQmol {
namespace Data {

   class NmrReference : public Base {

      friend class boost::serialization::access;

      public:
         NmrReference(QString const& method = QString()) : m_method(method) { }

         Type::ID typeID() const { return Type::NmrReference; }

         void addElement(QString const& symbol, double const shfit);
         double shift(QString const& symbol) const;

         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            Q_UNUSED(version);
            ar & m_method;
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_method;
            ar & m_shifts;
         }

      private:
         QString m_method;  // level of theory
         QMap<QString, double> m_shifts;
   };

   typedef Data::List<Data::NmrReference> NmrReferenceList;

} } // end namespace IQmol::Data

#endif
