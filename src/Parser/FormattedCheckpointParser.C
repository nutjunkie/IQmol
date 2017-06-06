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

#include "FormattedCheckpointParser.h"
//#include "MolecularOrbitalsList.h"
#include "CanonicalOrbitals.h"
#include "GeminalOrbitals.h"
#include "DipoleMoment.h"
#include "GeometryList.h"
#include "OrbitalsList.h"
#include "TextStream.h"
#include "Constants.h"
#include "Hessian.h"
#include "Energy.h"
#include "QsLog.h"
#include "Data.h"
#include <cmath>
#include <QtDebug>
#include "Spin.h"
#include "ExcitedStates.h"
#include "Constants.h"

namespace IQmol {
namespace Parser {

bool FormattedCheckpoint::parse(TextStream& textStream)
{
   Data::GeometryList* geometryList(new Data::GeometryList);
   Data::Geometry* geometry(0);

   Data::OrbitalsList* orbitalsList(new Data::OrbitalsList()); 

   OrbitalData hfData;
   hfData.orbitalType = Data::Orbitals::Canonical;
   hfData.label = "Canonical Orbitals";

   OrbitalData erData;
   erData.orbitalType = Data::Orbitals::Localized;
   erData.label = "Localized MOs (ER)";

   OrbitalData boysData;
   boysData.orbitalType = Data::Orbitals::Localized;
   boysData.label = "Localized MOs (Boys)";

   OrbitalData ntoData;
   ntoData.orbitalType = Data::Orbitals::NaturalTransition;
   ntoData.label = "Natural Transition Orbitals";

   OrbitalData nboData;
   nboData.orbitalType = Data::Orbitals::NaturalBond;
   nboData.label = "Natural Bond Orbitals";

   unsigned nAlpha(0);
   unsigned nBeta(0);
   bool     ok(true);

   GeomData  geomData;
   ShellData shellData;
   GmoData   gmoData;
   ExtData   extData;   
   extData.nState = 0;

   Data::DensityList densityList;

// DEPRECATE
/*
   Data::MolecularOrbitalsList* 
      molecularOrbitalsList(new Data::MolecularOrbitalsList()); 
   Data::MolecularOrbitalsList* 
      naturaltransOrbitalList(new Data::MolecularOrbitalsList()); 
   Data::MolecularOrbitalsList* 
      naturalbondOrbitalList(new Data::MolecularOrbitalsList()); 
*/
// END DEPRECATE
   QString key;

   while (!textStream.atEnd()) {

      QString line(textStream.nextLine());
      key = line;
      key.resize(42);
      key = key.trimmed();
      QString tmp(line.mid(43, 37));


      QStringList list(TextStream::tokenize(tmp));

      if (key == "Number of alpha electrons") {            // This should only appear once
         nAlpha = list.at(1).toInt(&ok);
         if (!ok) goto error;

      }else if (key == "Number of beta electrons") {       // This should only appear once
         nBeta = list.at(1).toInt(&ok);
         if (!ok) goto error;

      }else if (key == "Multiplicity") { 
         geomData.multiplicity = list.at(1).toUInt(&ok);  
         if (!ok) goto error;

      }else if (key == "Charge") { 
         geomData.charge = list.at(1).toInt(&ok);  
         if (!ok) goto error;

      }else if (key == "Atomic numbers") {                 // This should only appear once
         unsigned n(list.at(2).toUInt(&ok));  
         if (!ok) goto error;
         geomData.atomicNumbers = readUnsignedArray(textStream, n);

      }else if (key == "Current cartesian coordinates") { // This triggers a new geometry

         if (geometry) {
            // We are on a subsequent geometry, so we should have everything we 
            // need to create the MOs for the previous one.
            Data::Orbitals* orbitals(makeOrbitals(nAlpha, nBeta, hfData, shellData,
               *geometry, densityList));
            if (orbitals) orbitalsList->append(orbitals);
            densityList.clear();
            clear(hfData);
         }

         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         geomData.coordinates = readDoubleArray(textStream, n);
         geometry = makeGeometry(geomData);
         if (!geometry) goto error;
         geometryList->append(geometry);

      }else if (key == "SCF Iteration") {
         unsigned n(list.at(1).toUInt(&ok));
         if (!ok) goto error;
         hfData.label = "SCF Iter " +  QString::number(n-1);

      }else if (key == "SCF Iteration Energy") {
         double energy(list.at(1).toDouble(&ok));
         if (!ok) goto error;
         hfData.label += " (" + QString::number(energy, 'f', 6) + ")";

         if (geometry) {
            // The user may have requested orbitals for each SCF cycle, 
            Data::Orbitals* orbitals(makeOrbitals(nAlpha, nBeta, hfData, shellData,
               *geometry, densityList));
            if (orbitals) orbitalsList->append(orbitals);
         }

      }else if (key == "Number of basis functions") {
         // This is determined from the shell data

      }else if (key == "Shell types") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         shellData.shellTypes = readIntegerArray(textStream, n);
         
      }else if (key == "Number of primitives per shell") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         shellData.shellPrimitives = readUnsignedArray(textStream, n);

      }else if (key == "Shell to atom map") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         shellData.shellToAtom = readUnsignedArray(textStream, n);

      }else if (key == "Primitive exponents") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         shellData.exponents = readDoubleArray(textStream, n);

      }else if (key == "Contraction coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         shellData.contractionCoefficients = readDoubleArray(textStream, n);

      }else if (key == "P(S=P) Contraction coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         shellData.contractionCoefficientsSP = readDoubleArray(textStream, n);

      }else if (key == "SCF Energy") {
         double energy(list.at(1).toDouble(&ok));
         if (!ok || !geometry) goto error;
         Data::ScfEnergy& scf(geometry->getProperty<Data::ScfEnergy>());
         scf.setValue(energy, Data::Energy::Hartree);
         Data::TotalEnergy& total(geometry->getProperty<Data::TotalEnergy>());
         total.setValue(energy, Data::Energy::Hartree);

      }else if (key == "Total Energy") {
         double energy(list.at(1).toDouble(&ok));
         if (!ok || !geometry) goto error;
         Data::TotalEnergy& total(geometry->getProperty<Data::TotalEnergy>());
         total.setValue(energy, Data::Energy::Hartree);

      }else if (key == "Dipole_Data") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         QList<double> data(readDoubleArray(textStream, n));
         if (data.size() != 3) goto error;
         Data::DipoleMoment& dipole(geometry->getProperty<Data::DipoleMoment>());
         dipole.setValue(data[0],data[1],data[2]);

      }else if (key == "Cartesian Force Constants") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         QList<double> data(readDoubleArray(textStream, n));
         Data::Hessian& hessian(geometry->getProperty<Data::Hessian>());
         hessian.setData(geometry->nAtoms(), data);

      // Canonical Orbitals

      }else if (key == "Alpha MO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         hfData.alphaCoefficients = readDoubleArray(textStream, n);

	  }else if (key == "Beta MO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         hfData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Alpha Orbital Energies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         hfData.alphaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Beta Orbital Energies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         hfData.betaEnergies = readDoubleArray(textStream, n);

      // Natural Transition Orbitals

	  }else if (key == "Alpha NTO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         ntoData.alphaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Beta NTO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         ntoData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Alpha NTO amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         ntoData.alphaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Beta NTO amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         ntoData.betaEnergies = readDoubleArray(textStream, n);

      // Natural Bond Orbitals

	  }else if (key == "Alpha NBO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         nboData.alphaCoefficients = readDoubleArray(textStream, n);

	  }else if (key == "Beta NBO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         nboData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Alpha NBO occupancies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         nboData.alphaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Beta NBO occupancies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         nboData.betaEnergies = readDoubleArray(textStream, n);

      // Localized Orbitals

      }else if (key == "Localized Alpha MO Coefficients (ER)") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         erData.alphaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Beta  MO Coefficients (ER)") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         erData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Alpha MO Coefficients (Boys)") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         boysData.alphaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Beta  MO Coefficients (Boys)") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         boysData.betaCoefficients = readDoubleArray(textStream, n);

      // Geminals

      }else if (key == "Alpha GMO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         gmoData.alphaCoefficients = readDoubleArray(textStream, n);
         gmoData.betaCoefficients  = gmoData.alphaCoefficients;

      }else if (key == "Beta GMO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         gmoData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "MO to geminal map") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         gmoData.geminalMoMap = readIntegerArray(textStream, n);

      }else if (key == "Geminal Coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         gmoData.geminalCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Energies of Geminals") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         gmoData.geminalEnergies = readDoubleArray(textStream, n);

      }else if (key.contains("Density")) {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         QList<double> data(readDoubleArray(textStream, n));
         Data::SurfaceType type(Data::SurfaceType::Custom);
         type.setLabel(key);
         Data::Density* density(new Data::Density(type, data, key));
         density->dump();
         densityList.append(density);

      }else if (key.endsWith("Surface Title") || key == "NBO Ground State" ) {
         unsigned n(list.at(1).toUInt(&ok));
         if (!ok || !geometry) goto error;

         key.replace("NBO ","");
         key.replace("Surface Title","");
  	     ntoData.label = key + " State: " + QString::number(n);
  	     nboData.label = key + " State: " + QString::number(n);
    
         Data::Orbitals* ntos(makeOrbitals(nAlpha, nBeta, ntoData, 
            shellData, *geometry));   // returns 0 if no alpha coefficients exist
         if (ntos) {
            orbitalsList->append(ntos);
         }
         clear(ntoData);

         Data::Orbitals* nbos(makeOrbitals(nAlpha, nBeta, nboData, 
            shellData, *geometry));   // returns 0 if no alpha coefficients exist
         if (nbos) {
            orbitalsList->append(nbos);
         }
         clear(nboData);

      // Parsing CIS/TDDFT data
      //
      //}else if (key == "Number of Excited States") {
      //   extData.nState  = list.at(1).toInt(&ok);
      //   if (!ok) goto error;

      }else if (key.endsWith("Excitation Energies")) {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.excitationEnergies = readDoubleArray(textStream, n);
         extData.nState = n;
         extData.extType = key.contains("EOMEE") ? Data::ExcitedStates::EOM
                                                 : Data::ExcitedStates::CIS;
      }else if (key == "Oscillator Strengths") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.oscillatorStrengths = readDoubleArray(textStream, n);

      }else if (key == "Alpha Amplitudes" || key == "Alpha X Amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.alphaAmplitudes = readDoubleArray(textStream, n);
      
      }else if (key == "Alpha Y Amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.alphaYAmplitudes = readDoubleArray(textStream, n);
         extData.extType = Data::ExcitedStates::TDDFT;

      }else if (key == "Beta Amplitudes" || key == "Beta X Amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.betaAmplitudes = readDoubleArray(textStream, n);

      }else if (key == "Beta Y Amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.betaYAmplitudes = readDoubleArray(textStream, n);

      }else if (key == "Alpha J Indexes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.alphaSparseJ = readIntegerArray(textStream, n);

      }else if (key == "Alpha I Indexes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.alphaSparseI = readIntegerArray(textStream, n);

      }else if (key == "Beta J Indexes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.betaSparseJ = readIntegerArray(textStream, n);

      }else if (key == "Beta I Indexes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         extData.betaSparseI = readIntegerArray(textStream, n);

      }

   } // end of parsing text stream 


   if (geometry) {
      Data::Orbitals* orbitals(0);

      // We append the additional densities to the canonical orbitals 
      hfData.label = "Canonical Orbitals";
      orbitals = makeOrbitals(nAlpha, nBeta, hfData, shellData, *geometry, densityList); 
      if (orbitals) {
         orbitalsList->append(orbitals);
         if (extData.nState > 0) {  // install excitation 
            ok = installExcitedStates(nAlpha, nBeta, extData, hfData);
            if (!ok) goto error;
         }
      }
      densityList.clear();
         
      orbitals = makeOrbitals(nAlpha, nBeta, erData, shellData, *geometry); 
      if (orbitals) orbitalsList->append(orbitals);

      orbitals = makeOrbitals(nAlpha, nBeta, boysData, shellData, *geometry); 
      if (orbitals) orbitalsList->append(orbitals);

      Data::GeminalOrbitals* gmos(makeGeminalOrbitals(nAlpha, nBeta, gmoData, 
         shellData, *geometry)); 
      if (gmos) m_dataBank.append(gmos);
   }

   if (geometryList->isEmpty()) {
      delete geometryList;
   }else {
      geometryList->setDefaultIndex(-1);
      m_dataBank.append(geometryList);
   }

   if (orbitalsList->isEmpty()) {
      delete orbitalsList;
   }else {
      m_dataBank.append(orbitalsList);    
   }


