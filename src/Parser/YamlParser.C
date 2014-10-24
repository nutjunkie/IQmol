/*******************************************************************************
       
  Copyright (C) 2011-13 Andrew Gilbert
           
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

#include "YamlParser.h"
#include "YamlNode.h"
#include "yaml-cpp/yaml.h"
#include "yaml-cpp/eventhandler.h"
#include "TextStream.h"
#include "QsLog.h"
#include "QFile.h"

#include <fstream>
#include <iostream>

namespace IQmol {
namespace Parser {

bool Yaml::parse(TextStream& textStream)
{
   QByteArray byteArray(textStream.readAll().toLatin1());
   std::string s(std::string(byteArray.data()));
   std::istringstream iss(std::string(byteArray.data()));

   return parse(iss);
}


bool Yaml::parseFile(QString const& filePath)
{
   std::ifstream ifs;
   ifs.open(QFile::encodeName(filePath).data());
   if (!ifs) {
      m_errors.append("Failed to open file for reading");
      return false;
   }

   return parse(ifs);
}


bool Yaml::parse(std::istream& is)
{
  bool okay(true);
  try {
     YAML::Node doc(YAML::Load(is));
     Data::YamlNode* node(new Data::YamlNode(doc));
     m_dataBank.append(node);
   } catch (YAML::Exception& e) {
      m_errors.append(e.what());
      return false;
   }

   return okay;
}

} } // end namespace IQmol::Parser
