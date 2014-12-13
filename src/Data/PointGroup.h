#ifndef IQMOL_DATA_POINTGROUP_H
#define IQMOL_DATA_POINTGROUP_H
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
#include <QDebug>


namespace IQmol {
namespace Data {

   class PointGroup : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::PointGroup; }

         PointGroup(QString const& pointGroup = "C1") : m_pointGroup(pointGroup) { }

         void setValue(QString const& pointGroup) { m_pointGroup = pointGroup; }
         QString value() const { return m_pointGroup; }

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_pointGroup;
         }
         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_pointGroup;
         }

         void dump() const {
            qDebug() << "  " << m_pointGroup;
         }

      private:
         QString m_pointGroup;
   };

} } // end namespace IQmol::Data

#endif
