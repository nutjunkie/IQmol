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

#include "Atom.h"
#include "AtomicProperty.h"
#include <openbabel/mol.h>
#include <QDebug>


namespace IQmol {
namespace Data {

template<> const Type::ID AtomList::TypeID = Type::AtomList;


int Atom::atomicNumber(QString const& symbol) 
{
   int z(OpenBabel::etab.GetAtomicNum(symbol.toLatin1().data()));
   // we return -1 for invalid atomic symbols
   return (z == 0 ? -1 : z);
}


Atom::Atom(QString const& symbol)
{
   m_atomicNumber = atomicNumber(symbol);
}


void Atom::dump() const
{
   qDebug() << "Z =" << m_atomicNumber;
   m_properties.dump();
}

} } // end namespace IQmol::Data
