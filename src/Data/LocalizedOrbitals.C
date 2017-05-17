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

#include "LocalizedOrbitals.h"
#include "QsLog.h"
#include <QDebug>


namespace IQmol {
namespace Data {


QString LocalizedOrbitals::toString(LocalizationMethod const method)
{
   QString s;

   switch (method) {
      case Undefined:           s = "Undefined";   break;
      case Boys:                s = "Boys";        break;
      case EdmistonRuedenberg:  s = "ER";          break;
      case PipekMezey:          s = "PipekMezey";  break;
   }

   return s;
}


LocalizedOrbitals::LocalizedOrbitals(
   unsigned const nAlpha, 
   unsigned const nBeta, 
   ShellList const& shells,
   QList<double> const& alphaCoefficients, 
   QList<double> const& betaCoefficients,
   LocalizationMethod const method,
   bool const restricted)
 : Orbitals(nAlpha, nBeta, shells, alphaCoefficients, betaCoefficients, restricted), 
   m_localizationMethod(method)
{
   QString s("Localized Orbitals (");
   s += toString(m_localizationMethod) + ")";
   setLabel(s);
}

} } // end namespace IQmol::Data
