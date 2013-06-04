/*******************************************************************************
         
  Copyright (C) 2011 Andrew Gilbert
      
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
#include "MolecularOrbitalsLayer.h"
#include "AtomLayer.h"
#include "InfoLayer.h"
#include "IQmol.h"
#include "QsLog.h"
#include <QTextStream>
#include <QString>


namespace IQmol {
namespace Parser {

DataList FormattedCheckpoint::parse(QTextStream& textStream)
{
   m_parseOkay = true;

   while (!textStream.atEnd() && m_parseOkay) {
      processLine(textStream);
   }

   if (m_parseOkay) {
      makeAtomList();
      generateShells();
      // Note that this eats the coefficient arrays
      Layer::MolecularOrbitals* mos = new Layer::MolecularOrbitals(
         m_nAlpha, m_nBeta, m_nBasis, m_alphaCoefficients, m_alphaEnergies,
         m_betaCoefficients, m_betaEnergies, m_shells);
      m_dataList.append(mos);
   }else {
      throw FormatError();
   }

   return m_dataList;
}


void FormattedCheckpoint::makeAtomList()
{
   int nAtoms(m_atomicNumbers.size());
   if (nAtoms == 0 || m_coordinates.size() != 3*nAtoms) return;

   Layer::Atoms* atoms = new Layer::Atoms;
   Layer::Atom* atom;
   double x, y, z;

   for (int i = 0; i < m_atomicNumbers.size(); ++i) {
       atom = new Layer::Atom(m_atomicNumbers[i]);    
       x = m_coordinates[3*i+0];
       y = m_coordinates[3*i+1];
       z = m_coordinates[3*i+2];
       atom->setPosition(BohrToAngstrom*qglviewer::Vec(x,y,z));
       atoms->appendLayer(atom);
   }

   Layer::Info* info = new Layer::Info();
   info->addAtoms(atoms->getAtoms());
   m_dataList.append(info);
   m_dataList.append(atoms);
}



void FormattedCheckpoint::processLine(QTextStream& textStream)
{
   QString line = textStream.readLine();
   QString key = line;
   key.resize(42);
   key = key.trimmed();
   QString tmp = line.mid(43, 37);
   QStringList list = tmp.split(QRegExp("\\s+"), QString::SkipEmptyParts);

   if (key == "Number of alpha electrons") {
      m_nAlpha = list.at(1).toInt();

   }else if (key == "Number of beta electrons") {
      m_nBeta = list.at(1).toInt();

   }else if (key == "Number of basis functions") {
      m_nBasis = list.at(1).toInt();

   }else if (key == "Atomic numbers") {
      m_atomicNumbers = readIntegerArray(textStream, list.at(2).toInt());

   } else if (key == "Current cartesian coordinates") {
      m_coordinates = readDoubleArray(textStream, list.at(2).toInt());

   }else if (key == "Shell types") {
      m_shellTypes = readIntegerArray(textStream, list.at(2).toInt());

   }else if (key == "Number of primitives per shell") {
      m_shellPrimitives = readIntegerArray(textStream, list.at(2).toInt());

   }else if (key == "Shell to atom map") {
      m_shellToAtom = readIntegerArray(textStream, list.at(2).toInt());

   }else if (key == "Primitive exponents") {
      m_exponents = readDoubleArray(textStream, list.at(2).toInt());

   }else if (key == "Contraction coefficients") {
      m_contractionCoefficients = readDoubleArray(textStream, list.at(2).toInt());

   }else if (key == "P(S=P) Contraction coefficients") {
      m_contractionCoefficientsSP = readDoubleArray(textStream, list.at(2).toInt());

   }else if (key == "SCF Energy") {
      m_energy = list.at(1).toDouble();

   }else if (key == "Alpha MO coefficients") {
      m_alphaCoefficients = readDoubleArray(textStream, list.at(2).toInt());

   }else if (key == "Beta MO coefficients") {
      m_betaCoefficients = readDoubleArray(textStream, list.at(2).toInt());

   }else if (key == "Alpha Orbital Energies") {
      m_alphaEnergies = readDoubleArray(textStream, list.at(2).toInt());

   }else if (key == "Beta Orbital Energies") {
      m_betaEnergies = readDoubleArray(textStream, list.at(2).toInt());
   }
}


QList<int> FormattedCheckpoint::readIntegerArray(QTextStream& textStream, int n)
{
   QList<int> tmp;
   while (tmp.size() < n && m_parseOkay) {
      QString line = textStream.readLine();
      QStringList list = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      for (int i = 0; i < list.size(); ++i) {
          tmp.append(list.at(i).toInt(&m_parseOkay));
      }
   }
   return tmp;
}


QList<double> FormattedCheckpoint::readDoubleArray(QTextStream& textStream, int n)
{
   QList<double> tmp;
   while (tmp.size() < n && m_parseOkay) {
      QString line = textStream.readLine();
      QString token; 

      while (!line.isEmpty() && m_parseOkay) {
         token = line.left(16);
         tmp.append(token.toDouble(&m_parseOkay));
         line.remove(0,token.size());
      }
   }
   return tmp;
}


void FormattedCheckpoint::generateShells() 
{
   unsigned int cnt(0);
   QList<double> expts;
   QList<double> coefs;
   QList<double> coefsSP;
   // Conversion factor for the exponents
   double conv(1.0/(BohrToAngstrom*BohrToAngstrom));

   unsigned nBasis(0);

   for (int shell = 0; shell < m_shellTypes.size(); ++shell) {
       expts.clear();
       coefs.clear();
       coefsSP.clear();

       int atom(m_shellToAtom.at(shell)-1);
       qglviewer::Vec position(m_coordinates.at(3*atom), 
                               m_coordinates.at(3*atom+1), 
                               m_coordinates.at(3*atom+2));

       position *= BohrToAngstrom;

       for (int i = 0; i < m_shellPrimitives.at(shell); ++i) {
           expts.append(m_exponents.at(cnt)*conv);
           coefs.append(m_contractionCoefficients.at(cnt));
           if (!m_contractionCoefficientsSP.isEmpty()) {
              coefsSP.append(m_contractionCoefficientsSP.at(cnt));
           }
           ++cnt;
       }

       switch (m_shellTypes.at(shell)) {
          case 0:
             m_shells.append( new Shell(Shell::S, position, expts, coefs) );
             nBasis += 1;
             break;
          case 1:
             m_shells.append( new Shell(Shell::P, position, expts, coefs) );
             nBasis += 3;
             break;
          case -1:
             m_shells.append( new Shell(Shell::S, position, expts, coefs)   );
             m_shells.append( new Shell(Shell::P, position, expts, coefsSP) );
             nBasis += 4;
             break;
          case 2:
             m_shells.append( new Shell(Shell::D6, position, expts, coefs) );
             nBasis += 6;
             break;
          case -2:
             m_shells.append( new Shell(Shell::D5, position, expts, coefs) );
             nBasis += 5;
             break;
          case 3:
             m_shells.append( new Shell(Shell::F10, position, expts, coefs) );
             nBasis += 10;
             break;
          case -3:
             m_shells.append( new Shell(Shell::F7, position, expts, coefs) );
             nBasis += 7;
             break;

          default:
             QLOG_ERROR() << "Unknown Shell type found";
             break;
       }
   }

   if (nBasis != m_nBasis) { 
      QLOG_ERROR() << "Formatted Checkpoint File: nBasis read: " << m_nBasis 
                                                  << "tallied: " << nBasis;
      qDebug() << "Number of basis functions read  " << m_nBasis;
      qDebug() << "Number of basis functions tally " << nBasis;
      throw FormatError();
   }
}


} } // end namespace IQmol::Parser
