/*******************************************************************************
       
  Copyright (C) 2011-2015ndrew Gilbert
           
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

#include "CartesianCoordinatesParser.h"
#include "QChemOutputParser.h"
#include "QChemInputParser.h"
#include "TextStream.h"
#include "AtomicProperty.h"
#include "Constraint.h"
#include "Energy.h"
#include "Frequencies.h"
#include "Hessian.h"
#include "PointGroup.h"
#include "Geometry.h"
#include "DipoleMoment.h"
#include "EfpFragmentLibrary.h"
#include "MultipoleExpansion.h"
#include "NmrReference.h"
#include "EfpFragment.h"
#include "Constants.h"
#include "RemSectionData.h"
#include "Numerical.h"
#include "NmrData.h"
#include "Matrix.h"
#include "Spin.h"

#include "QsLog.h"
#include <QRegExp>
#include <QFile>

#include <QtDebug>


using qglviewer::Vec;

namespace IQmol {
namespace Parser {

QStringList QChemOutput::parseForErrors(QString const& filePath)
{
   QStringList errors;

   QFile file(filePath);
   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      TextStream textStream(&file);
      errors = parseForErrors(textStream);
      file.close();
   }else {
      errors.append("Unable to open file");
   }

   return errors;
}

   
QStringList QChemOutput::parseForErrors(TextStream& textStream)
{

   QStringList errors;
   QString line;

   int nJobs(0);
   int nNiceEndings(0);
   double time(0.0);

   QRegExp rx("Total job time:(.+)s\\(wall\\)");

   QString error;

   while (!textStream.atEnd()) {
      line = textStream.readLine();

      if (line.contains("Q-Chem fatal error")) {
         textStream.skipLine();  // blank line
         error = textStream.readLine().trimmed();
         if (error.isEmpty()) error = "Fatal error occured at end of output file";

      }else if (line.contains("Time limit has been exceeded")) {
         error = "Time limit has been exceeded";

      }else if (line.contains("Welcome to Q-Chem")) {
         if (!error.isEmpty()) {
            errors.append(error);
         }
         error.clear();
         ++nJobs;

      }else if (line.contains("Have a nice day")) {
         ++nNiceEndings;

      }else if (rx.indexIn(line) != -1) {
         bool ok(false);
         double t(rx.cap(1).toDouble(&ok));
         if (ok) time += t;
      }
   }

   if (!error.isEmpty()) {
      qDebug() << "Appending error:" << error;
      errors.append(error);
   }

   if (nNiceEndings < nJobs) {
      nJobs -= nNiceEndings;
      if (nJobs == 1) {
        errors.append("Job failed to finish");
      }else {
        errors.append(QString::number(nJobs) + " Jobs failed");
      }
   }

   int t(time);
   if (t == 0) t = 1;
   errors.append("Time: " + QString::number(t));

   return errors;
}


bool QChemOutput::parse(TextStream& textStream)
{
   // A single output file can contain multiple jobs, but they all must
   // correspond to a single molecule (possibly with different geometries).
   // We use the first Geometry found to check all the others.
   Data::Geometry* firstGeometry(0);
   Data::Geometry* currentGeometry(0);
   Data::GeometryList* geometryList(0);
   Data::GeometryList* scanGeometries(0);
   Data::Nmr* nmr(0);

   m_nAlpha = 0;
   m_nBeta  = 0;
   QString method;

   QStringList tokens;
   QString line;

   while (!textStream.atEnd()) {
      line = textStream.nextLine();

      if (line.contains("Welcome to Q-Chem")) {
/*
         if (geometryList && !geometryList->isEmpty()) {
            m_dataBank.append(geometryList);
            geometryList = 0;
            currentGeometry = 0;
         }
*/

      }else if (line.contains("Q-Chem fatal error occurred in module")) {
         textStream.skipLine();
         QString msg("Q-Chem fatal error line: ");
         msg += QString::number(textStream.lineNumber());
         m_errors.append(msg + "\n" + textStream.nextLine());

      }else if (line.contains("Time limit has been exceeded")) {
         if (!m_errors.isEmpty()) m_errors.removeLast();
         m_errors.append("Time limit has been exceeded");

      }else if (line.contains("User input:") && !line.contains(" of ")) {
         textStream.skipLine();
         QChemInput parser;
         if (parser.parse(textStream)) {
            // Remove the input geometry list
            Data::Bank& bank(parser.data());
            bank.deleteData<Data::GeometryList>();
            m_dataBank.merge(bank);
         }else {
            m_errors << parser.errors();
         }

      }else if (line.contains("Standard Nuclear Orientation")) {
         bool convertFromBohr(line.contains("Bohr"));
         textStream.skipLine(2);
         Data::Geometry* geometry(readStandardCoordinates(textStream));

         if (geometry) {
            if (convertFromBohr) geometry->scaleCoordinates(Constants::BohrToAngstrom);
            if (!firstGeometry) firstGeometry = geometry;

            if (!geometryList) {
               geometryList = new Data::GeometryList;
               currentGeometry = 0;
            }

            if (geometry->sameAtoms(*firstGeometry)) {
               geometryList->append(geometry);
               currentGeometry = geometry;
            }else if (geometryList->isEmpty()) {
               // Different geometry found, which is unsupported.
               m_errors.append("More than one molecule found in file");
               break; 
            }else {
               // Different geometry found, possibly from EFPs.  We ignore it.
            }
         }else {
            QString msg("Problem parsing coordinates, line number ");
            m_errors.append(msg + QString::number(textStream.lineNumber()));
            break;
         }

      }else if (line.contains("PES scan, value:")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() > 5 && currentGeometry) {
            bool   energyOk(false), valueOk(false);
            double value(tokens[3].toDouble(&valueOk));
            double energy(tokens[5].toDouble(&energyOk));

            if (energyOk && valueOk) {
               if (!scanGeometries) scanGeometries = new Data::GeometryList("Scan Geometries");
               Data::Geometry* geom(new Data::Geometry(*currentGeometry));
               Data::TotalEnergy& total(currentGeometry->getProperty<Data::TotalEnergy>());
               total.setValue(energy, Data::Energy::Hartree);
               Data::Constraint& constraint(geom->getProperty<Data::Constraint>());
               constraint.setValue(value);
               scanGeometries->append(geom);
            }
         }
     

      }else if (line.contains("Requested basis set is")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 5) {
            QList<Data::RemSection*> rem(m_dataBank.findData<Data::RemSection>());
            if (!rem.isEmpty()) {
               method = rem.last()->value("method").toUpper();
               method += "/" + tokens[4];
               qDebug() << "Setting method to" << method;
            }

         }

      }else if (line.contains("Molecular Point Group")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() > 3 && currentGeometry) {
            Data::PointGroup& pg = currentGeometry->getProperty<Data::PointGroup>();
            pg.setValue(tokens[3]);
         }

      }else if (line.contains("beta electrons")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 6) {
            bool ok;
            m_nAlpha = tokens[2].toUInt(&ok);
            m_nBeta  = tokens[5].toUInt(&ok);
            currentGeometry->setMultiplicity(m_nAlpha-m_nBeta + 1);
         }

      }else if (line.contains("Total energy in the final basis set")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 9 && currentGeometry) {
            bool ok;
            double energy(tokens[8].toDouble(&ok));
            if (ok) {
               Data::ScfEnergy& scf(currentGeometry->getProperty<Data::ScfEnergy>());
               scf.setValue(energy, Data::Energy::Hartree);
               Data::TotalEnergy& total(currentGeometry->getProperty<Data::TotalEnergy>());
               total.setValue(energy, Data::Energy::Hartree);
            }
         }

      }else if (line.contains("RIMP2         total energy")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 5) setTotalEnergy(tokens[4], currentGeometry, "RIMP2");

      }else if (line.contains("RI-MP2 TOTAL ENERGY")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 5) setTotalEnergy(tokens[4], currentGeometry, "RIMP2");

      }else if (line.contains("Total SOS-MP2 energy")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 5) setTotalEnergy(tokens[4], currentGeometry, "SOS-MP2");

      }else if (line.contains("Total MOS-MP2 energy")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 5) setTotalEnergy(tokens[4], currentGeometry, "MOS-MP2");

      }else if (line.contains("TRIM MP2           total energy  =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 6) setTotalEnergy(tokens[5], currentGeometry, "TRIM-MP2");

      }else if (line.contains("MP2[V]      total energy")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 5) setTotalEnergy(tokens[4], currentGeometry, "MP2[V]");
 
      }else if (line.contains("MP2         total energy")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() >= 5) setTotalEnergy(tokens[4], currentGeometry, "MP2");

      }else if (line.contains("CCSD total energy          =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 5) setTotalEnergy(tokens[4], currentGeometry, "CCSD");

      }else if (line.contains("CCD total energy           =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 5) setTotalEnergy(tokens[4], currentGeometry, "CC");

      }else if (line.contains("EMP4                   =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 3) setTotalEnergy(tokens[2], currentGeometry, "MP4");

      }else if (line.contains("Energy is  ")) { 
         // Over-ride for geometry optimizations, which might be on an excited state
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 3) setTotalEnergy(tokens[2], currentGeometry);

      }else if (line.contains("Ground-State Mulliken Net Atomic Charges")) {
         textStream.skipLine(3);
         if (currentGeometry) readCharges(textStream, *currentGeometry, "Mulliken");

      }else if (line.contains("Ground-State ChElPG Net Atomic Charges")) {
         textStream.skipLine(3);
         if (currentGeometry) readCharges(textStream, *currentGeometry, "CHELPG");
 
      }else if (line.contains("Stewart Net Atomic Charges")) {
         textStream.skipLine(3);
         if (currentGeometry) readCharges(textStream, *currentGeometry, "Stewart");

      }else if (line.contains("Orbital Energies (a.u.) and Symmetries")) {
         textStream.skipLine(2);
         bool readSymmetries(true);
         readOrbitalSymmetries(textStream, readSymmetries);

      }else if (line.contains("Orbital Energies (a.u.)")) {
         textStream.skipLine(2);
         bool readSymmetries(false);
         readOrbitalSymmetries(textStream, readSymmetries);

      }else if (line.contains("TDDFT Excitation Energies")) {
         textStream.skipLine(2);
         readCisStates(textStream, Data::ExcitedStates::TDDFT);

      }else if (line.contains("CIS Excitation Energies") ||
                line.contains("TDDFT/TDA Excitation Energies")) {
         textStream.skipLine(2);
         readCisStates(textStream, Data::ExcitedStates::CIS);

      }else if (line.contains("CIS(D) Excitation Energies")) {
         textStream.skipLine(2);
         readCisdStates(textStream);
 
      }else if (line.contains("Reference values")) {
         textStream.skipLine(1);
         if (!nmr) nmr = new Data::Nmr;
         readNmrReference(textStream, *nmr);
    
      }else if (line.contains("ATOM           ISOTROPIC        ANISOTROPIC       REL.")) {
         textStream.skipLine(1);
         if (!nmr) nmr = new Data::Nmr;
         nmr->setMethod(method);
         if (currentGeometry) readNmrShifts(textStream, *currentGeometry, *nmr);

      }else if (line.contains("Indirect Nuclear Spin--Spin")) {
         textStream.skipLine(11);
         if (!nmr) nmr = new Data::Nmr;
         if (currentGeometry) readNmrCouplings(textStream, *currentGeometry, *nmr);

      }else if (line.contains("Cartesian Multipole Moments")) {
         textStream.skipLine(4);
         if (currentGeometry) readDipoleMoment(textStream, *currentGeometry);

      }else if (line.contains("Hessian of the SCF Energy") || 
                line.contains("Final Hessian.")) {
         if (currentGeometry) readHessian(textStream, *currentGeometry);

      }else if (line.contains("VIBRATIONAL ANALYSIS")) {
         textStream.seek("Mode:");
         readVibrationalModes(textStream);

      }else if (line.contains("DISTRIBUTED MULTIPOLE ANALYSIS")) {
         textStream.skipLine(4);
         if (currentGeometry) readDMA(textStream, *currentGeometry);

      // There is a typo in the print out of the word Coordinates
      }else if (line.contains("atoms in the effective region (ANGSTROMS)")) {
         textStream.skipLine();
         readEffectiveRegion(textStream);
      }
   }

   if (geometryList) {
      if (geometryList->isEmpty()) {
         delete geometryList;
      }else {
         geometryList->setDefaultIndex(-1);
         m_dataBank.append(geometryList);
      }
   }

   if (scanGeometries) {
      if (scanGeometries->isEmpty()) {
         delete scanGeometries;
      }else {
         m_dataBank.append(scanGeometries);
      }
   }

   if (nmr) {
      nmr->dump();
      m_dataBank.append(nmr);
   }

   return m_errors.isEmpty();
}



