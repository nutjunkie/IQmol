#ifndef IQMOL_PARSEJOBFILES_H
#define IQMOL_PARSEJOBFILES_H
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

#include "ParseFile.h"
#include "JobInfo.h"


namespace IQmol {

   class ParseJobFiles : public Parser::ParseFile {

      Q_OBJECT 

      public:
         enum Options {
            MakeActive = 0x001,  // Makes the resulting molecule active
            Overwrite  = 0x002,  // Any existing molecule with the same base will be overwritten
            AddStar    = 0x004   // Adds a star to the molecule in the Model View
         };

         ParseJobFiles(QString const& filePath, QString const& filter, void* moleculePointer);
         ParseJobFiles(QString const& filePath);
//deprecate
//         ParseJobFiles(JobInfo const& jobInfo);

         void setFlags(unsigned flags) { m_flags = flags; }
         unsigned flags() const { return m_flags; }
//deprecate
//       JobInfo const* jobInfo() const { return m_jobInfo; }
         void* moleculePointer() const { return m_moleculePointer; }

      private:
         unsigned m_flags;
//deprecate         JobInfo const* m_jobInfo;
         void* m_moleculePointer;
   };

} // end namespace IQmol

#endif
