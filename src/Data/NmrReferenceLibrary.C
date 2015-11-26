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

#include "NmrReferenceLibrary.h"
#include "NmrReference.h"
#include <QDebug>


namespace IQmol {
namespace Data {

NmrReferenceLibrary* NmrReferenceLibrary::s_instance = 0;
QList<NmrReference*> NmrReferenceLibrary::s_library = QList<NmrReference*>();


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


QList<NmrReference const*> NmrReferenceLibrary::filter(QString const& element, 
  QString const& system, QString const& method)
{
   QList<NmrReference const*> hits;

   bool systemCheck(!system.isEmpty());
   bool methodCheck(!method.isEmpty());
   
   QList<NmrReference*>::iterator iter;
   for (iter = s_library.begin(); iter != s_library.end(); ++iter) {
       if ((*iter)->contains(element)) {
          bool hit(true);
          if (systemCheck && (*iter)->system() != system) hit = false;
          if (methodCheck && (*iter)->method() != method) hit = false;
          if (hit) hits.append(*iter);
       }
   }

   return hits;
}


void NmrReferenceLibrary::addReference(NmrReference const& reference)
{
   s_library.append(new NmrReference(reference));
}


void NmrReferenceLibrary::dump() const 
{
   QList<NmrReference*>::iterator iter;
   for (iter = s_library.begin(); iter != s_library.end(); ++iter) {
       (*iter)->dump();
   }
}


QStringList NmrReferenceLibrary::availableIsotopes() const
{
   QStringList isotopes;

   isotopes.append("Proton (1H)");
// isotopes.append("Boron (11B)");
   isotopes.append("Carbon (13C)");
// isotopes.append("Nitrogen (15N)");
// isotopes.append("Fluorine (19F)");
   isotopes.append("Silicon (29Si)");
// isotopes.append "Phosphorus (31P)";

   return isotopes;
}


void NmrReferenceLibrary::load()
{  
   NmrReference* ref(0);
   QStringList methods, bases;

   methods << "HF" << "LDA";
   bases   << "STO-3G" << "STO-6G";

/*
   // Tetramethylsilane (H, C, Si)
   QString system("TMS");
   for (int i = 0; i < methods.size(); ++i) {
       for (int j = 0; j < bases.size(); ++j) {
           QString method(methods[i] + "/" + bases[j]); 
           ref = new NmrReference(system, method); 
           ref->addElement("H",  32.0, 0.0);
           ref->addElement("C",  174.0, 0.0);
           ref->addElement("Si", 50000.0, 0.0);
           s_library.append(ref);
       }
   }
*/

   QString system("TMS");

ref = new NmrReference("TMS", "HF/STO-3G");
ref->addElement("Si", 578.55750846);
ref->addElement("C",  249.43162577);
ref->addElement("H",   33.75316081);
s_library.append(ref);

ref = new NmrReference("TMS", "HF/6-31G");
ref->addElement("Si", 484.50460832);
ref->addElement("C",  208.19772893);
ref->addElement("H",   33.67482488);
s_library.append(ref);

ref = new NmrReference("TMS", "B3LYP/6-31G");
ref->addElement("Si", 432.38303834);
ref->addElement("C",  194.31808787);
ref->addElement("H",   32.78097962);
s_library.append(ref);

ref = new NmrReference("TMS", "HF/6-31g*");
ref->addElement("Si", 450.14833029);
ref->addElement("C",  201.72683332);
ref->addElement("H",   32.90343930);
s_library.append(ref);

ref = new NmrReference("TMS", "B3LYP/6-31g*");
ref->addElement("Si", 413.56045472);
ref->addElement("C",  189.63133174);
ref->addElement("H",   32.18393750);
s_library.append(ref);

ref = new NmrReference("TMS", "HF/6-31g**");
ref->addElement("Si", 448.39860170);
ref->addElement("C",  203.14180478);
ref->addElement("H",   32.33377646);
s_library.append(ref);

ref = new NmrReference("TMS", "B3LYP/6-31g**");
ref->addElement("Si", 411.79455658);
ref->addElement("C",  191.71944127);
ref->addElement("H",   31.75480046);
s_library.append(ref);

ref = new NmrReference("TMS", "HF/6-31+G*");
ref->addElement("Si", 446.66061534);
ref->addElement("C",  201.83303666);
ref->addElement("H",   32.81723854);
s_library.append(ref);

ref = new NmrReference("TMS", "B3LYP/6-31+G*");
ref->addElement("Si", 408.14935394);
ref->addElement("C",  190.67818876);
ref->addElement("H",   32.06583609);
s_library.append(ref);

/*
   // Trimethyl Borate (B)
   system = "Trimethyl Borate";
   for (int i = 0; i < methods.size(); ++i) {
       for (int j = 0; j < bases.size(); ++j) {
           QString method(methods[i] + "/" + bases[j]); 
           ref = new NmrReference(system, method); 
           ref->addElement("B",  32.0, 0.0);
           s_library.append(ref);
       }
   }
    
   // Ammonia (N)
   system = "Ammonia";
   for (int i = 0; i < methods.size(); ++i) {
       for (int j = 0; j < bases.size(); ++j) {
           QString method(methods[i] + "/" + bases[j]); 
           ref = new NmrReference(system, method); 
           ref->addElement("N",  32.0, 0.0);
           s_library.append(ref);
       }
   }

   // Trichlorofluoromethane (F)
   system = "Trichlorofluoromethane";
   for (int i = 0; i < methods.size(); ++i) {
       for (int j = 0; j < bases.size(); ++j) {
           QString method(methods[i] + "/" + bases[j]); 
           ref = new NmrReference(system, method); 
           ref->addElement("F",  32.0, 0.0);
           s_library.append(ref);
       }
   }

   system = "Fluorobenzene";
   for (int i = 0; i < methods.size(); ++i) {
       for (int j = 0; j < bases.size(); ++j) {
           QString method(methods[i] + "/" + bases[j]); 
           ref = new NmrReference(system, method); 
           ref->addElement("F",  32.0, -113.13);
           s_library.append(ref);
       }
break;
   }

   //Phosphoric Acid H3PO4 (P) 
   system = "Phosphoric Acid";
   for (int i = 0; i < methods.size(); ++i) {
       for (int j = 0; j < bases.size(); ++j) {
           QString method(methods[i] + "/" + bases[j]); 
           ref = new NmrReference(system, method); 
           ref->addElement("P",  32.0, 0.0);
           s_library.append(ref);
       }
   }
*/
}

} } // end namespace IQmol::Data