// DEPRECATE   
/*
   if (molecularOrbitalsList) {
      if (molecularOrbitalsList->isEmpty()) {
         delete molecularOrbitalsList;
      }else {
         molecularOrbitalsList->setDefaultIndex(-1);
         m_dataBank.append(molecularOrbitalsList);
      }
   }
   
   if (naturaltransOrbitalList) {
      if (naturaltransOrbitalList->isEmpty()) {
         delete naturaltransOrbitalList;
      }else {
         naturaltransOrbitalList->setDefaultIndex(-1);
         m_dataBank.append(naturaltransOrbitalList);
      }
   }
   
   if (naturalbondOrbitalList) {
      if (naturalbondOrbitalList->isEmpty()) {
         delete naturalbondOrbitalList;
      }else {
         naturalbondOrbitalList->setDefaultIndex(-1);
         m_dataBank.append(naturalbondOrbitalList);
      }
   }
*/
// END DEPRECATE   

   return m_errors.isEmpty();
 
   error:
      QString msg("Error in data section '");
      msg += key + "' around line number ";
      msg += QString::number(textStream.lineNumber());
      m_errors.append(msg);

   delete geometryList;
   delete orbitalsList;

   return false;
}


void FormattedCheckpoint::clear(OrbitalData& orbitalData)
{
   orbitalData.label.clear();
   orbitalData.alphaCoefficients.clear();
   orbitalData.betaCoefficients.clear();
   orbitalData.alphaEnergies.clear();
   orbitalData.betaEnergies.clear();
   orbitalData.stateIndex = 0;
}


