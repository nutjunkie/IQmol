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

#include "ChargeMultiplicity.h"
#include <QDebug>


namespace IQmol {
namespace Data {

void ChargeMultiplicity::dump() const
{
   qDebug() << "Charge:" << m_charge 
            << "Electrons" << m_electrons 
            <<"Multiplicity:" << m_multiplicity;
}


void ChargeMultiplicity::setValues(int const charge, unsigned const electrons,
   unsigned const multiplicity)
{
   setCharge(charge);
   setElectrons(electrons);
   setMultiplicity(multiplicity);
}


void ChargeMultiplicity::setCharge(int const charge)
{
   m_charge = charge;
}


void ChargeMultiplicity::setElectrons(unsigned const electrons)
{
   m_electrons = electrons;
}


void ChargeMultiplicity::setMultiplicity(unsigned const multiplicity)
{
   m_multiplicity = multiplicity;
}


} } // end namespace IQmol::Data
