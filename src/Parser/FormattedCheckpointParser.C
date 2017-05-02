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
#include "MolecularOrbitalsList.h"
#include "MolecularOrbitals.h"
#include "GeminalOrbitals.h"
#include "DipoleMoment.h"
#include "GeometryList.h"
#include "TextStream.h"
#include "Constants.h"
#include "Hessian.h"
#include "Energy.h"
#include "QsLog.h"
#include <QtDebug>
#include <cmath>
#include "Data.h"
#include "Spin.h"
#include "ExcitedStates.h"
#include "Constants.h"

namespace IQmol {
namespace Parser {

bool FormattedCheckpoint::parse(TextStream& textStream)
{
   Data::GeometryList* geometryList(new Data::GeometryList);
   Data::MolecularOrbitalsList* 
      molecularOrbitalsList(new Data::MolecularOrbitalsList(Data::MolecularOrbitals::Canonical));
   Data::MolecularOrbitalsList* 
      naturaltransOrbitalList(new Data::MolecularOrbitalsList(Data::MolecularOrbitals::NaturalTransition));

   Data::MolecularOrbitalsList* 
      naturalbondOrbitalList(new Data::MolecularOrbitalsList(Data::MolecularOrbitals::NaturalBond));
   Data::Geometry* geometry(0);

   bool ok(true);
   GeomData  geomData;
   ShellData shellData;
   MoData    moData;
   MoData    ntoData;
   MoData    nboData;
   GmoData   gmoData;
   ExtData   extData;   extData.nState = 0;

   while (!textStream.atEnd()) {

      QString line(textStream.nextLine());
      QString key(line);
      key.resize(42);
      key = key.trimmed();
      QString tmp(line.mid(43, 37));

      QStringList list(TextStream::tokenize(tmp));

      if (key == "Number of alpha electrons") {            // This should only appear once
         moData.nAlpha  = list.at(1).toInt(&ok);
         ntoData.nAlpha = list.at(1).toInt(&ok);
         nboData.nAlpha = list.at(1).toInt(&ok);
         gmoData.nAlpha = list.at(1).toInt(&ok);
         if (!ok) goto error;

      }else if (key == "Number of beta electrons") {       // This should only appear once
         moData.nBeta  = list.at(1).toInt(&ok);
         ntoData.nBeta = list.at(1).toInt(&ok);
         nboData.nBeta = list.at(1).toInt(&ok);
         gmoData.nBeta = list.at(1).toInt(&ok);
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
            // We are on a subsequent geometry, so we should have everything we need to
            // create the MOs for the previous one.
            Data::MolecularOrbitals* mos(makeMolecularOrbitals(moData, shellData, *geometry)); 
            clear(moData);
            if (mos) molecularOrbitalsList->append(mos);
         }

         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         geomData.coordinates = readDoubleArray(textStream, n);
         geometry = makeGeometry(geomData);
         if (!geometry) goto error;
         geometryList->append(geometry);

      }else if (key == "Number of basis functions") {
         gmoData.nBasis = list.at(1).toUInt(&ok);
         if (!ok) goto error;
         // Also determined from the size of the MO coefficient matrices 
         // for MolecularOrbitals

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

      }else if (key == "Alpha MO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         moData.alphaCoefficients = readDoubleArray(textStream, n);
         moData.betaCoefficients  = moData.alphaCoefficients;
         moData.orbitalType       = Data::MolecularOrbitals::Canonical;

      }else if (key == "Alpha NTO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         ntoData.alphaCoefficients = readDoubleArray(textStream, n);
         ntoData.betaCoefficients  = ntoData.alphaCoefficients;
         ntoData.orbitalType       = Data::MolecularOrbitals::NaturalTransition;

      }else if (key == "Alpha NBO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         nboData.alphaCoefficients = readDoubleArray(textStream, n);
         nboData.betaCoefficients  = nboData.alphaCoefficients;
         nboData.orbitalType       = Data::MolecularOrbitals::NaturalBond;

      }else if (key == "Beta MO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         moData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Beta NTO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         ntoData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Beta NBO coefficients") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         nboData.betaCoefficients = readDoubleArray(textStream, n);

      }else if (key == "Alpha Orbital Energies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         moData.alphaEnergies = readDoubleArray(textStream, n);
         moData.betaEnergies  = moData.alphaEnergies;

      }else if (key == "Beta Orbital Energies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         moData.betaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Alpha NTO amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         ntoData.alphaEnergies = readDoubleArray(textStream, n);
         ntoData.betaEnergies  = ntoData.alphaEnergies;

      }else if (key == "Beta NTO amplitudes") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         ntoData.betaEnergies = readDoubleArray(textStream, n);

      }else if (key == "Alpha NBO occupancies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok) goto error;
         nboData.alphaEnergies = readDoubleArray(textStream, n);
         nboData.betaEnergies  = nboData.alphaEnergies;

      }else if (key == "Beta NBO occupancies") {
         unsigned n(list.at(2).toUInt(&ok));
         if (!ok || !geometry) goto error;
         nboData.betaEnergies = readDoubleArray(textStream, n);

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

      }else if (key.endsWith("Surface Title") || key == "NBO Ground State" ) {
         unsigned n(list.at(1).toUInt(&ok));
         if (!ok || !geometry) goto error;

         if (ntoData.orbitalType == Data::MolecularOrbitals::NaturalTransition) {
            ntoData.whichState = n;
            ntoData.stateTag = QString(key.replace("Surface Title",""));
            Data::MolecularOrbitals* ntos(makeMolecularOrbitals(ntoData, shellData, *geometry)); 
            clear(ntoData);
            if (ntos) naturaltransOrbitalList->append(ntos);
	        qDebug() << "Append one NTO to MO lists";

         }else if (nboData.orbitalType == Data::MolecularOrbitals::NaturalBond) {
  	        nboData.whichState = n;
            if (key == "NBO Ground State") {
	           nboData.stateTag = QString("Ground Sate");
	        }else {
	           nboData.stateTag = QString(key.replace("Surface Title",""));
            }

            Data::MolecularOrbitals* nbos(makeMolecularOrbitals(nboData, shellData, *geometry)); 
            clear(nboData);
            if (nbos) naturalbondOrbitalList->append(nbos);
	        qDebug() << "Append one NBO to MO lists";
         }
      //
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
         if (key.contains("EOMEE")) extData.extType = Data::ExcitedStates::EOM;
         else                       extData.extType = Data::ExcitedStates::CIS;

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
      Data::MolecularOrbitals* mos(makeMolecularOrbitals(moData, shellData, *geometry)); 
      if (mos) molecularOrbitalsList->append(mos);
      Data::GeminalOrbitals* gmos(makeGeminalOrbitals(gmoData, shellData, *geometry)); 
      if (gmos) m_dataBank.append(gmos);
//*
      // install excitation 
      if (extData.nState > 0) {
         ok = installExcitedStates(extData,moData);
         if (!ok) goto error;
      }
//*/
   }