void QChemOutput::readCisStates(TextStream& textStream, 
   Data::ExcitedStates::ExcitedStatesT type)
{
   Data::ExcitedStates* states(new Data::ExcitedStates(type));
qDebug() << "Reading" << states->typeLabel() << "States";

   QStringList tokens;

   QString label, line;
   double energy(0.0), strength(0.0), s2(0.0);
   bool ok;
   qglviewer::Vec moment;

   // This is to match lines similar to the following
   //  D(  7) --> S(  1) amplitude =  0.6732 beta
   QRegExp 
      rx("([DS])\\(\\s*(\\d+)\\) --> ([VS])\\(\\s*(\\d+)\\) amplitude = (.{7})\\s?([ab]?)");
   
   while (!textStream.atEnd()) {
      tokens = textStream.nextLineAsTokens();
      int size(tokens.size());

      if (size == 0) {
         // do nothing
      }else if (tokens[0].contains("-------------------------------------------")) {
         break;

      }else if (size >= 8 && tokens[0].contains("Excited")) {
      
         energy = tokens[7].toDouble(&ok);
         if (!ok) goto error;

      }else if (size >= 2 && tokens[0].contains("Multiplicity")) {
         if (tokens[1].contains("Singlet")) {
            s2 = 0.0;
         }else if (tokens[1].contains("Triplet")) {
            s2 = 2.0;
         }else {
            s2 = tokens[1].toDouble(&ok);
            if (!ok) {
               QLOG_WARN() << "Failed to convert S^2 value for transition";
               s2 = 0.0;
            }
         }

         label = tokens[1];

      }else if (size >= 8 && tokens[0].contains("Trans")) {

         moment.x = tokens[2].toDouble(&ok);
         if (!ok) goto error;

         moment.y = tokens[4].toDouble(&ok);
         if (!ok) goto error;

         moment.z = tokens[6].toDouble(&ok);
         if (!ok) goto error;

      }else if (size >= 3 && tokens[0].contains("Strength")) {
         strength = tokens[2].toDouble(&ok);
         if (!ok) goto error;

         Data::ElectronicTransition* transition(
            new Data::ElectronicTransition(energy, strength, moment, s2));

         while (!textStream.atEnd()) {
            line = textStream.nextLine();
            if (rx.indexIn(line,0) == -1) break;
            if (!transition->addAmplitude(rx.capturedTexts().mid(1), m_nAlpha, m_nBeta)) {
               goto error;
            }
         }
     
         states->append(transition);
      }
   }

   //states->dump();

   if (states->nTransitions() > 0) {
      m_dataBank.append(states);
   }

   return;

   error:
     QString msg("Problem parsing excited states section, line number ");
     m_errors.append(msg += QString::number(textStream.lineNumber()));
}


