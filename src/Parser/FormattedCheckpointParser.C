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
#include "CanonicalOrbitals.h"
#include "LocalizedOrbitals.h"
#include "NaturalTransitionOrbitals.h"
#include "NaturalBondOrbitals.h"
#include "GeminalOrbitals.h"
#include "DysonOrbitals.h"
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


bool FormattedCheckpoint::toInt(unsigned& n, QStringList const& list, unsigned const index)
{
   bool ok(false);
   if (list.size() > (int)index) n = list.at(index).toInt(&ok);
   return ok;
}

bool FormattedCheckpoint::toDouble(double& x, QStringList const& list, unsigned const index)
{
   bool ok(false);
   if (list.size() > (int)index) x = list.at(index).toDouble(&ok);
   return ok;
}



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

   OrbitalData virtLocData;
   virtLocData.orbitalType = Data::Orbitals::Localized;
   virtLocData.label = "Localized MOs (VirtLoc)";

   OrbitalData ntoData;
   ntoData.orbitalType = Data::Orbitals::NaturalTransition;
   ntoData.label = "Natural Transition Orbitals";

   OrbitalData nboData;
   nboData.orbitalType = Data::Orbitals::NaturalBond;
   nboData.label = "Natural Bond Orbitals";

   OrbitalData dysonData;
   dysonData.orbitalType = Data::Orbitals::Dyson;
   dysonData.label = "Dyson Orbitals";

   OrbitalData genericData;
   genericData.orbitalType = Data::Orbitals::Generic;
   genericData.label = "Generic Orbitals";


   unsigned nAlpha(0);
   unsigned nBeta(0);
   bool     ok(true);
   unsigned n(0);

   GeomData  geomData;
   Data::ShellData shellData;

   GmoData   gmoData;
   ExtData   extData;   
   extData.nState = 0;

   Data::DensityList densityList;

   QString key;

   while (!textStream.atEnd()) {

      QString line(textStream.nextLine());
      key = line;
      key.resize(42);
      key = key.trimmed();
      QString tmp(line.mid(43, 37));


      QStringList list(TextStream::tokenize(tmp));

      if (key == "Number of alpha electrons") {            // This should only appear once
         if (!toInt(nAlpha, list, 1)) goto error;

      }else if (key == "Number of beta electrons") {       // This should only appear once
         if (!toInt(nBeta, list, 1)) goto error;

      }else if (key == "Multiplicity") { 
         if (!toInt(n, list, 1)) goto error;
         geomData.multiplicity = n;

      }else if (key == "Charge") { 
         if (!toInt(n, list, 1)) goto error;
         geomData.charge = n;

      }else if (key == "Atomic numbers") {                 // This should only appear once
         if (!toInt(n, list, 2)) goto error;
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

         if (!toInt(n, list, 2)) goto error;
         geomData.coordinates = readDoubleArray(textStream, n);
         geometry = makeGeometry(geomData);
         if (!geometry) goto error;
         geometryList->append(geometry);

      }else if (key == "SCF Iteration") {
         if (!toInt(n, list, 1)) goto error;
         hfData.label = "SCF Iter " +  QString::number(n-1);

      }else if (key == "SCF Iteration Energy") {
         double energy(0.0);
         if (!toDouble(energy, list, 1)) goto error;
         if (!ok) goto error;
         hfData.label += " (" + QString::number(energy, 'f', 6) + ")";

         if (geometry) {
            // The user may have requested orbitals for each SCF cycle, 
            Data::Orbitals* orbitals(makeOrbitals(nAlpha, nBeta, hfData, shellData,
               *geometry, densityList));
            if (orbitals) orbitalsList->append(orbitals);
         }

      }else if (key == "Number of basis functions") {
         if (!toInt(n, list, 1)) goto error;
         shellData.nBasis = n;

      }else if (key == "Shell types") {
         if (!toInt(n, list, 2)) goto error;
         shellData.shellTypes = readIntegerArray(textStream, n);
         
      }else if (key == "Number of primitives per shell") {
         if (!toInt(n, list, 2)) goto error;
         shellData.shellPrimitives = readUnsignedArray(textStream, n);

      }else if (key == "Shell to atom map") {
         if (!toInt(n, list, 2)) goto error;
         shellData.shellToAtom = readUnsignedArray(textStream, n);

      }else if (key == "Primitive exponents") {
         if (!toInt(n, list, 2)) goto error;
         shellData.exponents = readDoubleArray(textStream, n);

      }else if (key == "Contraction coefficients") {
         if (!toInt(n, list, 2)) goto error;
         shellData.contractionCoefficients = readDoubleArray(textStream, n);

      }else if (key == "P(S=P) Contraction coefficients") {
         if (!toInt(n, list, 2)) goto error;
         shellData.contractionCoefficientsSP = readDoubleArray(textStream, n);

      }else if (key == "Overlap Matrix") {
         if (!toInt(n, list, 2)) goto error;
         shellData.overlapMatrix = readDoubleArray(textStream, n);

      }else if (key == "SCF Energy") {
         double energy(0.0);
         if (!geometry || !toDouble(energy, list, 1)) goto error;
         Data::ScfEnergy& scf(geometry->getProperty<Data::ScfEnergy>());
         scf.setValue(energy, Data::Energy::Hartree);
         Data::TotalEnergy& total(geometry->getProperty<Data::TotalEnergy>());
         total.setValue(energy, Data::Energy::Hartree);

      }else if (key == "Total Energy") {
         double energy(0.0);
         if (!geometry || !toDouble(energy, list, 1)) goto error;
         if (!ok || !geometry) goto error;
         Data::TotalEnergy& total(geometry->getProperty<Data::TotalEnergy>());
         total.setValue(energy, Data::Energy::Hartree);

      }else if (key == "Dipole_Data") {
         if (!geometry || !toInt(n, list, 2)) goto error;
         QList<double> data(readDoubleArray(textStream, n));
         if (data.size() != 3) goto error;
         Data::DipoleMoment& dipole(geometry->getProperty<Data::DipoleMoment>());
         dipole.setValue(data[0],data[1],data[2]);

      }else if (key == "Cartesian Force Constants") {
         if (!geometry || !toInt(n, list, 2)) goto error;
         QList<double> data(readDoubleArray(textStream, n));
         Data::Hessian& hessian(geometry->getProperty<Data::Hessian>());
         hessian.setData(geometry->nAtoms(), data);

      // Canonical Orbitals

      }else if (key == "Alpha MO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         hfData.alphaCoefficients = readDoubleArray(textStream, n);

	  }else if (key == "Beta MO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         hfData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Alpha Orbital Energies") {
         if (!toInt(n, list, 2)) goto error;
         hfData.alphaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Beta Orbital Energies") {
         if (!toInt(n, list, 2)) goto error;
         hfData.betaEnergies = readDoubleArray(textStream, n);

      // Natural Transition Orbitals

	  }else if (key == "Alpha NTO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         ntoData.alphaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Beta NTO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         ntoData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Alpha NTO amplitudes") {
         if (!toInt(n, list, 2)) goto error;
         ntoData.alphaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Beta NTO amplitudes") {
         if (!toInt(n, list, 2)) goto error;
         ntoData.betaEnergies = readDoubleArray(textStream, n);

      // Natural Bond Orbitals

	  }else if (key == "Alpha NBO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         nboData.alphaCoefficients = readDoubleArray(textStream, n);

	  }else if (key == "Beta NBO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         nboData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Alpha NBO occupancies") {
         if (!toInt(n, list, 2)) goto error;
         nboData.alphaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Beta NBO occupancies") {
         if (!toInt(n, list, 2)) goto error;
         nboData.betaEnergies = readDoubleArray(textStream, n);

      // Localized Orbitals

      }else if (key == "Localized Alpha MO Coefficients (ER)") {
         if (!toInt(n, list, 2)) goto error;
         erData.alphaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Beta  MO Coefficients (ER)") {
         if (!toInt(n, list, 2)) goto error;
         erData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Alpha MO Coefficients (Boys)") {
         if (!toInt(n, list, 2)) goto error;
         boysData.alphaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Beta  MO Coefficients (Boys)") {
         if (!toInt(n, list, 2)) goto error;
         boysData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Alpha MO Coefficients (VirtLoc)") {
         if (!toInt(n, list, 2)) goto error;
         virtLocData.alphaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Localized Beta  MO Coefficients (VirtLoc)") {
         if (!toInt(n, list, 2)) goto error;
         virtLocData.betaCoefficients = readDoubleArray(textStream, n);


      // Dyson Orbitals

      }else if (key.contains("EOM-IP") || 
                key.contains("EOM-EA") ||
                key.contains("EOM-SF") ) {
         double energy(0.0);
         if (!toDouble(energy, list, 1)) goto error;
         dysonData.alphaEnergies.append(energy*Constants::HartreeToEv);
         dysonData.labels.append(key);
                
      }else if (key == "Dyson Orbital (left)") {
         if (!toInt(n, list, 2)) goto error;
         dysonData.alphaCoefficients.append(readDoubleArray(textStream, n));
         
      }else if (key == "Dyson Orbital (right)") {
         if (!toInt(n, list, 2)) goto error;
         dysonData.betaCoefficients.append(readDoubleArray(textStream, n));
         
      // Generic Orbitals
      }else if (key.contains("Orbital Coefficients")) {
         if (!toInt(n, list, 2)) goto error;
         genericData.alphaCoefficients.append(readDoubleArray(textStream, n));
         key.replace("Orbital Coefficients", "");
         genericData.label = key.trimmed();

      // Geminals

      }else if (key == "Alpha GMO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         gmoData.alphaCoefficients = readDoubleArray(textStream, n);
         gmoData.betaCoefficients  = gmoData.alphaCoefficients;

      }else if (key == "Beta GMO coefficients") {
         if (!toInt(n, list, 2)) goto error;
         gmoData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "MO to geminal map") {
         if (!toInt(n, list, 2)) goto error;
         gmoData.geminalMoMap = readIntegerArray(textStream, n);

      }else if (key == "Geminal Coefficients") {
         if (!toInt(n, list, 2)) goto error;
         gmoData.geminalCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Energies of Geminals") {
         if (!toInt(n, list, 2)) goto error;
         gmoData.geminalEnergies = readDoubleArray(textStream, n);

      }else if (key.contains("RMS Density")) {
         // Skip this

      }else if (key.contains("Density", Qt::CaseInsensitive)) {
         if (!toInt(n, list, 2)) goto error;
         QList<double> data(readDoubleArray(textStream, n));
         Data::SurfaceType type(Data::SurfaceType::Custom);
         type.setLabel(key);
         // check if the density matrix is square
         bool square(n == shellData.nBasis*shellData.nBasis);
qDebug() << "Density matrix is square?" << square;
         Data::Density* density(new Data::Density(type, data, key, square));
         density->dump();
         densityList.append(density);

      }else if (key.endsWith("Surface Title") || key == "NBO Ground State" ) {
         if (!geometry || !toInt(n, list, 1)) goto error;

         if (key.contains("Surface Title")) {
            key.replace("NBO ","");
            key.replace("Surface Title","");
  	        ntoData.label = key + " State: " + QString::number(n);
  	        nboData.label = key + " State: " + QString::number(n);
         }else {
  	        nboData.label = key;
  	        ntoData.label = key;
         }
    
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
         if (!toInt(n, list, 2)) goto error;
         extData.excitationEnergies = readDoubleArray(textStream, n);
         extData.nState = n;
         extData.extType = key.contains("EOMEE") ? Data::ExcitedStates::EOM
                                                 : Data::ExcitedStates::CIS;
      }else if (key == "Oscillator Strengths") {
         if (!toInt(n, list, 2)) goto error;
         extData.oscillatorStrengths = readDoubleArray(textStream, n);

      }else if (key == "Alpha Amplitudes" || key == "Alpha X Amplitudes") {
         if (!toInt(n, list, 2)) goto error;
         extData.alphaAmplitudes = readDoubleArray(textStream, n);
      
      }else if (key == "Alpha Y Amplitudes") {
         if (!toInt(n, list, 2)) goto error;
         extData.alphaYAmplitudes = readDoubleArray(textStream, n);
         extData.extType = Data::ExcitedStates::TDDFT;

      }else if (key == "Beta Amplitudes" || key == "Beta X Amplitudes") {
         if (!toInt(n, list, 2)) goto error;
         extData.betaAmplitudes = readDoubleArray(textStream, n);

      }else if (key == "Beta Y Amplitudes") {
         if (!toInt(n, list, 2)) goto error;
         extData.betaYAmplitudes = readDoubleArray(textStream, n);

      }else if (key == "Alpha J Indexes") {
         if (!toInt(n, list, 2)) goto error;
         extData.alphaSparseJ = readIntegerArray(textStream, n);

      }else if (key == "Alpha I Indexes") {
         if (!toInt(n, list, 2)) goto error;
         extData.alphaSparseI = readIntegerArray(textStream, n);

      }else if (key == "Beta J Indexes") {
         if (!toInt(n, list, 2)) goto error;
         extData.betaSparseJ = readIntegerArray(textStream, n);

      }else if (key == "Beta I Indexes") {
         if (!toInt(n, list, 2)) goto error;
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

      orbitals = makeOrbitals(nAlpha, nBeta, virtLocData, shellData, *geometry); 
      if (orbitals) orbitalsList->append(orbitals);

      orbitals = makeOrbitals(nAlpha, nBeta, dysonData, shellData, *geometry); 
      if (orbitals) orbitalsList->append(orbitals);

      orbitals = makeOrbitals(nAlpha, nBeta, genericData, shellData, *geometry); 
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
   // Don't clear this as it results in blank items in multi-job files.
   // orbitalData.label.clear();  
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


bool FormattedCheckpoint::dataAreConsistent(Data::ShellData const& shellData, 
   unsigned const nAtoms)
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
   unsigned const nBeta, OrbitalData const& orbitalData, Data::ShellData const& shellData, 
   Data::Geometry const& geometry, Data::DensityList densityList)
{
   if (orbitalData.alphaCoefficients.isEmpty()) return 0;
   // TODO: This needs to move to avoid duplication 
   //Data::ShellList* shellList = makeShellList(shellData, geometry);
   Data::ShellList* shellList = new Data::ShellList(shellData, geometry);
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
         orbitals = new Data::LocalizedOrbitals(nAlpha, nBeta, *shellList, 
            orbitalData.alphaCoefficients, orbitalData.betaCoefficients, 
            orbitalData.label);
      } break;

      case Data::Orbitals::NaturalTransition: {
         orbitals = new Data::NaturalTransitionOrbitals(*shellList,
            orbitalData.alphaCoefficients, orbitalData.alphaEnergies, 
            orbitalData.betaCoefficients,  orbitalData.betaEnergies, orbitalData.label);
      } break;

      case Data::Orbitals::Dyson: {
         orbitals = new Data::DysonOrbitals(*shellList, orbitalData.alphaCoefficients, 
            orbitalData.betaCoefficients, orbitalData.alphaEnergies, orbitalData.labels);
      } break;

      case Data::Orbitals::NaturalBond: {
         orbitals = new Data::NaturalBondOrbitals(nAlpha, nBeta, *shellList,
            orbitalData.alphaCoefficients, orbitalData.alphaEnergies, 
            orbitalData.betaCoefficients,  orbitalData.betaEnergies, orbitalData.label);
      }  break;

      case Data::Orbitals::Generic: {
         orbitals = new Data::Orbitals(Data::Orbitals::Generic, *shellList, 
            orbitalData.alphaCoefficients, orbitalData.betaCoefficients, orbitalData.label);
            
      } break;

      default:
         QLOG_WARN() << "Unknown orbital type in FormattedCheckpoint::makeOrbitals";
         break;
   }

   if (orbitals && !orbitals->consistent()) {
      QString msg(Data::Orbitals::toString(orbitalData.orbitalType));
      msg += " data are inconsistent. Check shell types.";
      m_errors.append(msg);
      delete orbitals;
      orbitals = 0;
   }

   return orbitals;
}