void FormattedCheckpoint::clear(GmoData& gmoData)
{
   gmoData.alphaCoefficients.clear();
   gmoData.betaCoefficients.clear();
   gmoData.geminalEnergies.clear();
   gmoData.geminalCoefficients.clear();
   gmoData.geminalMoMap.clear();
}


Data::Geometry* FormattedCheckpoint::makeGeometry(GeomData const& geomData)
{
   unsigned nAtoms(geomData.atomicNumbers.size());
   Data::Geometry* geom(0);
   if (nAtoms > 0 && 3*nAtoms == (unsigned)geomData.coordinates.size()) {
      geom = new Data::Geometry(geomData.atomicNumbers, geomData.coordinates);
      geom->setChargeAndMultiplicity(geomData.charge, geomData.multiplicity);
      geom->scaleCoordinates(Constants::BohrToAngstrom);
      geom->computeGasteigerCharges();
   }
   return geom;
}


bool FormattedCheckpoint::dataAreConsistent(ShellData const& shellData, unsigned const nAtoms)
{
   for (int i = 0; i < shellData.shellToAtom.size(); ++i) {
       int atomIndex(shellData.shellToAtom[i]);
       if (atomIndex < 1 || atomIndex > (int)nAtoms) {
          QString msg("Invalid atom index ");
          msg += QString::number(atomIndex) + " ";
          msg += QString::number(nAtoms);
          m_errors.append(msg);
          return false;
       }
   }

   int nPrimitives(0);
   int nShells(shellData.shellTypes.size());
   for (int i = 0; i < shellData.shellPrimitives.size(); ++i) {
       nPrimitives += shellData.shellPrimitives[i];
   }

   if ( shellData.shellToAtom.size()               != nShells     ||
        shellData.shellPrimitives.size()           != nShells     ||
        shellData.exponents.size()                 != nPrimitives ||
        shellData.contractionCoefficients.size()   != nPrimitives ||
       (shellData.contractionCoefficientsSP.size() != nPrimitives &&
        shellData.contractionCoefficientsSP.size() != 0)      ) {
       QLOG_WARN() << "Inconsistent checkpoint data:";
       QLOG_WARN() << "  Shells" << shellData.shellToAtom.size()               << nShells;
       QLOG_WARN() << "  Prims " << shellData.shellPrimitives.size()           << nShells;
       QLOG_WARN() << "  Expts " << shellData.exponents.size()                 << nPrimitives;
       QLOG_WARN() << "  CCs   " << shellData.contractionCoefficients.size()   << nPrimitives;
       QLOG_WARN() << "  SP CCs" << shellData.contractionCoefficientsSP.size() << nPrimitives;
       m_errors.append("Inconsistent shell data read from file");
       return false;
   }
   return true;
}



