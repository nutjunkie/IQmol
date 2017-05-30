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

#include "Geometry.h"
#include "AtomicProperty.h"
#include "Numerical.h"
#include "QsLog.h"
#include <QDebug>
#include "openbabel/mol.h"


namespace IQmol {
namespace Data {


Geometry::Geometry() : m_charge(0), m_multiplicity(1) { }

Geometry::Geometry(Geometry const& that) : Base()
{
   unsigned n(that.nAtoms());
   for (unsigned i = 0; i < n; ++i) {
       append(that.atomicNumber(i), that.position(i));
   }
   m_charge = that.m_charge;
   m_multiplicity = that.m_multiplicity;
   m_properties = that.m_properties;
}


Geometry::Geometry(QList<unsigned> const& atomicNumbers, QList<double> const& coordinates)
{
   unsigned n(atomicNumbers.size());
   if (3*n > (unsigned)coordinates.size()) {
      qDebug() << "Array length mismatch in Geometry constructor: 3x" 
               << atomicNumbers.size() << "!=" << coordinates.size();
      n = coordinates.size()/3;
   }

   double x, y, z;
   for (unsigned i = 0; i < n; ++i) {
       x = coordinates[3*i+0];
       y = coordinates[3*i+1];
       z = coordinates[3*i+2];
       append(atomicNumbers[i], qglviewer::Vec(x, y, z));
   }  
   m_charge = 0;
   m_multiplicity = 1;
}


QString Geometry::atomicSymbol(unsigned const i) const
{
   QString symbol;
   if (i < (unsigned)m_atoms.size()) symbol = m_atoms[i]->getLabel<AtomicSymbol>();
   return symbol;
}


unsigned Geometry::atomicNumber(unsigned const i) const
{
   int z(0);
   if (i < (unsigned)m_atoms.size()) z = m_atoms[i]->atomicNumber();
   return z;
}


qglviewer::Vec Geometry::position(unsigned const i) const
{
   qglviewer::Vec v;
   if (i < (unsigned)m_coordinates.size()) v = m_coordinates[i];
   return v;
}


QString Geometry::coordinatesAsString() const
{
   QString coords;

   if (m_atoms.size() == m_coordinates.size()) {
      unsigned nAtoms(m_atoms.size());
      for (unsigned i = 0; i <nAtoms; ++i) {
          qglviewer::Vec const& position(m_coordinates[i]);
          coords += QString("%1").arg(atomicSymbol(i), 3);
          coords += QString("%1").arg(position.x, 13, 'f', 7);
          coords += QString("%1").arg(position.y, 13, 'f', 7);
          coords += QString("%1").arg(position.z, 13, 'f', 7) + "\n";
      }

   }else {
      QLOG_WARN() << "Coordinate mismatch in Geometry::coordinatesAsString()";
   }

   coords.chop(1);
   return coords;
}


bool Geometry::sameAtoms(Geometry const& that) const
{
   bool same(m_atoms.size() == that.m_atoms.size());
   for (int i = 0; i < m_atoms.size(); ++i) {
       same = same && (m_atoms[i]->atomicNumber() == that.m_atoms[i]->atomicNumber());
   }
   return same;
}


bool Geometry::sameAtoms(QStringList const& symbols) const
{
   bool same(m_atoms.size() == symbols.size());
   if (same) {
      unsigned nAtoms(m_atoms.size());

      for (unsigned i = 0; i < nAtoms; ++i) {
          same = same && 
             (m_atoms[i]->getLabel<AtomicSymbol>().compare(symbols[i], 
                 Qt::CaseInsensitive) == 0);
      }
   }
   return same;
}


bool Geometry::sameAtoms(QList<unsigned> const& atomicNumbers) const
{
   bool same(m_atoms.size() == atomicNumbers.size());
   if (same) {
      unsigned nAtoms(m_atoms.size());
      for (unsigned i = 0; i < nAtoms; ++i) {
          same = same && (m_atoms[i]->atomicNumber() == atomicNumbers[i]);
      }
   }
   return same;
}



void Geometry::append(unsigned const z, qglviewer::Vec const& position)
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


void Geometry::append(QList<unsigned> const& z, QList<qglviewer::Vec> const& positions)
{
   if (z.size() != positions.size()) {
      throw std::runtime_error("Invalid coordinate data");
   }

   for (unsigned i = 0; i < (unsigned)z.size(); ++i) {
       append(z[i], positions[i]);
   }
}


void Geometry::setCharge(int const charge)
{
   m_charge = charge;
   int numberOfElectrons(totalNuclearCharge()-m_charge);
   if (Util::isEven(numberOfElectrons)) {
      if (Util::isEven(m_multiplicity)) {
         m_multiplicity -= 1;
      }
   }else {
      if (Util::isOdd(m_multiplicity)) {
          m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
      }
   }
}


void Geometry::setChargeAndMultiplicity(int const charge, unsigned const multiplicity)
{
   setMultiplicity(multiplicity);
   setCharge(charge);
}


void Geometry::setMultiplicity(unsigned const multiplicity)
{
   m_multiplicity = multiplicity;
   int numberOfElectrons(totalNuclearCharge()-m_charge);
   if (Util::isEven(m_multiplicity)) {
      if (Util::isEven(numberOfElectrons)) {
         m_charge += m_charge > 0 ? -1 : 1;
      }
   }else {
      if (Util::isOdd(numberOfElectrons)) {
         m_charge += (numberOfElectrons == 1) ? -1 : 1;
      }
   }
}


unsigned Geometry::totalNuclearCharge() const
{
   unsigned totalNuclearCharge(0);
   QList<Atom*>::const_iterator atom;
   for (atom = m_atoms.begin(); atom != m_atoms.end(); ++atom) {
       totalNuclearCharge += (*atom)->atomicNumber();
   }
   return totalNuclearCharge;
}


void Geometry::computeGasteigerCharges()
{
// This is returning zero charges for some reason
return;
    qDebug() << "Geometry::computeGasteigerCharges() not working correctly";
    OpenBabel::OBAtom* obAtom(0);
    OpenBabel::OBMol obMol;

    obMol.BeginModify();
    obMol.UnsetPartialChargesPerceived();
    for (int i = 0; i < m_atoms.size(); ++i) {
        obAtom = obMol.NewAtom();
        obAtom->SetAtomicNum(m_atoms[i]->atomicNumber());
        obAtom->SetVector(m_coordinates[i].x, m_coordinates[i].y, m_coordinates[i].z);
    }
    obMol.SetTotalCharge(m_charge);
    obMol.SetTotalSpinMultiplicity(m_multiplicity);
    obMol.EndModify();
    
    OpenBabel::OBMolAtomIter iter(&obMol);
    for (int i = 0; i < m_atoms.size(); ++i, ++iter) {
        int index(iter->GetIdx());
        qDebug() << "Setting Gasteiger Charge for" << index << "to" << iter->GetPartialCharge();
        GasteigerCharge& charge(m_atoms[i]->getProperty<GasteigerCharge>());
        charge.setValue(iter->GetPartialCharge());
    }
}

  
void Geometry::dump() const
{
   qDebug() << "Geometry:";
   for (int i = 0; i < m_atoms.size(); ++i) {
        qDebug() << "  " << atomicSymbol(i)    << "  " 
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


void Geometry::setCoordinates(QList<qglviewer::Vec> const& newCoordinates)
{
   if (newCoordinates.size() == m_coordinates.size()) {
   qglviewer::Vec vec(m_coordinates.first());
      m_coordinates = newCoordinates;
      vec  = m_coordinates.first();
   }else {
      QLOG_WARN() << "Coordinate mismatch in Geometry::setCoordinates";
   }
}



/*
void Geometry::translate(qglviewer::Vec const& shift)
{
   for (int i = 0; i < m_coordinates.size(); ++i) {
       m_coordinates[i] += shift;
   }
}
*/

} } // end namespace IQmol::Data
