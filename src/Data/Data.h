#ifndef IQMOL_DATA_H
#define IQMOL_DATA_H
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

#include "Serialization.h"
#include <boost/serialization/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>


/// The Data namespace includes POD classes that can be serialized.  The
/// Data::Base class herein takes care of this serialization and also
/// implements a generic copy behavior.  New Data classes must have an
/// identifier added to the ID enum below and be added to the DataFactory 
/// ensure the serialization process can occur.

namespace IQmol {
namespace Data {

   typedef boost::archive::text_iarchive InputArchive;
   typedef boost::archive::text_oarchive OutputArchive;

   namespace Type {
      enum ID { Undefined = 0, 
               /*---------------------  *---------------------  *--------------------- */
                Atom,                   AtomList,               Bank, 
                PointCharge,            PointChargeList,        EfpFragment, 
                EfpFragmentList,        EfpFragmentLibrary,     GridData, 
                GridDataList,           File,                   FileList, 
                Geometry,               GeometryList,           RemSection,
                ChargeMultiplicity,     CubeData,
                Energy,                 ConformerEnergy,        Hessian,
                ScfEnergy,              TotalEnergy,            ForceFieldEnergy,
                DipoleMoment,           VibrationalMode,        VibrationalModeList, 
                PointGroup,
                Frequencies,            MolecularOrbitals,      MolecularOrbitalsList,
                Orbitals,               OrbitalsList,           LocalizedOrbitals, 
                CanonicalOrbitals, 
                Density,                DensityList,
                Shell,                  ShellList,              Mesh,
                MeshList,               Surface,                SurfaceList,
                SurfaceInfo,            SurfaceInfoList,        SurfaceType, 
               /*---------------------  *---------------------  *--------------------- */
                Constraint,             PositionConstraint,     DistanceConstraint,
                AngleConstraint,        TorsionConstraint,
               /*---------------------  *---------------------  *--------------------- */
                AtomicProperty,         AtomicSymbol,           AtomColor, 
                AtomicNumber,
                NmrShielding,           NmrShift,               Mass, 
                MultipoleExpansion,     MullikenCharge,         MultipoleDerivedCharge, 
                AtomicCharge,           GasteigerCharge,        ChelpgCharge,
                SpinDensity,            VdwRadius,              MultipoleExpansionList,
                NmrReference,           NmrReferenceList,       Nmr,
                ElectronicTransition,   ElectronicTransitionList, ExcitedStates,
                OrbitalSymmetries,
               /*---------------------  *---------------------  *--------------------- */
                YamlNode,               PovRay,                 GeminalOrbitals
      };

      QString toString(ID const);
   }

   /// Base class for all data classes that can be serialized.
   class Base {

      public:
         Base() { }
         Base(Base const& that) { copy(that); }
         virtual ~Base() { destroy(); }

         Base& operator=(Base const& that) {
            if (this != &that) copy(that);  return *this;
         }

		 /// Returns a portable type identification for the Data::Factory.  New
		 /// Data classes must have their TypeID added to the Factory::create()
		 /// method before they can be used by the Factory.
		 virtual Type::ID typeID() const { return Type::Undefined; }

		 /// This can't be a template function as templates and virtual
		 /// functions don't play nicely together.
         virtual void serialize(InputArchive&,  unsigned int const version = 0) = 0;
         virtual void serialize(OutputArchive&, unsigned int const version = 0) = 0;

         /// This is only meant for debugging;
		 virtual void dump() const = 0;

      protected:
         /// This should delete any resources owned by the data object.
         virtual void destroy() { }

		 /// The Base implementation is very general, but inefficient as it
		 /// relies on serialization for the copy.  Over-ride if efficiency 
         /// matters.
         virtual void copy(Base const& that);
   };

} } // end namespace IQmol::Data

#endif
