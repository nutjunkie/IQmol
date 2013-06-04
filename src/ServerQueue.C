/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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

#include "ServerQueue.h"
#include "QsLog.h"


namespace IQmol {

int ServerQueue::parseResource(QString& r)
{
   int  Mb(0);
   bool ok(false);

   if (r.contains("gb", Qt::CaseInsensitive)) {
      Mb = r.remove("gb").toInt(&ok);
      Mb *= 1024;
   }else if (r.contains("mb", Qt::CaseInsensitive)) {
      Mb = r.remove("mb").toInt(&ok);
   }

   if (!ok) QLOG_DEBUG() << "Failed to parse:" << r;

   return Mb;
}

} // end namespace IQmol

