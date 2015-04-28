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

#include "ParseJobFiles.h"


namespace IQmol {

ParseJobFiles::ParseJobFiles(QString const& filePath) : Parser::ParseFile(filePath),  
//   m_jobInfo(0), m_moleculePointer(0)
    m_moleculePointer(0)
{
   m_flags = MakeActive;
}


ParseJobFiles::ParseJobFiles(QString const& filePath, QString const& filter, 
   void* moleculePointer) : Parser::ParseFile(filePath, filter),  // m_jobInfo(0), 
   m_moleculePointer(moleculePointer)
{
   m_flags = Overwrite | AddStar;
}


/*
ParseJobFiles::ParseJobFiles(JobInfo const& jobInfo) : 
   Parser::ParseFile(jobInfo.get(JobInfo::LocalWorkingDirectory)), m_jobInfo(&jobInfo), 
   m_moleculePointer(0) 
{
   m_flags = Overwrite | AddStar;
}
*/

} // end namespace IQmol