Data::Orbitals* FormattedCheckpoint::makeOrbitals(unsigned const nAlpha, 
   unsigned const nBeta, OrbitalData const& orbitalData, ShellData const& shellData, 
   Data::Geometry const& geometry, Data::DensityList densityList)
{
   if (orbitalData.alphaCoefficients.isEmpty()) return 0;
   // TODO: This needs to move to avoid duplication 
   Data::ShellList* shellList = makeShellList(shellData, geometry);
   if (!shellList) return 0;

   Data::Orbitals* orbitals(0);
   QString surfaceTag;

   switch (orbitalData.orbitalType) {

      case Data::Orbitals::Canonical: {
         Data::CanonicalOrbitals* canonical = 
            new Data::CanonicalOrbitals(nAlpha, nBeta, *shellList,
                orbitalData.alphaCoefficients, orbitalData.alphaEnergies, 
                orbitalData.betaCoefficients,  orbitalData.betaEnergies, orbitalData.label);
         canonical->appendDensities(densityList); 
         orbitals = canonical;
      } break;

      case Data::Orbitals::Localized: {
         orbitals = new Data::Orbitals(orbitalData.orbitalType, nAlpha, nBeta, 
            *shellList, orbitalData.alphaCoefficients, orbitalData.betaCoefficients, 
            orbitalData.label);
      } break;

      case Data::Orbitals::NaturalTransition: {
         //TODO: create new derived orbitals classes for these
         Data::CanonicalOrbitals* canonical = 
            new Data::CanonicalOrbitals(nAlpha, nBeta, *shellList,
                orbitalData.alphaCoefficients, orbitalData.alphaEnergies, 
                orbitalData.betaCoefficients,  orbitalData.betaEnergies, orbitalData.label);
         orbitals = canonical;
         orbitals->setOrbitalType(Data::Orbitals::NaturalTransition);
      } break;

      case Data::Orbitals::NaturalBond: {

         //TODO: create new derived orbitals classes for these
         Data::CanonicalOrbitals* canonical = 
            new Data::CanonicalOrbitals(nAlpha, nBeta, *shellList,
                orbitalData.alphaCoefficients, orbitalData.alphaEnergies, 
                orbitalData.betaCoefficients,  orbitalData.betaEnergies, orbitalData.label);
         orbitals = canonical;
         orbitals->setOrbitalType(Data::Orbitals::NaturalBond);

//       qDebug() << "Add one NTO(3)/NBO(4). Code = " << mos->orbitalType();
//       surfaceTag = QString(moData.stateTag);
//       if (moData.stateNumber!= 0) surfaceTag += QString::number(moData.stateNumber);
         //surfaceTag = QString(moData.stateTag) + QString::number(moData.stateNumber);
//       mos->setOrbTitle(surfaceTag);
      }  break;

      default:
         QLOG_WARN() << "Unknown oribital type in FormattedCheckpoint::makeOrbitals";
         break;
   }

   if (orbitals && !orbitals->consistent()) {
      QString msg(Data::Orbitals::toString(orbitalData.orbitalType));
      msg += " orbital data are inconsistent. Check shell types.";
      m_errors.append(msg);
      delete orbitals;
      orbitals = 0;
   }

   return orbitals;
}


