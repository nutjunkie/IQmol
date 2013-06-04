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

#include "IQmolParser.h"
#include "Data.h"
#include <fstream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>


namespace IQmol {
namespace Parser2 {

Data::Bank& IQmol::parseFile(QString const& filePath)
{
   std::ifstream ifs(filePath.toStdString().data(), std::ios_base::binary);

   if (ifs.is_open()) {
      boost::iostreams::filtering_istream filter; 
      filter.push(boost::iostreams::gzip_decompressor());
      filter.push(ifs);
      Data::InputArchive inputArchive(filter);
      m_dataBank.serialize(inputArchive);
   }else {
      QString msg("Failed to open file for read: ");
      msg += filePath;
      m_errors.append(msg);
   }

   return m_dataBank;
}


void IQmol::saveData(QString const& filePath, Data::Bank& data)
{
   std::ofstream ofs(filePath.toStdString().data(), std::ios_base::binary);
   if (ofs.is_open()) {
      boost::iostreams::filtering_ostream filter;
      filter.push(boost::iostreams::gzip_compressor());
      filter.push(ofs);
      boost::archive::text_oarchive outputArchive(filter);
      data.serialize(outputArchive, 0);
   }else {
      QString msg("Failed to open file for write: ");
      msg += filePath;
      m_errors.append(msg);
    }
}

} } // end namespace IQmol::Parser