Data::GeminalOrbitals* FormattedCheckpoint::makeGeminalOrbitals(unsigned const nAlpha,
   unsigned const nBeta, GmoData const& gmoData, Data::ShellData const& shellData, 
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


// This has been moved to Data::ShellList so Parser::QChemOutputFile can use it.
// To be removed
Data::ShellList* FormattedCheckpoint::makeShellList(Data::ShellData const& shellData, 
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
       qglviewer::Vec pos(geometry.position(atom));

       for (unsigned i = 0; i < shellData.shellPrimitives.at(shell); ++i, ++cnt) {
		   // Convert exponents from bohr to angstrom.  The conversion factor
		   // for the coefficients depends on the angular momentum and the 
           // conversion is effectively done in the  Shell constructor
           expts.append(shellData.exponents.at(cnt)*convExponents);

           coefs.append(shellData.contractionCoefficients.at(cnt));
           if (!shellData.contractionCoefficientsSP.isEmpty()) {
              coefsSP.append(shellData.contractionCoefficientsSP.at(cnt));
           }
       }

       switch (shellData.shellTypes.at(shell)) {
          case 0:
             shellList->append( new Data::Shell(Data::Shell::S, atom, pos, expts, coefs) );
             break;
          case -1:
             shellList->append( new Data::Shell(Data::Shell::S, atom, pos, expts, coefs) );
             shellList->append( new Data::Shell(Data::Shell::P, atom, pos, expts, coefsSP) );
             break;
          case 1:
             shellList->append( new Data::Shell(Data::Shell::P, atom, pos, expts, coefs) );
             break;
          case -2:
             shellList->append( new Data::Shell(Data::Shell::D5, atom, pos, expts, coefs) );
             break;
          case 2:
             shellList->append( new Data::Shell(Data::Shell::D6, atom, pos, expts, coefs) );
             break;
          case -3:
             shellList->append( new Data::Shell(Data::Shell::F7, atom, pos, expts, coefs) );
             break;
          case 3:
             shellList->append( new Data::Shell(Data::Shell::F10, atom, pos, expts, coefs) );
             break;
          case -4:
             shellList->append( new Data::Shell(Data::Shell::G9, atom, pos, expts, coefs) );
             break;
          case 4:
             shellList->append( new Data::Shell(Data::Shell::G15, atom, pos, expts, coefs) );
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

   unsigned nBasis(shellList->nBasis());
   if (shellData.overlapMatrix.size() == (nBasis+1)*nBasis/2) {
      shellList->setOverlapMatrix(shellData.overlapMatrix);
   }

   shellList->resize();
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

   qDebug() << "Number of orbitals etc" 
            << NOa << NOb << NVa << NVb <<  nAlpha << nBeta 
            << moData.alphaEnergies.size()  << moData.betaEnergies.size();
   qDebug() << "nState" << extData.nState   << extData.excitationEnergies.size() 
            << extData.oscillatorStrengths.size();

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