Data::GeminalOrbitals* FormattedCheckpoint::makeGeminalOrbitals(unsigned const nAlpha,
   unsigned const nBeta, GmoData const& gmoData, ShellData const& shellData, 
   Data::Geometry const& geometry)
{
   // This needs fixing.  Newer versions of QChem only print the orbitals for the
   // final geometry, so the first ones are just for the geometries
   if (gmoData.geminalEnergies.isEmpty()) return 0;
   Data::ShellList* shellList = makeShellList(shellData, geometry);
   if (!shellList) return 0;
   Data::GeminalOrbitals* gmos = new Data::GeminalOrbitals(
      nAlpha, 
      nBeta,
      shellList->nBasis(),
      gmoData.alphaCoefficients,
      gmoData.betaCoefficients,
      gmoData.geminalEnergies,
      gmoData.geminalCoefficients,
      gmoData.geminalMoMap,
      *shellList
   );

   if (!gmos->consistent()) {
      QString msg("Geminal data are inconsistent. Check shell types.");
      m_errors.append(msg);
      delete gmos;
      gmos = 0;
   }

   return gmos;
}


Data::ShellList* FormattedCheckpoint::makeShellList(ShellData const& shellData, 
   Data::Geometry const& geometry)
{
   if (!dataAreConsistent(shellData, geometry.nAtoms())) return 0;

   static double const convExponents(std::pow(Constants::BohrToAngstrom, -2.0));
   Data::ShellList* shellList(new Data::ShellList);
   unsigned nShells(shellData.shellTypes.size());
   unsigned cnt(0);

   for (unsigned shell = 0; shell < nShells; ++shell) {

       QList<double> expts;
       QList<double> coefs;
       QList<double> coefsSP;

       unsigned atom(shellData.shellToAtom.at(shell)-1);
       qglviewer::Vec position(geometry.position(atom));

       for (unsigned i = 0; i < shellData.shellPrimitives.at(shell); ++i, ++cnt) {
		   // Convert exponents from bohr to angstrom.  The conversion factor
		   // for the coefficients depends on the angular momentum and the 
           // conversion is effectively done Shell constructor
           expts.append(shellData.exponents.at(cnt)*convExponents);

           coefs.append(shellData.contractionCoefficients.at(cnt));
           if (!shellData.contractionCoefficientsSP.isEmpty()) {
              coefsSP.append(shellData.contractionCoefficientsSP.at(cnt));
           }
       }

       switch (shellData.shellTypes.at(shell)) {
          case 0:
             shellList->append( new Data::Shell(Data::Shell::S, position, expts, coefs) );
             break;
          case -1:
             shellList->append( new Data::Shell(Data::Shell::S, position, expts, coefs)   );
             shellList->append( new Data::Shell(Data::Shell::P, position, expts, coefsSP) );
             break;
          case 1:
             shellList->append( new Data::Shell(Data::Shell::P, position, expts, coefs) );
             break;
          case -2:
             shellList->append( new Data::Shell(Data::Shell::D5, position, expts, coefs) );
             break;
          case 2:
             shellList->append( new Data::Shell(Data::Shell::D6, position, expts, coefs) );
             break;
          case -3:
             shellList->append( new Data::Shell(Data::Shell::F7, position, expts, coefs) );
             break;
          case 3:
             shellList->append( new Data::Shell(Data::Shell::F10, position, expts, coefs) );
             break;
          case -4:
             shellList->append( new Data::Shell(Data::Shell::G9, position, expts, coefs) );
             break;
          case 4:
             shellList->append( new Data::Shell(Data::Shell::G15, position, expts, coefs) );
             break;

          default:
             delete shellList;
             QString msg("Unknown Shell type found at position ");
             msg += QString::number(shell);
             msg += ", type: "+ QString::number(shellData.shellTypes.at(shell));
             m_errors.append(msg);
             return 0;
             break;

       }
   }

   return shellList;
}

