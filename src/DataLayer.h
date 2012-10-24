#ifndef IQMOL_DATALAYER_H
#define IQMOL_DATALAYER_H
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

#include "BaseLayer.h"


namespace IQmol {
namespace Layer {

   class Molecule;

   /// Base class for Layers that represent data associated with a Molecule.
   /// These include MOCoefficients, Frequencies and AtomLists.
   class Data : public Base {

      Q_OBJECT 

      public:
         Data(QString const& text = QString(), QObject* parent = 0) 
            : Base(text, parent), m_molecule(0) 
         { 
            setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
         }
         virtual void setMolecule(Molecule* molecule) { m_molecule = molecule; }

      protected:
         QString m_fileName;   // where the data came from
         Molecule* m_molecule; // what Molecule the data is associated with
   };

} // end namespace Layer

typedef QList<Layer::Data*> DataList;

/*
  class DataList : public  QList<Layer::Data*> {   
         /// Convenience function that returns a pointer to the first data layer 
         /// of the specfied type that the file contains. 
         template <class T> T* getData(); 
      template <class T>
      T* Base::getData() 
      {         
         T* t(0);
         DataList::iterator iter;
         for (iter =  m_dataList.begin(); iter != m_dataList.end(); ++iter) {
             if (t = dynamic_cast<T*>(*iter)) break;
         }   
         return t;
      } 
   };

*/

} // end namespace IQmol::Layer

#endif