void QChemOutput::readCisdStates(TextStream& textStream)
{
qDebug() << "Reading CIS(D) Energies";
   // We should already have the CIS energies lying around
   QList<Data::ExcitedStates*> es(m_dataBank.findData<Data::ExcitedStates>());
   if (es.isEmpty()) return;

   Data::ExcitedStates* states(es.last());
   unsigned nStates(states->nTransitions());
   if (nStates == 0) return;

   bool ok;
   QString line;
   double energy;
   unsigned nFound(0);
   QStringList tokens;

   QList<double> singlets;
   QList<double> triplets;


   while (!textStream.atEnd() && nFound < nStates) {
      line = textStream.seek("CIS(D) excitation energy for state");
      tokens = TextStream::tokenize(line);

	  // We need to account for the restricted and unrestricted separately as
	  // the order in which the states are printed differs.
      if (tokens.size() == 10) {      // Restricted case, includes "singlet"/"triplet"
         energy = tokens[8].toDouble(&ok);
         if (!ok) goto error;

         if (tokens[6] == "triplet") {
            triplets.append(energy);
         }else {
            singlets.append(energy);
         }
         ++nFound;
      }else if (tokens.size() == 9) { // Unrestricted case
         energy = tokens[7].toDouble(&ok);
         if (!ok) goto error;
         singlets.append(energy);
         ++nFound;
      }
   }
   
   if (nFound != nStates) goto error;

   states->setCisdEnergies(singlets, triplets);
   return;

   error:
     QString msg("Problem parsing excited states section, line number ");
     m_errors.append(msg += QString::number(textStream.lineNumber()));
}