bool FormattedCheckpoint::installExcitedStates(unsigned const nAlpha, unsigned const nBeta,
   ExtData &extData, OrbitalData const& moData)
{
   Data::ExcitedStates* states(new Data::ExcitedStates(extData.extType));
      qDebug() << "Reading" << states->typeLabel() << "States";

   qglviewer::Vec moment;
   double strength(0.0), energy(0.0), s2(0.0);

   int NOa = nAlpha;
   int NOb = nBeta;
   int NVa = moData.alphaEnergies.size() - NOa;
   int NVb = moData.betaEnergies.size()  - NOb;

   bool restricted(moData.betaEnergies.isEmpty());
   if (restricted) NVb += moData.alphaEnergies.size();

qDebug() << "Number of orbitals etc" << NOa << NOb << NVa << NVb <<  nAlpha << nBeta << moData.alphaEnergies.size()  << moData.betaEnergies.size();
qDebug() << "nState" << extData.nState <<extData.excitationEnergies.size() << extData.oscillatorStrengths.size();

   for (unsigned i = 0; i < extData.nState; i++) {
      energy = extData.excitationEnergies[i] * Constants::HartreeToEv;
      strength = extData.oscillatorStrengths[i];
      Data::ElectronicTransition* transition(
          new Data::ElectronicTransition(energy, strength, moment, s2));
      if (extData.alphaSparseJ.size() > 0) {
         if (!transition->addAmplitude(extData.alphaAmplitudes,
               extData.alphaSparseJ, extData.alphaSparseI,
               i+1,NOa, NVa, Data::Alpha)) return false;
         if (extData.betaAmplitudes.size() > 0) {
           if (!transition->addAmplitude(extData.betaAmplitudes,
               extData.betaSparseJ, extData.betaSparseI,
               i+1,NOb, NVb, Data::Beta)) return false;
         }
      } 
      else {
         if (!transition->addAmplitude(extData.alphaAmplitudes,i+1,
               NOa, NVa, Data::Alpha)) return false;
         if (extData.betaAmplitudes.size() > 0) {
           if (!transition->addAmplitude(extData.betaAmplitudes,i+1,
               NOb, NVb, Data::Beta)) return false;
         }
         if (extData.alphaYAmplitudes.size() > 0) {
           if (!transition->addAmplitude(extData.alphaYAmplitudes,i+1,
                 NOa, NVa, Data::Alpha)) return false;
           if (extData.betaYAmplitudes.size() > 0) {
             if (!transition->addAmplitude(extData.betaYAmplitudes,i+1,
                 NOb, NVb, Data::Beta)) return false;
           }
         }
      }
      states->append(transition);
      qDebug() << "Add transitions to state: " << i + 1;
   }
   if (states->nTransitions() > 0) m_dataBank.append(states);

   // set up orbital symmetry
   QList<Data::ExcitedStates*> es(m_dataBank.findData<Data::ExcitedStates>());
   if (es.isEmpty()) return false;
   Data::OrbitalSymmetries& data(es.last()->orbitalSymmetries());

   QString sym = "A";
   data.setOccupied(Data::Alpha,nAlpha);
   for (int n = 0; n < NOa+NVa ; n++) {
      energy = moData.alphaEnergies[n];
      data.append(Data::Alpha, energy, sym);
   }
   data.setOccupied(Data::Beta,nBeta);
   for (int n = 0; n < NOb+NVb; n++) {
      energy = restricted ? moData.alphaEnergies[n] : moData.betaEnergies[n];
      data.append(Data::Beta, energy, sym);
   }

   return true;
}


