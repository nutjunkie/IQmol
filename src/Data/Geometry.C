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

#include "Geometry.h"
#include "Atom.h"
#include "AtomicProperty.h"
#include "IQmol.h"
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID GeometryList::TypeID = Type::GeometryList;


Geometry::Geometry() : m_charge(0), m_multiplicity(1) { }

Geometry::Geometry(Geometry const& that) : Base()
{
   int n(that.nAtoms());
   for (int i = 0; i < n; ++i) {
       append(that.atomicNumber(i), that.position(i));
   }
   m_charge = that.m_charge;
   m_multiplicity = that.m_multiplicity;
}


QString Geometry::atomicSymbol(int const i) const
{
   QString symbol;
   if (i < m_atoms.size()) symbol = m_atoms[i]->getLabel<AtomicSymbol>();
   return symbol;
}


int Geometry::atomicNumber(int const i) const
{
   int z(0);
   if (i < m_atoms.size()) z = m_atoms[i]->atomicNumber();
   return z;
}


qglviewer::Vec Geometry::position(int const i) const
{
   qglviewer::Vec v;
   if (i < m_coordinates.size()) v = m_coordinates[i];
   return v;
}


bool Geometry::sameAtoms(Geometry const& that) const
{
   bool same(m_atoms.size() == that.m_atoms.size());
   if (same) {
      for (int i = 0; i < m_atoms.size(); ++i) {
          same = same && (m_atoms[i]->atomicNumber() == that.m_atoms[i]->atomicNumber());
      }
   }
   return same;
}


bool Geometry::sameAtoms(QStringList const& symbols) const
{
   bool same(m_atoms.size() == symbols.size());
   if (same) {
      for (int i = 0; i < m_atoms.size(); ++i) {
          same = same && (m_atoms[i]->getLabel<AtomicSymbol>() == symbols[i]);
      }
   }
   return same;
}


void Geometry::append(int const z, qglviewer::Vec const& position)
{
   Atom* atom = new Atom(z);
   m_atoms.push_back(atom);
   m_coordinates.push_back(position);
}


void Geometry::append(QString const& symbol, qglviewer::Vec const& position)
{
   Atom* atom = new Atom(symbol);
   m_atoms.push_back(atom);
   m_coordinates.push_back(position);
}


void Geometry::append(QList<int> const& z, QList<qglviewer::Vec> const& positions)
{
   if (z.size() != positions.size()) {
      throw std::runtime_error("Invalid coordinate data");
   }

   for (int i = 0; i < z.size(); ++i) {
       append(z[i], positions[i]);
   }
}


void Geometry::setCharge(int const charge)
{
   m_charge = charge;
   int numberOfElectrons(totalNuclearCharge()-m_charge);
   if (isEven(numberOfElectrons)) {
      if (isEven(m_multiplicity)) {
         m_multiplicity -= 1;
      }
   }else {
      if (isOdd(m_multiplicity)) {
          m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
      }
   }
}


void Geometry::setChargeAndMultiplicity(int const charge, int const multiplicity)
{
   setMultiplicity(multiplicity);
   setCharge(charge);
}


void Geometry::setMultiplicity(int const multiplicity)
{
   m_multiplicity = multiplicity;
   int numberOfElectrons(totalNuclearCharge()-m_charge);
   if (isEven(m_multiplicity)) {
      if (isEven(numberOfElectrons)) {
         m_charge += m_charge > 0 ? -1 : 1;
      }
   }else {
      if (isOdd(numberOfElectrons)) {
         m_charge += (numberOfElectrons == 1) ? -1 : 1;
      }
   }
}


int Geometry::totalNuclearCharge() const
{
   int totalNuclearCharge(0);
   QList<Atom*>::const_iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       totalNuclearCharge += (*atom)->atomicNumber();
   }
   return totalNuclearCharge;
}

  
void Geometry::dump() const
{
   qDebug() << "Geometry:";
   for (int i = 0; i < m_atoms.size(); ++i) {
        qDebug() << atomicSymbol(i)    << "  " 
                 << m_coordinates[i].x << "  " 
                 << m_coordinates[i].y << "  " 
                 << m_coordinates[i].z;
   }
   m_properties.dump();
#if 0 
   for (int i = 0; i < m_atoms.size(); ++i) {
         m_atoms[i]->dump();
   }
#endif
}


void Geometry::scaleCoordinates(double const scale)
{
   for (int i = 0; i < m_coordinates.size(); ++i) {
       m_coordinates[i] *= scale;
   }
}


} } // end namespace IQmol::Data