void QChemOutput::readOrbitalSymmetries(TextStream& textStream, bool const readSymmetries)
{
   qDebug() << "Reading orbital energies";
   // We only parse the orbital symmetries section if we have excited states
   QList<Data::ExcitedStates*> es(m_dataBank.findData<Data::ExcitedStates>());
   if (es.isEmpty()) return;

   Data::OrbitalSymmetries& data(es.last()->orbitalSymmetries());
   Data::Spin spin(Data::Alpha);

   unsigned nOrb(0);
   QStringList tokens;
   QStringList symmetries;

   while (!textStream.atEnd()) {
      tokens = textStream.nextLineAsTokens(); 

      if (tokens.isEmpty()) {
         // do nothing
      }else if (tokens[0].contains("--------------")) {
         break;
      }else if (tokens[0].contains("Alpha")) {
         spin = Data::Alpha;
      }else if (tokens[0].contains("Beta")) {
         spin = Data::Beta;

      }else if (tokens[0] == ("--") && tokens[1].contains("Occupied")) {
         nOrb = 0;
      }else if (tokens[0] == ("--") && tokens[1].contains("Virtual")) {
         data.setOccupied(spin, nOrb);
      }else {
         if (readSymmetries) {
            symmetries = textStream.nextLineAsTokens();
            if (2*tokens.size() != symmetries.size()) goto error;
         }

         bool ok;
         double energy;
         QString symmetry;

         for (int i = 0; i < tokens.size(); ++i) {
             energy   = tokens[i].toDouble(&ok);   if (!ok) goto error;
             if (readSymmetries) {
                symmetry = symmetries.at(2*i);
                symmetry += " " + symmetries.at(2*i + 1);
             }else {
                symmetry = "A";
             }
             data.append(spin, energy, symmetry);
             ++nOrb;
         }
      } 
   }

   return;

   error:
     QString msg("Problem parsing excited states section, line number ");
     m_errors.append(msg += QString::number(textStream.lineNumber()));
}


