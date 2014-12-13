/*******************************************************************************
       
  Copyright (C) 2011-13 Andrew Gilbert
           
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
#include "Frequencies.h"
#include "Hessian.h"
#include "Energy.h"
#include "PointGroup.h"
#include "DipoleMoment.h"
#include "EfpFragmentLibrary.h"
#include "MultipoleExpansion.h"
#include "EfpFragment.h"
#include "Constants.h"
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
        errors.append("Job failed");
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
qDebug() << "Creating new geometry list";
               geometryList = new Data::GeometryList;
               currentGeometry = 0;
            }

            if (geometry->sameAtoms(*firstGeometry)) {
qDebug() << "Appending geometry to list";
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

      }else if (line.contains("Molecular Point Group")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() > 3 && currentGeometry) {
            Data::PointGroup& pg = currentGeometry->getProperty<Data::PointGroup>();
            pg.setValue(tokens[3]);
         }

      }else if (line.contains("Total energy in the final basis set")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 9 && currentGeometry) {
            bool ok;
            double energy(tokens[8].toDouble(&ok));
            if (ok) {
               Data::ScfEnergy& scf = currentGeometry->getProperty<Data::ScfEnergy>();
               scf.setValue(energy, Data::Energy::Hartree);
               Data::TotalEnergy& total = currentGeometry->getProperty<Data::TotalEnergy>();
               total.setValue(energy, Data::Energy::Hartree);
qDebug() << "Setting total energy to" << total.value();
            }
         }

      }else if (line.contains("MP2         total energy =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 6) setTotalEnergy(tokens[4], currentGeometry);

      }else if (line.contains("RIMP2         total energy")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 6) setTotalEnergy(tokens[4], currentGeometry);

      }else if (line.contains("CCSD total energy          =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 5) setTotalEnergy(tokens[4], currentGeometry);

      }else if (line.contains("CCD total energy           =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 5) setTotalEnergy(tokens[4], currentGeometry);

      }else if (line.contains("EMP4                   =")) {
         tokens = TextStream::tokenize(line);
         if (tokens.size() == 3) setTotalEnergy(tokens[2], currentGeometry);

      }else if (line.contains("Ground-State Mulliken Net Atomic Charges")) {
         textStream.skipLine(3);
         readCharges(textStream, currentGeometry, "Mulliken");

      }else if (line.contains("Ground-State ChElPG Net Atomic Charges")) {
         textStream.skipLine(3);
         readCharges(textStream, currentGeometry, "CHELPG");
 
      }else if (line.contains("Stewart Net Atomic Charges")) {
         textStream.skipLine(3);
         readCharges(textStream, currentGeometry, "Stewart");

      }else if (line.contains("ATOM           ISOTROPIC")) {
         textStream.skipLine(1);
         readNmrShifts(textStream, currentGeometry);

      }else if (line.contains("Cartesian Multipole Moments")) {
         textStream.skipLine(4);
         readDipoleMoment(textStream, currentGeometry);

      }else if (line.contains("Hessian of the SCF Energy") || 
                line.contains("Final Hessian.")) {
         readHessian(textStream, currentGeometry);

      }else if (line.contains("VIBRATIONAL ANALYSIS")) {
         textStream.skipLine(9);
         readVibrationalModes(textStream);

      }else if (line.contains("DISTRIBUTED MULTIPOLE ANALYSIS")) {
         textStream.skipLine(4);
         readDMA(textStream, currentGeometry);

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

   return m_errors.isEmpty();
}


void QChemOutput::readDMA(TextStream& textStream, Data::Geometry* geometry)
{
   Data::MultipoleExpansionList* dma(new Data::MultipoleExpansionList);
   Data::MultipoleExpansion* site;
   
   QList<double> x;
   QStringList tokens;
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

   geometry->appendProperty(dma);

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


void QChemOutput::setTotalEnergy(QString const& field, Data::Geometry* geometry)
{
   if (!geometry) return;

   bool ok;
   double energy(field.toDouble(&ok));
   if (!ok) return;
   Data::TotalEnergy& data(geometry->getProperty<Data::TotalEnergy>());
   data.setValue(energy, Data::Energy::Hartree);
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
      line = textStream.nextLine();
      if (!line.contains("Mode:")) break;

      // Frequency:
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() < 2 || !tokens[0].contains("Frequency:")) goto error;

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

      // IR Active:
      textStream.skipLine(2);
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

      // Eigenvectors
      textStream.skipLine(); 
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

      textStream.skipLine();
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
     if (v1) delete v1;
     if (v2) delete v2;
     if (v3) delete v3;
     delete frequencies;
}


void QChemOutput::readDipoleMoment(TextStream& textStream, Data::Geometry* geometry)
{
   if (!geometry) return;

   QStringList tokens;
   tokens = textStream.nextLineAsTokens();
   if (tokens.size() != 6) goto error;

   bool ok;
   double x, y, z;

   x = tokens[1].toDouble(&ok);  if (!ok) goto error;
   y = tokens[3].toDouble(&ok);  if (!ok) goto error;
   z = tokens[5].toDouble(&ok);  if (!ok) goto error;

   geometry->getProperty<Data::DipoleMoment>().setValue(x,y,z);
   return;

   error:
      QString msg("Problem parsing dipole moment, line number ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
}


void QChemOutput::readHessian(TextStream& textStream, Data::Geometry* geometry)
{
   if (!geometry) return;

   unsigned nAtoms(geometry->nAtoms());
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

   Data::Hessian& h(geometry->getProperty<Data::Hessian>());
   h.setData(hessian);
}


void QChemOutput::readNmrShifts(TextStream& textStream, Data::Geometry* geometry)
{
   if (!geometry) return;
   QStringList tokens;

   QList<double> isotropic;
   QList<double> relative;
   QList<QString> atomicSymbols;

   int n;
   bool done(false), allOk(true), ok;
   bool haveRelativeShifts(false);

   while (!textStream.atEnd() && !done && allOk) {
      tokens = textStream.nextLineAsTokens();
      n = tokens.size();

      if (n == 6) {
         haveRelativeShifts = true;
         relative.append(tokens[5].toDouble(&ok)); 
         allOk = allOk && ok;
      }else if (n == 5) {
         relative.append(0.0);
      }

      if (n >= 5) {
         isotropic.append(tokens[3].toDouble(&ok)); 
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

   if (!geometry->sameAtoms(atomicSymbols)) {
      QString msg("Atom list mismatch around line number: ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
      return;
   }

   allOk = geometry->setAtomicProperty<Data::NmrShiftIsotropic>(isotropic);

   if (!allOk) m_errors.append("Failed to read NMR Isotropic shifts");

   if (haveRelativeShifts) {
      allOk = geometry->setAtomicProperty<Data::NmrShiftRelative>(relative);
      if (!allOk) m_errors.append("Failed to read NMR Relative shifts");
   }
}


void QChemOutput::readCharges(TextStream& textStream, Data::Geometry* geometry, 
   QString const& label)
{
   if (!geometry) return;
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

   if (!geometry->sameAtoms(atomicSymbols)) {
      QString msg("Atom list mismatch around line number: ");
      m_errors.append(msg + QString::number(textStream.lineNumber()));
      return;
   }

   if (label == "Mulliken") {
      allOk = geometry->setAtomicProperty<Data::MullikenCharge>(charges);
      if (allOk && !spins.isEmpty()) {
         allOk = allOk && geometry->setAtomicProperty<Data::SpinDensity>(spins);
      }
   }else if (label == "Stewart") {
      allOk = geometry->setAtomicProperty<Data::MultipoleDerivedCharge>(charges);
   }else if (label == "CHELPG") {
      allOk = geometry->setAtomicProperty<Data::ChelpgCharge>(charges);
   }else {
      m_errors.append("Unknown charge type");
   }

   if (!allOk) m_errors.append("Problem setting atomic charges");
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
