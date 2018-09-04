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

#include "InfoLayer.h"
#include "DipoleLayer.h"
#include "MoleculeLayer.h"
#include "Numerical.h"
#include "SymmetryLayer.h"
#include "StringFormat.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {

Info::Info(Molecule* molecule) : m_dipoleEstimated(true), m_configurator(*this),
   m_symmetry(m_pointGroup)
{
   clear();
   setText("Info");
   setMolecule(molecule);
   //appendLayer(&m_symmetry);
   appendLayer(&m_dipoleLayer);
   setConfigurator(&m_configurator);
}


void Info::setMolecule(Molecule* molecule)
{
   m_molecule = molecule;

   if (m_molecule) {
      connect(m_molecule, SIGNAL(pointGroupAvailable(Data::PointGroup const&)), 
         this, SLOT(setPointGroup(Data::PointGroup const&)));

      connect(m_molecule, SIGNAL(energyAvailable(double const, Info::EnergyUnit)), 
         this, SLOT(setEnergy(double const, Info::EnergyUnit)));

      connect(m_molecule, SIGNAL(dipoleAvailable(qglviewer::Vec const&, bool const)), 
         this, SLOT(setDipole(qglviewer::Vec const&, bool const)));

      connect(m_molecule, SIGNAL(chargeAvailable(int)), 
         this, SLOT(setCharge(int)));

      connect(m_molecule, SIGNAL(multiplicityAvailable(unsigned)), 
         this, SLOT(setMultiplicity(unsigned)));

      connect(m_molecule, SIGNAL(dipoleAvailable(qglviewer::Vec const&, bool const)), 
         this, SLOT(setDipole(qglviewer::Vec const&, bool const)));

      connect(this, SIGNAL(updated()), &m_symmetry, SLOT(update()));

      m_dipoleLayer.setMolecule(m_molecule);
   }
}


void Info::addAtoms(AtomList const& atomList)
{
   m_suspendUpdate = true;
   AtomList::const_iterator atom;
   for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
       addAtom(*atom);
   }
   m_suspendUpdate = false;
   updated();
}


void Info::removeAtoms(AtomList const& atomList)
{
   m_suspendUpdate = true;
   AtomList::const_iterator atom;
   for (atom = atomList.begin(); atom != atomList.end(); ++atom) {
       removeAtom(*atom);
   }
   m_suspendUpdate = false;
   updated();
}


void Info::clear()
{
   m_charge        = 0;
   m_energy        = 0.0;
   m_mass          = 0.0;
   m_numberOfAtoms = 0;
   m_nuclearCharge = 0;
   m_multiplicity  = 1;
   m_energyUnit    = Hartree;
   m_formula.clear();
   m_pointGroup.clear();
   setDipoleValid(false);
   m_suspendUpdate = false;
}


//! This assumes the atom we are adding is neutral
void Info::addAtom(Atom const* atom)
{
   QString s(atom->getAtomicSymbol());
   double m(atom->getMass());
   int z(atom->getAtomicNumber());

   m_numberOfAtoms  += 1;
   m_nuclearCharge  += z;
   m_mass           += m;
   m_formula[s]     += 1;
   m_energy          = 0.0;
   m_pointGroup.clear();
   setDipoleValid(false);

   // This expression favours low-spin systems
   if (Util::isOdd(z)) m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
   if (!m_suspendUpdate) updated();
}


//! This assumes the atom we are removing is neutral
void Info::removeAtom(Atom const* atom)
{
   QString s(atom->getAtomicSymbol());
   double m(atom->getMass());
   int z(atom->getAtomicNumber());

   m_numberOfAtoms  -= 1;
   m_nuclearCharge  -= z;
   m_mass           -= m;
   m_formula[s]     -= 1;
   m_energy          = 0.0;
   m_pointGroup.clear();
   setDipoleValid(false);

   // This expression favours low-spin systems
   if (Util::isOdd(z)) m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
   if (!m_suspendUpdate) updated();
}


QString Info::formula() const 
{
   QStringList elements(m_formula.keys());
   qSort(elements.begin(), elements.end());
   elements.removeAll("C");
   elements.removeAll("H");

   QString formula;
   int n;

   n = m_formula["C"];
   if (n > 0) {
      formula += "C";
      if (n>1) formula += Util::subscript(QString::number(n));
   }

   n = m_formula["H"];
   if (n > 0) {
      formula += "H";
      if (n>1) formula += Util::subscript(QString::number(n));
   }

   QStringList::iterator iter;
   for (iter = elements.begin(); iter != elements.end(); ++iter) {
       n = m_formula[*iter];
       if (n > 0) {
          formula += *iter;
          if (n>1) formula += Util::subscript(QString::number(n));
       }
   }

   return formula;
}


void Info::setCharge(int const charge)
{
   m_charge = charge;

   if ( Util::isEven(numberOfElectrons()) ) {
      if (Util::isEven(m_multiplicity)) {
         m_multiplicity -= 1;
      }
   } else {                        
      if (Util::isOdd(m_multiplicity)) {
         m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
      }
   }
   
   updated();
}


void Info::setMultiplicity(unsigned const multiplicity)
{
   m_multiplicity = multiplicity;
   int n(numberOfElectrons());

   if (Util::isEven(m_multiplicity)) {
      if (Util::isEven(n)) { 
         m_charge += 1;
      }
   } else {                                
      if (Util::isOdd(n)) {
         m_charge += (n == 1) ? -1 : 1;
      }
   }

   updated();
}


unsigned Info::numberOfElectrons() const
{ 
   return m_nuclearCharge - m_charge;
}


unsigned Info::numberOfAlphaElectrons() const
{ 
   return (numberOfElectrons() + m_multiplicity - 1) / 2;
}


unsigned Info::numberOfBetaElectrons() const
{ 
   return (numberOfElectrons() - m_multiplicity + 1) / 2;
}


void Info::setEnergy(double const energy, EnergyUnit unit) 
{
   m_energy = energy; 
   m_energyUnit = unit;
   updated();
}


void Info::setDipole(qglviewer::Vec const& value, bool const estimated) 
{ 
   setDipoleValid(true);
   m_dipoleValue = value.norm();
   m_dipoleLayer.setValue(value);
   m_dipoleEstimated = estimated;
   updated();
}


void Info::setDipoleValid(bool tf)
{
   QList<Dipole*> list(findLayers<Dipole>(Children));
   if (!list.isEmpty()) {
      Dipole* dipole(list.first());
      if (!tf) dipole->setCheckState(Qt::Unchecked);
      dipole->setEnabled(tf);
   }
}


void Info::setPointGroup(Data::PointGroup const& pointGroup) 
{ 
   m_pointGroup = pointGroup;
   updated();
}


void Info::detectSymmetry()
{
   if (m_molecule) m_molecule->detectSymmetry();
}

} } // end namespace IQmol::Layer
