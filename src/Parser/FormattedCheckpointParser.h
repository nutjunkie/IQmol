#ifndef IQMOL_PARSER_FORMATTEDCHECKPOINT_H
#define IQMOL_PARSER_FORMATTEDCHECKPOINT_H
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

#include "Parser.h"
#include "Shell.h"
#include "Density.h"
#include "Geometry.h"
#include "Orbitals.h"
#include "ExcitedStates.h"


namespace IQmol {

namespace Data {
   class GeminalOrbitals;
}

namespace Parser {

   class FormattedCheckpoint : public Base {

      public:
         bool parse(TextStream&);

      private:
         QList<int> readIntegerArray(TextStream&, unsigned nTokens);
         QList<double> readDoubleArray(TextStream&, unsigned nTokens);
         QList<unsigned> readUnsignedArray(TextStream&, unsigned nTokens);

         struct GeomData {
               QList<unsigned> atomicNumbers;
               QList<double> coordinates;
               int charge;
               unsigned multiplicity;
         };

         Data::Geometry* makeGeometry(GeomData const&);

         struct ShellData {
               QList<int>      shellTypes;
               QList<unsigned> shellToAtom;
               QList<unsigned> shellPrimitives;
               QList<double>   exponents;
               QList<double>   contractionCoefficients;
               QList<double>   contractionCoefficientsSP;
         };

         bool dataAreConsistent(ShellData const&, unsigned const nAtoms);
         Data::ShellList* makeShellList(ShellData const&, Data::Geometry const& geometry);

         struct OrbitalData {
            Data::Orbitals::OrbitalType orbitalType;
            int stateIndex;
            QString label;

            QList<double> alphaCoefficients;
            QList<double> betaCoefficients;
            QList<double> alphaEnergies;
            QList<double> betaEnergies;
         };

         void clear(OrbitalData&);
         Data::Orbitals* makeOrbitals(unsigned const nAlpha, unsigned const nBeta,
            OrbitalData const&, ShellData const&, Data::Geometry const&,
            Data::DensityList densities = Data::DensityList()); 


// DEPRECATE
/*
         struct MoData {
            QString  label;
            unsigned nAlpha;
            unsigned nBeta;
            unsigned nBasis;
            QList<double> alphaCoefficients;
            QList<double> betaCoefficients;
            QList<double> alphaEnergies;
            QList<double> betaEnergies;
	    
            int stateNumber;
            QString stateTag;
            Data::Orbitals::OrbitalType orbitalType;
         };

         void clear(MoData&);
         Data::MolecularOrbitals* makeMolecularOrbitals(unsigned const nAlpha, 
            unsigned const nBeta, MoData const&, ShellData const&, Data::Geometry const&);
*/
// END DEPRECATE

         struct GmoData {
            QList<double> alphaCoefficients;
            QList<double> betaCoefficients;
            QList<double> geminalEnergies;
            QList<double> geminalCoefficients;
            QList<int>    geminalMoMap;
         };

         void clear(GmoData&);
         Data::GeminalOrbitals* makeGeminalOrbitals(unsigned const nAlpha, 
            unsigned const nBeta, GmoData const&, ShellData const&, Data::Geometry const&);
            
         struct ExtData {
           unsigned nState;
           Data::ExcitedStates::ExcitedStatesT extType;
           QList<double> excitationEnergies;
           QList<double> oscillatorStrengths;
           QList<double> alphaAmplitudes;
           QList<double> alphaYAmplitudes;
           QList<double> betaAmplitudes;
           QList<double> betaYAmplitudes;
           QList<int>    alphaSparseJ;
           QList<int>    alphaSparseI;
           QList<int>    betaSparseJ;
           QList<int>    betaSparseI;
         };

         void clear(ExtData&);
         bool installExcitedStates(unsigned const nAlpha, unsigned const nBeta, 
            ExtData &extData, OrbitalData const&);
   };

} } // end namespace IQmol::Parser

#endif
