#ifndef IQMOL_DATA_ATOM_H
#define IQMOL_DATA_ATOM_H
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

#include "DataList.h"
#include "Bank.h"


namespace IQmol {
namespace Data {

   class AtomicProperty;

   /// Data structure representing an atom.  Note that 
   class Atom : public Base {

      friend class boost::serialization::access;

      public:
         Atom(int const Z = 0) :  m_atomicNumber(Z) { }
         Atom(QString const& symbol);

         Type::ID typeID() const { return Type::Atom; }
         int atomicNumber() const { return m_atomicNumber; }
         void dump() const;

         /// Returns a pointer to the requested AtomicProperty, if it exists, 
         /// otherwise creates a new one and adds it to the list.
         template <class P>
         P* getProperty() {
            P* p(0);
            Bank::iterator iter;
            for (iter = m_properties.begin(); iter != m_properties.end(); ++iter) {
                if ( (p = dynamic_cast<P*>(*iter)) ) break; 
            }
            if (p == 0) {
               p = new P(m_atomicNumber);
               m_properties.append(p);
            }
            return p;
         }

         template <class P>
         QString getLabel() {
            return getProperty<P>()->label();
         }
 
         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         static int atomicNumber(QString const&);

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned int const) {
            ar & m_atomicNumber;
            m_properties.serialize(ar);
         }

         int m_atomicNumber;
         Bank m_properties;
   };

   typedef Data::List<Data::Atom> AtomList;


} } // end namespace IQmol::Data

#endif
