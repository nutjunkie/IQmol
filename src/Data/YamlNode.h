#ifndef IQMOL_DATA_YAMLNODE_H
#define IQMOL_DATA_YAMLNODE_H
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
#include "yaml-cpp/yaml.h"


namespace IQmol {
namespace Data {

   class YamlNode : public YAML::Node, public Base {

      friend class boost::serialization::access;

      public:
         YamlNode() { }
         YamlNode(YAML::Node const& node) : YAML::Node(node) { }

         Type::ID typeID() const { return Type::YamlNode; }
         void dump() const;

         bool saveToFile(QString const& filePath);

         void serialize(InputArchive& /*ar*/, unsigned int const version = 0) {
            Q_UNUSED(version);
         }

         void serialize(OutputArchive& /*ar*/, unsigned int const version = 0) {
            Q_UNUSED(version);
         }
   };

} } // end namespace IQmol::Data

#endif
