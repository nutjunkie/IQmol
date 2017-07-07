#ifndef IQMOL_DATA_POINTGROUP_H
#define IQMOL_DATA_POINTGROUP_H
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

   class PointGroup : public Base {

      friend class boost::serialization::access;

      public:
         enum Group { undef = 0,
            C1, C2,  C3,  C4,  C5,  C6,  C7,  C8, 
            Cs, C2h, C3h, C4h, C5h, C6h, C7h, C8h,
                C2v, C3v, C4v, C5v, C6v, C7v, C8v,
                D2,  D3,  D4,  D5,  D6,  D7,  D8, 
                D2d, D3d, D4d, D5d, D6d, D7d, D8d,
                D2h, D3h, D4h, D5h, D6h, D7h, D8h,
                Ci,       S4,       S6,       S8, 
                T,   Td,  Th,  O,   Oh,  I,   Ih, 
                Civ, Dih, MaxPG
         }; 

         Type::ID typeID() const { return Type::PointGroup; }

         PointGroup(Group group = undef) : m_group(group) { }
         // Used to initialize from the return value from symmol
         PointGroup(QString const&);

         QString toString() const;  
         QString toGLString() const;  // sans subscripts

         void clear() { m_group = undef; }
         void setPointGroup(Group const group) { m_group = group; }
         void setPointGroup(QString const& pg);

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_group;
         }
         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            Q_UNUSED(version);
            ar & m_group;
         }

         void dump() const;

      private:
         Group m_group;
   };

} } // end namespace IQmol::Data

#endif
