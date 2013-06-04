#ifndef IQMOL_DATA_GEOMETRY_H
#define IQMOL_DATA_GEOMETRY_H
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
#include "Atom.h"


using qglviewer::Vec;

namespace IQmol {
namespace Data {

   /// Data class representing molecule with a particular geometry.  
   class Geometry : public Base {

      friend class boost::serialization::access;

      public:
		 // Copy constructor only copies the charge, multiplicity, atomic
		 // numbers and positions.  Atom and Geometry properties are not copied.
         Geometry();
         Geometry(Geometry const&);

         Type::ID typeID() const { return Type::Geometry; }
         
         int nAtoms() const { return m_atoms.size(); }
         QString atomicSymbol(int const i) const;
         int atomicNumber(int const i) const;
         Vec position(int const i) const;
         QList<Vec> const& coordinates() const { return m_coordinates; }

         /// Use to convert between Bohr and Angstrom
         void scaleCoordinates(double const);

		 /// Two geometries are considered the same if their AtomLists contain
		 /// the same atoms in the same order.  Everything else could be
		 /// different.  This allows, for example, the structures of a geometry
         /// optimization to be considered the same.
         bool sameAtoms(Geometry const&) const;
         bool sameAtoms(QStringList const& symbols) const;

         void append(int const z, Vec const& position);
         void append(QString const& symbol, Vec const& position);
         void append(QList<int> const& z, QList<Vec> const& positions);

         void setCharge(int const charge);
         void setMultiplicity(int const multiplicity);
         void setChargeAndMultiplicity(int const charge, int const multiplicity);

         template <class P>
         bool setAtomicProperty(QList<double> values) {
            if (values.size() != m_atoms.size()) return false; 
            for (int i = 0; i < m_atoms.size(); ++i) {
                P* p(m_atoms[i]->getProperty<P>());
                if (!p) return false;
                p->setValue(values[i]);
            }
            return true;
         }

         template <class P>
         P* getProperty() {
            P* p(0);
            Bank::iterator iter;
            for (iter = m_properties.begin(); iter != m_properties.end(); ++iter) {
                if ( (p = dynamic_cast<P*>(*iter)) ) break; 
            }
            if (p == 0) {
               p = new P;
               m_properties.append(p);
            }
            return p;
         }

         template <class P>
         QStringList getLabels() {
            QStringList labels;
            QList<Atom*>::iterator iter;
            for (iter = m_atoms.begin(); iter != m_atoms.end(); ++iter) {
                labels.append((*iter)->getLabel<P>());
            }
            return labels;
         }

         void dump() const;

         void serialize(InputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

         void serialize(OutputArchive& ar, unsigned int const version = 0) {
            privateSerialize(ar, version);
         }

      private:
         template <class Archive>
         void privateSerialize(Archive& ar, unsigned const /* version */) {
            ar & m_atoms;
            ar & m_coordinates;
            ar & m_charge;
            ar & m_multiplicity;
            m_properties.serialize(ar);
         }

         int totalNuclearCharge() const;

         AtomList m_atoms;
         QList<Vec> m_coordinates;
         int m_charge;
         int m_multiplicity;
         Bank m_properties;
   };

   typedef Data::List<Data::Geometry> GeometryList;

} } // end namespace IQmol::Data

#endif
