#ifndef IQMOL_LAYER_INFO_H
#define IQMOL_LAYER_INFO_H
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

#include "InfoConfigurator.h"
#include "AtomLayer.h"
#include "DipoleLayer.h"
#include "SymmetryLayer.h"
#include "PointGroup.h"
#include <QMap>
#include <QRect>


namespace IQmol {
namespace Layer {


   // Container class for information about the whole molecule such as
   // number of atoms, charge, multiplicity etc.
   class Info : public Base {

      Q_OBJECT

      friend class Configurator::Info;

      public:
         enum EnergyUnit { Hartree, KJMol, KCalMol };

         explicit Info(Molecule* molecule = 0);

         void setMolecule(Molecule*);
         void addAtom(Atom const*);
         void addAtoms(AtomList const&);
         void removeAtom(Atom const*);
         void removeAtoms(AtomList const&);
         void clear();

         int  getCharge() const { return m_charge; }
         unsigned getMultiplicity() const { return m_multiplicity; }
         unsigned numberOfElectrons() const;

      public Q_SLOTS:
         void setCharge(int const charge);
         void setMultiplicity(unsigned const multiplicity);
         void setEnergy(double const energy, Info::EnergyUnit unit);
         void setDipole(qglviewer::Vec const& dipole, bool const estimated = false);
         void setPointGroup(Data::PointGroup const&);

      protected:
         unsigned numberOfAlphaElectrons() const;
         unsigned numberOfBetaElectrons() const;
         QString formula() const;
         void detectSymmetry();

         int    m_charge;
         double m_energy;
         double m_mass;
         double m_dipoleValue;

         unsigned m_numberOfAtoms;
         unsigned m_nuclearCharge;
         unsigned m_multiplicity;

         EnergyUnit m_energyUnit;
         QMap<QString, int> m_formula;
         Data::PointGroup m_pointGroup;

         bool m_dipoleEstimated;
         bool m_suspendUpdate;

      private:
         void setDipoleValid(bool);
         Configurator::Info m_configurator;
         Dipole m_dipoleLayer;
         Symmetry m_symmetry;
   };

} } // end namespace IQmol::Layer

#endif
