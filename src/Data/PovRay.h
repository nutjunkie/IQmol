#ifndef IQMOL_DATA_POVRAY_H
#define IQMOL_DATA_POVRAY_H
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

   class PovRay : public Base {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::PovRay; }

         QMap<QString, QString> textures() const { 
             return m_textures;
         }

         void addTexture(QString const& name, QString const& declaration) {
             m_textures.insert(name, declaration);
         }

         void dump() const;

         void serialize(InputArchive& ar, unsigned int const /*version*/) {
            ar & m_textures;
         }

         void serialize(OutputArchive& ar, unsigned int const /*version*/) {
            ar & m_textures;
         }

      private:
         QMap<QString, QString> m_textures;
   };

} } // end namespace IQmol::Data

#endif
