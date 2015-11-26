/*******************************************************************************
       
  Copyright (C) 2011-2015ndrew Gilbert
           
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
namespace Parser {

bool IQmol::parseFile(QString const& filePath)
{
   std::ifstream ifs(filePath.toStdString().data(), std::ios_base::binary);

   if (ifs.is_open()) {
      boost::iostreams::filtering_istream filter; 
#ifdef Q_OS_MAC
      // Can't get this working on Windows, or Linux, or even Mac now
      //filter.push(boost::iostreams::gzip_decompressor());
#endif
      filter.push(ifs);
      Data::InputArchive inputArchive(filter);
      m_dataBank.serialize(inputArchive);
   }else {
      QString msg("Failed to open file for read: ");
      msg += filePath;
      m_errors.append(msg);
   }

   return m_errors.isEmpty();
}


// Note the Bank should be a const&, but the serialize functions are declared
// non-const for some Boost-related reason.
bool IQmol::save(QString const& filePath, Data::Bank& data)
{
   std::ofstream ofs(filePath.toStdString().data(), std::ios_base::binary);
   if (ofs.is_open()) {
      boost::iostreams::filtering_ostream filter;
#ifdef Q_OS_MAC
      //filter.push(boost::iostreams::gzip_compressor());
#endif
      filter.push(ofs);
      boost::archive::text_oarchive outputArchive(filter);
      data.serialize(outputArchive, 0);
   }else {
      QString msg("Failed to open file for write: ");
      msg += filePath;
      m_errors.append(msg);
    }

    return m_errors.isEmpty();
}

} } // end namespace IQmol::Parser
