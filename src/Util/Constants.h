#ifndef IQMOL_UTIL_CONSTANTS_H
#define IQMOL_UTIL_CONSTANTS_H
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


namespace IQmol {
namespace Constants {

   // Values taken from the source of all knowledge: Wikipedia

   double const BohrRadius          = 5.2917721092e-11;
   double const BohrToAngstrom      = BohrRadius*1.0e10;
   double const AngstromToBohr      = 1.0/BohrToAngstrom;

   double const Mole                = 6.02214129e23;

   double const JoulesPerCalorie    = 4.184;
   double const HartreeToEv         = 27.21138386;
   double const HartreeToWavenumber = 219474.6314;
   double const HartreeToMHz        = 6.579683921e9;
   double const HartreeToKJmol      = 2625.49962;
   double const HartreeToKCalmol    = HartreeToKJmol/JoulesPerCalorie;

   enum Units { Hartree, ElectronVolt, Wavenumber, KJoulePerMol, KCalPerlMol, MegaHertz };

} } // end namespace IQmol::Constants

#endif