   if (geometryList) {
      if (geometryList->isEmpty()) {
         delete geometryList;
      }else {
         geometryList->setDefaultIndex(-1);
         m_dataBank.append(geometryList);
      }
   }

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
   
   return m_errors.isEmpty();
 
   error:
      QString msg("Error parsing checkpoint data around line number ");
      msg += QString::number(textStream.lineNumber());
      m_errors.append(msg);

   delete geometryList;
   delete molecularOrbitalsList;
   delete naturaltransOrbitalList;
   delete naturalbondOrbitalList;

   return false;
}


void FormattedCheckpoint::clear(MoData& moData)
{
   moData.alphaCoefficients.clear();
   moData.betaCoefficients.clear();
   moData.alphaEnergies.clear();
   moData.betaEnergies.clear();
   moData.orbitalType = Data::MolecularOrbitals::Undefined;
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



Data::MolecularOrbitals* FormattedCheckpoint::makeMolecularOrbitals(MoData const& moData, 
   ShellData const& shellData, Data::Geometry const& geometry)
{
   // This needs fixing.  Newer versions of QChem only print the orbitals for the
   // final geometry, so the first ones are just for the geometries
   if (moData.alphaEnergies.isEmpty()) return 0;
   Data::ShellList* shellList = makeShellList(shellData, geometry);
   if (!shellList) return 0;

   Data::MolecularOrbitals* mos = new Data::MolecularOrbitals(
      moData.orbitalType,
      moData.nAlpha, 
      moData.nBeta,
      moData.alphaCoefficients,
      moData.alphaEnergies,
      moData.betaCoefficients,
      moData.betaEnergies,
      *shellList
   );

   if (!mos->consistent()) {
      QString msg("Data are inconsistent. Check shell types.");
      m_errors.append(msg);
      delete mos;
      mos = 0;
   }

   QString surfaceTag;
   switch (moData.orbitalType) {

      case Data::MolecularOrbitals::Canonical: 
      case Data::MolecularOrbitals::Localized: 
         qDebug() << "Add one MO: " << mos->orbitalType();
         mos->setOrbTitle("MO Surfaces");
         break;

      case Data::MolecularOrbitals::NaturalTransition:
      case Data::MolecularOrbitals::NaturalBond:
         qDebug() << "Add one NTO(3)/NBO(4). Code = " << mos->orbitalType();
         surfaceTag = QString(moData.stateTag);
         if (moData.whichState != 0) surfaceTag += QString::number(moData.whichState);
         //surfaceTag = QString(moData.stateTag) + QString::number(moData.whichState);
         mos->setOrbTitle(surfaceTag);
         break;

      case Data::MolecularOrbitals::Undefined: 
         qDebug() << "Undefined molecular orbtal type requested";
         break;
   }

   return mos;
}
            

Data::GeminalOrbitals* FormattedCheckpoint::makeGeminalOrbitals(GmoData const& gmoData, 
   ShellData const& shellData, Data::Geometry const& geometry)
{
   // This needs fixing.  Newer versions of QChem only print the orbitals for the
   // final geometry, so the first ones are just for the geometries
   if (gmoData.geminalEnergies.isEmpty()) return 0;
   Data::ShellList* shellList = makeShellList(shellData, geometry);
   if (!shellList) return 0;
   Data::GeminalOrbitals* gmos = new Data::GeminalOrbitals(
      gmoData.nAlpha, 
      gmoData.nBeta,
      gmoData.nBasis,
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

bool FormattedCheckpoint::installExcitedStates(ExtData &extData, MoData const& moData)
{
   Data::ExcitedStates* states(new Data::ExcitedStates(extData.extType));
      qDebug() << "Reading" << states->typeLabel() << "States";

   qglviewer::Vec moment;
   double strength(0.0), energy(0.0), s2(0.0);
   int NOa = moData.nAlpha;
   int NOb = moData.nBeta;
   int NVa = moData.alphaEnergies.size() - NOa;
   int NVb = moData.betaEnergies.size() - NOb;
   for (int i = 0; i < extData.nState; i++) {
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
   data.setOccupied(Data::Alpha,moData.nAlpha);
   for (int n = 0; n < NOa+NVa ; n++) {
      energy = moData.alphaEnergies[n];
      data.append(Data::Alpha, energy, sym);
   }
   data.setOccupied(Data::Beta,moData.nBeta);
   for (int n = 0; n < NOb+NVb; n++) {
      energy = moData.betaEnergies[n];
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
