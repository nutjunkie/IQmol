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

#include "NmrReferenceLibrary.h"
#include <QDebug>


namespace IQmol {
namespace Data {

NmrReferenceLibrary* NmrReferenceLibrary::s_instance = 0;

NmrReferenceLibrary& NmrReferenceLibrary::instance() 
{
   if (s_instance == 0) { 
      s_instance = new NmrReferenceLibrary();
      atexit(NmrReferenceLibrary::cleanup);
   }
   return *s_instance;
}


NmrReferenceLibrary::NmrReferenceLibrary()
{
   load();
}


void NmrReferenceLibrary::cleanup()
{
   QList<NmrReference*>::iterator iter;
   for (iter = s_library.begin(); iter != s_library.end(); ++iter) {
       delete (*iter);
   }
}


void NmrReferenceLibrary::dump() const 
{
   QList<NmrReference*>::iterator iter;
   for (iter = s_library.begin(); iter != s_library.end(); ++iter) {
       (*iter)->dump();
   }
}


void NmrReferenceLibrary::load()
{  
   NmrReference* ref(0);
   QStringList methods, bases;

   methods << "HF" << "LDA";
   bases   << "STO-3G" << "STO-6G";

   // TMS
   QString system("TMS");
   for (int i = 0; i < methods.size(); ++i) {
       for (int j = 0; j < bases.size(); ++j) {
           ref = new NmrReference(system, methods[i], bases[i]); 
           ref->addElement("H", 32.0, 0.0);
           ref->addElement("C", 174.0, 0.0);
           s_library.append(ref);
       }
   }
}

} } // end namespace IQmol::Data
