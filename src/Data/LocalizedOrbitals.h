#ifndef IQMOL_DATA_LOCALIZEDORBITALS_H
#define IQMOL_DATA_LOCALIZEDORBITALS_H
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

#include "Orbitals.h"


namespace IQmol {
namespace Data {

   class LocalizedOrbitals : public Orbitals {

      friend class boost::serialization::access;

      public:
         Type::ID typeID() const { return Type::LocalizedOrbitals; }

         LocalizedOrbitals() : Orbitals(), m_nAlpha(0), m_nBeta(0) { }

         LocalizedOrbitals(
            unsigned const nAlpha, 
            unsigned const nBeta, 
            ShellList const& shellList,
            QList<double> const& alphaCoefficients, 
            QList<double> const& betaCoefficients, 
            QString const& label) 
          : Orbitals(Orbitals::Localized, shellList, 
            alphaCoefficients, betaCoefficients, label), 
            m_nAlpha(nAlpha), m_nBeta(nBeta) { }
              

         unsigned nAlpha() const { return m_nAlpha; }
         unsigned nBeta()  const { return m_nBeta;  }

         QString label(unsigned index, bool alpha = true) const
         {
             QString s(QString::number(index+1));
             unsigned n(alpha ? m_nAlpha : m_nBeta);
             if (index < n)  s += " (occ)";
             return s;
         }

      private:
         unsigned m_nAlpha;
         unsigned m_nBeta;
   };

} } // end namespace IQmol::Data

#endif
