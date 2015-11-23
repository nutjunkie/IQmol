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

#include "YamlNode.h"
#include <QDebug>
#include <QFile>


namespace IQmol {
namespace Data {


void YamlNode::dump() const 
{
   qDebug() << "YAML node contents:";
   std::cout << YAML::Dump(*this);
}


bool YamlNode::saveToFile(QString const& filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QByteArray buffer("# IQmol Server Configuration\n---\n");
    buffer.append(QString::fromStdString(YAML::Dump(*this)));
    buffer.append("\n");
    file.write(buffer);
    file.close();
    return true;
}

} } // end namespace IQmol::Data
