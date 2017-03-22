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

#include "ShellList.h"
#include <cmath>


namespace IQmol {
namespace Data {

unsigned ShellList::nBasis() const
{
    unsigned n(0);
    for (int i = 0; i < size(); ++i) {
        n += at(i)->nBasis();
    }
   return n;
}


void ShellList::boundingBox(qglviewer::Vec& min, qglviewer::Vec& max) 
{
   if (isEmpty()) {
      min.x = min.y = min.z = 0.0;
      max.x = max.y = max.z = 0.0;
      return;
   }

   at(0)->boundingBox(min, max);

   qglviewer::Vec tmin, tmax;
   ShellList::const_iterator iter;
   for (iter = begin(); iter != end(); ++iter) {
       (*iter)->boundingBox(tmin, tmax);
       min.x = std::min(tmin.x, min.x);
       min.y = std::min(tmin.y, min.y);
       min.z = std::min(tmin.z, min.z);
       max.x = std::max(tmax.x, max.x);
       max.y = std::max(tmax.y, max.y);
       max.z = std::max(tmax.z, max.z);
   }
}


void ShellList::dump() const
{
   for (int i = 0; i < size(); ++i) at(i)->dump();
}

} } // end namespace IQmol::Data
