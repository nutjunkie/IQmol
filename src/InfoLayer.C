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

#include "InfoLayer.h"
#include "MoleculeLayer.h"
#include "IQmol.h"


using namespace qglviewer;

namespace IQmol {
namespace Layer {

Info::Info(Molecule* molecule) : m_dipoleEstimated(true), m_configurator(this)
{
   clear();
   setText("Info");
   setMolecule(molecule);
   appendRow(&m_dipole);
   setConfigurator(&m_configurator);
}


void Info::setMolecule(Molecule* molecule)
{
   m_molecule = molecule;
   if (m_molecule) {
      connect(m_molecule, SIGNAL(pointGroupAvailable(QString const&)), 
         this, SLOT(setPointGroup(QString const&)));
      connect(m_molecule, SIGNAL(energyAvailable(double const, Info::EnergyUnit)), 
         this, SLOT(setEnergy(double const, Info::EnergyUnit)));
      connect(m_molecule, SIGNAL(dipoleAvailable(qglviewer::Vec const&, bool const)), 
         this, SLOT(setDipole(qglviewer::Vec const&, bool const)));

      connect(m_molecule, SIGNAL(centerOfChargeAvailable(qglviewer::Vec const&)), 
         &m_dipole, SLOT(setPosition(qglviewer::Vec const&)));
      connect(m_molecule, SIGNAL(radiusAvailable(double const&)), 
         &m_dipole, SLOT(setRadius(double const&)));

      connect(&m_dipole, SIGNAL(updated()), m_molecule, SIGNAL(softUpdate()));

      m_dipole.setPosition(m_molecule->centerOfNuclearCharge());
      m_dipole.setRadius(m_molecule->radius());
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
   m_suspendUpdate   = false;
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
   if (isOdd(z)) m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
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
   m_pointGroup      = "";
   m_energy          = 0.0;
   m_pointGroup.clear();
   setDipoleValid(false);

   // This expression favours low-spin systems
   if (isOdd(z)) m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
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
      if (n>1) formula += subscript(QString::number(n));
   }

   n = m_formula["H"];
   if (n > 0) {
      formula += "H";
      if (n>1) formula += subscript(QString::number(n));
   }

   QStringList::iterator iter;
   for (iter = elements.begin(); iter != elements.end(); ++iter) {
       n = m_formula[*iter];
       if (n > 0) {
          formula += *iter;
          if (n>1) formula += subscript(QString::number(n));
       }
   }

   return formula;
}


void Info::setCharge(int const charge)
{
   m_charge = charge;

   if ( isEven(numberOfElectrons()) ) {
      if (isEven(m_multiplicity)) {
         m_multiplicity -= 1;
      }
   } else {                        
      if (isOdd(m_multiplicity)) {
         m_multiplicity += (m_multiplicity == 1) ? 1 : -1;
      }
   }
   
   updated();
}


void Info::setMultiplicity(unsigned int const multiplicity)
{
   m_multiplicity = multiplicity;
   int n(numberOfElectrons());

   if (isEven(m_multiplicity)) {
      if (isEven(n)) { 
         m_charge += 1;
      }
   } else {                                
      if (isOdd(n)) {
         m_charge += (n == 1) ? -1 : 1;
      }
   }

   updated();
}


unsigned int Info::numberOfElectrons() const
{ 
   return m_nuclearCharge - m_charge;
}


unsigned int Info::numberOfAlphaElectrons() const
{ 
   return (numberOfElectrons() + m_multiplicity - 1) / 2;
}


unsigned int Info::numberOfBetaElectrons() const
{ 
   return (numberOfElectrons() - m_multiplicity + 1) / 2;
}


void Info::setEnergy(double const energy, EnergyUnit unit) 
{
   m_energy = energy; 
   m_energyUnit = unit;
   updated();
}


void Info::setDipole(Vec const& value, bool const estimated) 
{ 
   setDipoleValid(true);
   m_dipole.setValue(value);
   m_dipoleEstimated = estimated;
   updated();
}


void Info::setDipoleValid(bool tf)
{
   if (!tf) m_dipole.setCheckState(Qt::Unchecked);
   m_dipole.setEnabled(tf);
}


void Info::setPointGroup(QString const& pointGroup) 
{ 
   m_pointGroup = pointGroup;
   updated();
}


void Info::detectSymmetry()
{
   if (m_molecule) m_molecule->detectSymmetry();
}


} } // end namespace IQmol::Layer