void QChemOutput::readDMA(TextStream& textStream, Data::Geometry& geometry)
{
   Data::MultipoleExpansionList* dma(new Data::MultipoleExpansionList);
   Data::MultipoleExpansion* site;
   
   QList<double> x;
   QString line;

   // Charges and positions
   while (!textStream.atEnd()) {
      x = textStream.nextLineAsDoubles();
      if (x.size() < 5) break;
      site = new Data::MultipoleExpansion(Vec(x[1], x[2], x[3]));
      site->addCharge(x[4]);
      dma->append(site);
   }

   if (dma->isEmpty()) {
      delete dma;
      return;
   }

   geometry.appendProperty(dma);

   line = textStream.nextLine();
   if (!line.contains("DIPOLES")) return;
   textStream.skipLine(2);

   int nSites(0);
   while (!textStream.atEnd()) {
      x = textStream.nextLineAsDoubles();
      if (x.size() < 4) break;
      dma->at(nSites)->addDipole(x[1], x[2], x[3]);
      ++nSites;
   }

   // Digest the nuclear and electronic charges, which are given separately
   if (nSites < dma->size()) {
      int nAtoms(dma->size()-nSites);
      for (int i = nAtoms-1; i >= 0; --i) {
          site = dma->takeLast();
          *(dma->at(i)) += *site;
      //    delete site;
      }
   }else {
      goto error;
   }

   line = textStream.nextLine();
   if (!line.contains("QUADRUPOLES")) return;
   textStream.skipLine(2);

   for (int i = 0; i < nSites; ++i) {
       x = textStream.nextLineAsDoubles();
       if (x.size() < 5)  goto error;
       dma->at(i)->addQuadrupole(x[1], x[2], x[3], x[4], 0.0, 0.0);
   }

   textStream.skipLine(2);
   for (int i = 0; i < nSites; ++i) {
       x = textStream.nextLineAsDoubles();
       if (x.size() < 3)  goto error;
       dma->at(i)->addQuadrupole(0.0, 0.0, 0.0, 0.0, x[1], x[2]);
   }

   textStream.skipLine();
   line = textStream.nextLine();
   if (!line.contains("OCTUPOLES")) return;

   textStream.skipLine(2);
   for (int i = 0; i < nSites; ++i) {
       x = textStream.nextLineAsDoubles();
       if (x.size() < 5)  goto error;
       dma->at(i)->addOctopole(x[1], x[2], x[3], x[4], 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
   }

   textStream.skipLine(2);
   for (int i = 0; i < nSites; ++i) {
       x = textStream.nextLineAsDoubles();
       if (x.size() < 5)  goto error;
       dma->at(i)->addOctopole(0.0, 0.0, 0.0, 0.0, x[1], x[2], x[3], x[4], 0.0, 0.0);
   }

   textStream.skipLine(2);
   for (int i = 0; i < nSites; ++i) {
       x = textStream.nextLineAsDoubles();
       if (x.size() < 3)  goto error;
       dma->at(i)->addOctopole(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, x[1], x[2]);
   }

   return;

   error:
     QString msg("Problem parsing DMA, line number ");
     m_errors.append(msg += QString::number(textStream.lineNumber()));
}


void QChemOutput::setTotalEnergy(QString const& field, Data::Geometry* geometry, 
   QString const& label)
{
   if (!geometry) return;
   bool ok;
   double energy(field.toDouble(&ok));
   if (!ok) return;
   Data::TotalEnergy& data(geometry->getProperty<Data::TotalEnergy>());
   data.setValue(energy, Data::Energy::Hartree);
   if (!label.isEmpty()) data.setLabel(label + " energy");
}


void QChemOutput::readEffectiveRegion(TextStream& textStream)
{
   CartesianCoordinates parser;
   Data::Geometry* geometry(parser.parse(textStream));
   QList<Data::EfpFragmentList*> fragmentLists(m_dataBank.findData<Data::EfpFragmentList>());

   if (fragmentLists.isEmpty() || !geometry) {
      m_errors.append("Problem parsing EFP coordinates");
      delete geometry;
      return;
   }

   Data::EfpFragmentList* list(fragmentLists.last());
   Data::EfpFragmentLibrary& library(Data::EfpFragmentLibrary::instance());
   QList<Vec> coordinates(geometry->coordinates());
   delete geometry;

   QString name;
   int n, count(0);

   qDebug() << "Initial EFP alignment:";
   list->dump();

   Data::EfpFragmentList::iterator fragment;
   for (fragment = list->begin(); fragment != list->end(); ++fragment) {
       name = (*fragment)->name();
       // need to determine how many atoms are in the fragment
       if (!library.defined(name)) {
          m_errors.append("Fragment not found in library");
          return;
       }

       n = library.geometry(name).nAtoms();
       if (count + n > coordinates.size()) {
          m_errors.append("Too many EFP coordinates found");
          return;
       }
       
       if (!(*fragment)->align(coordinates.mid(count,n))) {
          m_errors.append("Problem with EFP alignment");
          return;
       }
       count += n;
   }

   qDebug() << "New EFP alignment:";
   list->dump();
}


void QChemOutput::readVibrationalModes(TextStream& textStream)
{
   QString line;
   QStringList tokens;
   double x, y, z, w;
   double zpve(0.0), enthalpy(0.0), entropy(0.0);
   bool ok;

   Data::VibrationalMode* v1(0);
   Data::VibrationalMode* v2(0);
   Data::VibrationalMode* v3(0);
   Data::Frequencies* frequencies = new Data::Frequencies;

   while (!textStream.atEnd()) {
      // Frequency:
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() < 2 || !tokens[0].contains("Frequency:")) break;

      v1 = 0;  v2 = 0;  v3 = 0;

      if (tokens.size() == 4) {
         w = tokens[3].toDouble(&ok);
         if (!ok) goto error;
         v3 = new Data::VibrationalMode(w);
      }
      if (tokens.size() >= 3) {
         w = tokens[2].toDouble(&ok);
         if (!ok) goto error;
         v2 = new Data::VibrationalMode(w);
      }
      if (tokens.size() >= 2) {
         w = tokens[1].toDouble(&ok);
         if (!ok) goto error;
         v1 = new Data::VibrationalMode(w);
      }

     // Skip Force Cnst and Red. Mass
      textStream.skipLine(2);

      // IR Active:
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() < 3 || !tokens[1].contains("Active:")) goto error;
      if (tokens.size() == 5 && v3) v3->setIrActive(tokens[4].contains("YES"));
      if (tokens.size() >= 4 && v2) v2->setIrActive(tokens[3].contains("YES"));
      if (tokens.size() >= 3 && v1) v1->setIrActive(tokens[2].contains("YES"));

      // IR Intens:
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() < 3 || !tokens[1].contains("Intens:")) goto error;

      if (tokens.size() == 5) {
         w = tokens[4].toDouble(&ok);
         if (!ok) goto error;
         if (v3) v3->setIntensity(w);
      }
      if (tokens.size() >= 4) {
         w = tokens[3].toDouble(&ok);
         if (!ok) goto error;
         if (v2) v2->setIntensity(w);
      }
      if (tokens.size() >= 3) {
         w = tokens[2].toDouble(&ok);
         if (!ok) goto error;
         if (v1) v1->setIntensity(w);
      }

      // Raman Active:
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() < 2 || !tokens[0].contains("Raman")) goto error;
      if (tokens.size() == 5 && v3) v3->setRamanActive(tokens[4].contains("YES"));
      if (tokens.size() >= 4 && v2) v2->setRamanActive(tokens[3].contains("YES"));
      if (tokens.size() >= 3 && v1) v1->setRamanActive(tokens[2].contains("YES"));

      // Raman Intens: (maybe)
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() > 2 && tokens[1].contains("Intens:")) {
         frequencies->haveRaman(true);
         if (tokens.size() == 5) {
            w = tokens[4].toDouble(&ok);
            if (!ok) goto error;
            if (v3) v3->setRamanIntensity(w);
         }
         if (tokens.size() >= 4) {
            w = tokens[3].toDouble(&ok);
            if (!ok) goto error;
            if (v2) v2->setRamanIntensity(w);
         }
         if (tokens.size() >= 3) {
            w = tokens[2].toDouble(&ok);
            if (!ok) goto error;
            if (v1) v1->setRamanIntensity(w);
         }
         // Skip Depolar and X Y Z lines
         textStream.skipLine(2); 
      }

      // Eigenvectors
      while (!textStream.atEnd()) {
         tokens = textStream.nextLineAsTokens();
         if (tokens.size() < 4 || tokens[0].contains("TransDip")) break;

         if (tokens.size() == 10) {
            x = tokens[7].toDouble(&ok);  if (!ok) goto error;
            y = tokens[8].toDouble(&ok);  if (!ok) goto error;
            z = tokens[9].toDouble(&ok);  if (!ok) goto error;
            if (v3) v3->appendDirectionVector(Vec(x,y,z)); 
         }
         if (tokens.size() >= 7) {
            x = tokens[4].toDouble(&ok);  if (!ok) goto error;
            y = tokens[5].toDouble(&ok);  if (!ok) goto error;
            z = tokens[6].toDouble(&ok);  if (!ok) goto error;
            if (v2) v2->appendDirectionVector(Vec(x,y,z)); 
         }
         if (tokens.size() >= 4) {
            x = tokens[1].toDouble(&ok);  if (!ok) goto error;
            y = tokens[2].toDouble(&ok);  if (!ok) goto error;
            z = tokens[3].toDouble(&ok);  if (!ok) goto error;
            if (v1) v1->appendDirectionVector(Vec(x,y,z)); 
         }
      }
      
      if (v1) { frequencies->append(v1);  v1 = 0; }
      if (v2) { frequencies->append(v2);  v2 = 0; }
      if (v3) { frequencies->append(v3);  v3 = 0; }

      textStream.skipLine(2);
   }

   line = textStream.seek("Zero point vibrational energy:");
   tokens = TextStream::tokenize(line);
   if (tokens.size() == 6) zpve = tokens[4].toDouble(); 

   line = textStream.seek("Total Enthalpy:");
   tokens = TextStream::tokenize(line);
   if (tokens.size() == 4) enthalpy = tokens[2].toDouble(); 

   line = textStream.seek("Total Entropy:");
   tokens = TextStream::tokenize(line);
   if (tokens.size() == 4) entropy = tokens[2].toDouble(); 

   frequencies->setThermochemicalData(zpve, entropy, enthalpy);
   m_dataBank.append(frequencies);
   return;

   error:
     QString msg("Problem parsing vibrational frequencies, line ");
     m_errors.append(msg += QString::number(textStream.lineNumber()));