QList<int> FormattedCheckpoint::readIntegerArray(TextStream& textStream, unsigned n)
{
   bool ok;
   QList<int> values;

   while ((unsigned)values.size() < n) {
      QStringList tokens(textStream.nextLineAsTokens());
      for (unsigned i = 0; i < (unsigned)tokens.size(); ++i) {
          int v(tokens.at(i).toInt(&ok));
          if (!ok) goto error;
          values.append(v);
      }
   }

   if ((unsigned)values.size() == n) return values;

   error:
      QString msg("Error parsing checkpoint data around line number ");
      msg += QString::number(textStream.lineNumber()) + "\n";
      msg += "Expected integer value";
      m_errors.append(msg);

   return QList<int>();
}


QList<unsigned> FormattedCheckpoint::readUnsignedArray(TextStream& textStream, unsigned n)
{
   bool ok;
   QList<unsigned> values;

   while ((unsigned)values.size() < n) {
      QStringList tokens(textStream.nextLineAsTokens());
      for (unsigned i = 0; i < (unsigned)tokens.size(); ++i) {
          unsigned v(tokens.at(i).toUInt(&ok));
          if (!ok) goto error;
          values.append(v);
      }
   }

   if ((unsigned)values.size() == n) return values;

   error:
      QString msg("Error parsing checkpoint data around line number ");
      msg += QString::number(textStream.lineNumber()) + "\n";
      msg += "Expected unsigned integer value";
      m_errors.append(msg);

   return QList<unsigned>();
}


QList<double> FormattedCheckpoint::readDoubleArray(TextStream& textStream, unsigned n)
{
   bool ok;
   QList<double> values;

   while ((unsigned)values.size() < n) {
      QString line(textStream.readLine());
      while (!line.isEmpty()) {
         QString token(line.left(16));
         double v(token.toDouble(&ok));
         if (!ok) goto error;
         values.append(v);
         line.remove(0,token.size());
      }
   }

   if ((unsigned)values.size() == n) return values;

   error:
      QString msg("Error parsing checkpoint data around line number ");
      msg += QString::number(textStream.lineNumber()) + "\n";
      msg += "Expected double value";
      m_errors.append(msg);

   return QList<double>();
}

} } // end namespace IQmol::Parser