qDebug() << "ERROR: " << msg;
     if (v1) delete v1;
     if (v2) delete v2;
     if (v3) delete v3;
     delete frequencies;
}


void QChemOutput::readDipoleMoment(TextStream& textStream, Data::Geometry& geometry)
{
   QStringList tokens;
   tokens = textStream.nextLineAsTokens();
   if (tokens.size() != 6) goto error;

   bool ok;
   double x, y, z;

   x = tokens[1].toDouble(&ok);  if (!ok) goto error;
   y = tokens[3].toDouble(&ok);  if (!ok) goto error;
   z = tokens[5].toDouble(&ok);  if (!ok) goto error;

   geometry.getProperty<Data::DipoleMoment>().setValue(x,y,z);
   return;

   error:
      QString msg("Problem parsing dipole moment, line number ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
}


void QChemOutput::readHessian(TextStream& textStream, Data::Geometry& geometry)
{
   unsigned nAtoms(geometry.nAtoms());
   Matrix hessian(3*nAtoms, 3*nAtoms);
   QStringList tokens;
   QList<double> values;

   unsigned firstCol(0);
   unsigned lastCol(0);
   unsigned nCol(0);

   while (firstCol < 3*nAtoms) {
       // header line
       tokens  = textStream.nextLineAsTokens();
       nCol    = tokens.size();
       lastCol = firstCol + tokens.size();

       for (unsigned row = 0; row < 3*nAtoms; ++row) {
           values = textStream.nextLineAsDoubles();
           if ((unsigned)values.size() != lastCol-firstCol+1) {  // +1 for the row index
              QString msg("Problem parsing hessian, line number ");
              m_errors.append(msg + QString::number(textStream.lineNumber()));
              return;
           }
           values.removeFirst();
           for (unsigned col = firstCol; col < lastCol; ++col) {
               hessian(row, col) = values.takeFirst();
           }
       }
       firstCol += nCol;
   }

   Data::Hessian& h(geometry.getProperty<Data::Hessian>());
   h.setData(hessian);
}


void QChemOutput::readNmrReference(TextStream& textStream, Data::Nmr& nmr)
{
   qDebug() << "Reading NMR Reference";

   int n;
   QString atom;
   double shift(0.0), offset(0.0);
   bool done(false), ok(true);
   QStringList tokens;

   Data::NmrReference ref;

   QString method;

   while (!textStream.atEnd() && !done && ok) {
      tokens = textStream.nextLineAsTokens();
      n = tokens.size();

      if (n >= 3) {
         if (tokens[0] == "Reference") {
            ref.setSystem(tokens[2]);
         }

      }else if (n >= 2) {
         if (tokens[0] == "Atom:") {
            atom = tokens[1];
         }else if (tokens[0] == "Method:") {
            method = tokens[1];
         }else if (tokens[0] == "Basis:") {
            method += "/" + tokens[1];
            ref.setMethod(method);
         }else if (tokens[0] == "Shift:") {
            shift = tokens[1].toDouble(&ok);
         }else if (tokens[0] == "Offset:") {
            offset = tokens[1].toDouble(&ok);
            ref.addElement(atom, shift, offset);
         }
      
      } else if (n > 0 && tokens[0].contains("-------")) {
         done = true;
      }
   }

   if (ok) {
      nmr.setReference(ref);
   }else {
      m_errors.append("Problem parsing NMR reference ");
   }
}


void QChemOutput::readNmrShifts(TextStream& textStream, Data::Geometry& geometry, 
   Data::Nmr& nmr)
{
   qDebug() << "Reading NMR Shifts";
   QStringList tokens;

   QList<double> shieldings;
   QList<double> shifts;
   QStringList   atomicSymbols;

   int n;
   bool done(false), allOk(true), ok;
   bool haveShifts(false);

   while (!textStream.atEnd() && !done && allOk) {
      tokens = textStream.nextLineAsTokens();
      n = tokens.size();

      if (n == 6) {
         haveShifts = true;
         shifts.append(tokens[5].toDouble(&ok)); 
         allOk = allOk && ok;
      }else if (n == 5) {
         shifts.append(0.0);
      }

      if (n >= 5) {
         shieldings.append(tokens[3].toDouble(&ok)); 
         allOk = allOk && ok;
         atomicSymbols.append(tokens[1]);
      }else {
         done = true;
      }
   }

   if (!allOk) {
      QString msg("Problem parsing NMR shifts, line number ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
      return;
   }

   if (!geometry.sameAtoms(atomicSymbols)) {
      QString msg("Atom list mismatch around line number: ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
      return;
   }

   qDebug() << "Setting Shieldings" << shieldings;
   allOk = geometry.setAtomicProperty<Data::NmrShielding>(shieldings);

   if (allOk) { 
      nmr.setAtomLabels(atomicSymbols);
      nmr.setShieldings(shieldings);
   }else {
      m_errors.append("Failed to read NMR shieldings");
   }

   if (haveShifts) {
      nmr.setShifts(shifts);
      qDebug() << "Setting chemical shifts" << shifts;
      allOk = geometry.setAtomicProperty<Data::NmrShift>(shifts);
      if (!allOk) m_errors.append("Failed to read NMR shifts");
   }
}


void QChemOutput::readNmrCouplings(TextStream& textStream, Data::Geometry& geometry, 
   Data::Nmr& nmr)
{
   qDebug() << "Reading NMR coupling constants";

   QStringList tokens;

   bool done(false), allOk(true);
   unsigned nAtoms(geometry.nAtoms());

   Matrix* couplings = new Matrix(nAtoms, nAtoms);
   for (unsigned i = 0; i < nAtoms; ++i) {
       for (unsigned j = 0; j < nAtoms; ++j) {
           (*couplings)(i,j) = 0.0;
       }
   }

   QRegExp rx("#(\\d+)");

   while (!textStream.atEnd() && !done && allOk) {
      tokens = textStream.nextNonEmptyLineAsTokens();

      if (tokens.first() == "Atoms" && tokens.size() > 6) {

         int atom1(0), atom2(0);
         if (allOk && rx.indexIn(tokens[2],0) != -1) {
             atom1 = rx.capturedTexts().at(1).toInt(&allOk);
         }
         if (allOk && rx.indexIn(tokens[5],0) != -1) {
            atom2 = rx.capturedTexts().at(1).toInt(&allOk);
         }

         if (atom1 > 0 && atom2 > 0 && allOk) {
            textStream.seek("Total Spin-Spin Coupling Tensor");
            textStream.skipLine(4);
    
            tokens = textStream.nextLineAsTokens();
            if (tokens.size() > 2) {
               (*couplings)(atom1-1, atom2-1) = tokens[1].toDouble(&allOk);
               (*couplings)(atom2-1, atom1-1) = (*couplings)(atom1-1, atom2-1);
            }

         }else {
            allOk = false;
            break;
         }
  
         
      }else if (tokens.first() == "MO-PROP") {
         done = true;
      }
   }

   if (allOk) { 
      nmr.setCouplings(*couplings);
      // This is a bit of a hack.  The couplings calculation is done seperately
      // and so the shifts/shieldings are possibly not available.
      geometry.setAtomicProperty<Data::NmrShift>(nmr.shifts());
      geometry.setAtomicProperty<Data::NmrShielding>(nmr.shieldings());
   }else {
      m_errors.append("Failed to read NMR ISS couplings");
   }

   delete couplings;
}


void QChemOutput::readCharges(TextStream& textStream, Data::Geometry& geometry, 
   QString const& label)
{
   QStringList tokens;

   QList<double> charges;
   QList<double> spins;
   QStringList atomicSymbols;

   int n;
   bool done(false), allOk(true), ok;

   while (!textStream.atEnd() && !done && allOk) {
      tokens = textStream.nextLineAsTokens();
      n = tokens.size();

      if (n == 4) {
         spins.append(tokens[3].toDouble(&ok)); 
         allOk = allOk && ok;
      }

      if (n >=3) {
         charges.append(tokens[2].toDouble(&ok)); 
         allOk = allOk && ok;
         atomicSymbols.append(tokens[1]);
      }else {
         done = true;
      }
   }

   if (!allOk) {
      QString msg("Problem parsing charges, line number: ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
      return;
   }

   if (!geometry.sameAtoms(atomicSymbols)) {
      QString msg("Atom list mismatch around line number: ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
      return;
   }

   if (label == "Mulliken") {
      allOk = geometry.setAtomicProperty<Data::MullikenCharge>(charges);
      if (allOk && !spins.isEmpty()) {
         allOk = allOk && geometry.setAtomicProperty<Data::SpinDensity>(spins);
      }
   }else if (label == "Stewart") {
      allOk = geometry.setAtomicProperty<Data::MultipoleDerivedCharge>(charges);
   }else if (label == "CHELPG") {
      allOk = geometry.setAtomicProperty<Data::ChelpgCharge>(charges);
   }else {
      m_errors.append("Unknown charge type");
   }

   if (allOk) {
      double q(0.0);
      for (int i = 0; i < charges.size(); ++i) q += charges[i];
      geometry.setCharge(Util::round(q));
qDebug() << "Setting charge to:" << Util::round(q);
      
   }else {
      m_errors.append("Problem setting atomic charges");
   }
}


Data::Geometry* QChemOutput::readStandardCoordinates(TextStream& textStream)
{  
   CartesianCoordinates parser;
   Data::Geometry* geometry(parser.parse(textStream));
   if (geometry) {
      QString error(parser.error());
      if (!error.isEmpty()) {
         m_errors.append(error);
         delete geometry;
         geometry = 0;
      }
   }else {
      m_errors.append("No coordinates found");
   }

   return geometry;
} 

} } // end namespace IQmol::Parser
