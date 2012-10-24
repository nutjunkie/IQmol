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

#include "QChemParser.h"
#include "ExternalChargesParser.h"
#include "OpenBabelParser.h"
#include "openbabel/mol.h"
#include "openbabel/format.h"
#include "openbabel/obconversion.h"
#include <QTextStream>
#include <QString>
#include <QtDebug>


using namespace qglviewer;
using namespace OpenBabel;

namespace IQmol {
namespace Parser {


QStringList QChem::parseForErrors(QTextStream& textStream)
{
   QString line;
   QStringList errors;

   while (!textStream.atEnd()) {
      line = textStream.readLine();
      if (line.contains("Q-Chem fatal error")) {
         textStream.readLine().trimmed();  // blank line
         line = textStream.readLine().trimmed();
         if (line.isEmpty()) line = "Fatal error occured at end of output file";
         errors.append(line);
      }
   }
   return errors;
}


DataList QChem::parse(QTextStream& textStream)
{
   m_prependConformer = false;
   
   // Attempt to find conformers in an output file
   while (!textStream.atEnd()) {
      processLine(textStream);
   }

   m_dataList = m_inputDataList;

   if (!m_conformers.isEmpty()) {
      if (!m_defaultConformer) m_defaultConformer = m_currentConformer;
      m_dataList.append(new Layer::ConformerList(m_conformers, m_defaultConformer));
   }

   m_dataList += m_chargesList;

   return m_dataList;
}


void QChem::processLine(QTextStream& textStream)
{
   bool eOK;
   double energy;
   QString line = textStream.readLine().trimmed();

   if (line.contains("Standard Nuclear Orientation (Angstroms)") ) {
      readCoordinates(textStream);
      m_currentConformer = new Layer::Conformer(m_coordinates);

      if (m_prependConformer) {
         m_conformers.prepend(m_currentConformer);
      }else {
         m_conformers.append(m_currentConformer);
      }

   }else if (line.contains("$molecule", Qt::CaseInsensitive)) {
      MoleculeSection moleculeSection;
      m_inputDataList += moleculeSection.parse(textStream);

   }else if (line.contains("$external_charges", Qt::CaseInsensitive)) {
      ExternalCharges externalCharges;
      m_chargesList += externalCharges.parse(textStream);

   }else if (line.contains("Q-Chem fatal error")) {
     

   }else if (m_currentConformer) {  

      if (line.contains("IRC -- convergence criterion reached")) {
         // This is a bit of a hack to ensure the IRC path doesn't jump
         // when going from the forward to the backward direction.
         if (!m_prependConformer) m_defaultConformer = m_conformers.first();
         m_prependConformer = true;
      }else if (line.contains("Convergence criterion met")) {
         QStringList tokens(line.split(" ", QString::SkipEmptyParts));
         energy = tokens[1].toDouble(&eOK);
         if (!eOK) throw FormatError();
         m_currentConformer->setEnergy(energy);
      }else if (line.contains("MP2         total energy =")) {
         QStringList tokens(line.split(" ", QString::SkipEmptyParts));
         energy = tokens[4].toDouble(&eOK);
         if (!eOK) throw FormatError();
         m_currentConformer->setEnergy(energy);
      }else if (line.contains("Mulliken Net Atomic Charges")) {
         readMullikenCharges(textStream);
         m_currentConformer->setCharges(m_partialCharges, m_spinDensities);
      }else if (line.contains("Cartesian Multipole Moments")) {
         readMultipoles(textStream);
         m_currentConformer->setMultipoles(m_multipoles);
      }

   }

}


// Note that this only reads the coordinates for a Conformer.  
// Atom identities are loaded by Parser::OpenBabel later.
void QChem::readCoordinates(QTextStream& textStream)
{
   m_coordinates.clear();
   textStream.readLine();
   textStream.readLine();

   QString line;
   QStringList tokens;
   double x, y, z;
   bool xOK(true), yOK(true), zOK(true), allOK;

   while (!textStream.atEnd()) {
      line = textStream.readLine();
      if (line.contains("----------")) break;
      tokens = line.split(" ", QString::SkipEmptyParts);
      x = tokens[2].toDouble(&xOK); 
      y = tokens[3].toDouble(&yOK); 
      z = tokens[4].toDouble(&zOK); 

      allOK = xOK && yOK && zOK;
      if (!allOK) throw FormatError();
      m_coordinates.append(Vec(x, y, z));
   }
}


void QChem::readMullikenCharges(QTextStream& textStream)
{
   m_partialCharges.clear();
   m_spinDensities.clear();
   textStream.readLine();
   
   QStringList tokens;
   QString line(textStream.readLine());
   bool withSpin(line.contains("Spin"));
   textStream.readLine();

   double q, s;
   bool qOK(true), sOK(true), allOK;
qDebug() << "Parser::QChem with spin = " << withSpin;

   while (!textStream.atEnd()) {
      line = textStream.readLine();
      if (line.contains("----------")) return;
      tokens = line.split(" ", QString::SkipEmptyParts);
      q = tokens[2].toDouble(&qOK);
      s = withSpin ? tokens[3].toDouble(&sOK) : 0.0;

      allOK = qOK && sOK;
      if (!allOK) throw FormatError();

      m_partialCharges.append(q);
      m_spinDensities.append(s);
   }
}


void QChem::readMultipoles(QTextStream& textStream)
{
   m_multipoles.clear();
   textStream.readLine();
   textStream.readLine();
   
   QStringList tokens;
   double q, x, y, z, xx, xy, yy, xz, yz, zz;
   bool ok, allOK(true);

   // charge
   QString line(textStream.readLine());
   tokens = line.split(" ", QString::SkipEmptyParts);
   q = tokens[0].toDouble(&allOK);
   if (!allOK) throw FormatError();
   m_multipoles.append(q);

   line = textStream.readLine();
   if (line.contains("----------")) return;

   // dipole
   line = textStream.readLine();
   tokens = line.split(" ", QString::SkipEmptyParts);
   x = tokens[1].toDouble(&ok); allOK = allOK && ok;
   y = tokens[3].toDouble(&ok); allOK = allOK && ok;
   z = tokens[5].toDouble(&ok); allOK = allOK && ok;
   if (!allOK) throw FormatError();
   m_multipoles.append(x);
   m_multipoles.append(y);
   m_multipoles.append(z);
   
   textStream.readLine();
   line = textStream.readLine();
   if (line.contains("----------")) return;

   // quadrupole
   line = textStream.readLine();
   tokens = line.split(" ", QString::SkipEmptyParts);
   xx = tokens[1].toDouble(&ok); allOK = allOK && ok;
   xy = tokens[3].toDouble(&ok); allOK = allOK && ok;
   yy = tokens[5].toDouble(&ok); allOK = allOK && ok;

   line = textStream.readLine();
   tokens = line.split(" ", QString::SkipEmptyParts);
   xz = tokens[1].toDouble(&ok); allOK = allOK && ok;
   yz = tokens[3].toDouble(&ok); allOK = allOK && ok;
   zz = tokens[5].toDouble(&ok); allOK = allOK && ok;

   if (!allOK) throw FormatError();
   m_multipoles.append(xx);
   m_multipoles.append(xy);
   m_multipoles.append(yz);
   m_multipoles.append(xz);
   m_multipoles.append(yz);
   m_multipoles.append(zz);
}



DataList QChem::MoleculeSection::parse(QTextStream& textStream)
{
   QStringList lines;
   QString line;

   while (!textStream.atEnd()) {
      line = textStream.readLine().trimmed();
      if (line.contains("$end", Qt::CaseInsensitive)) break;
      lines << line;
   }

   if (lines.size() == 1 && lines[0].contains("read", Qt::CaseInsensitive)) {
      return DataList();
   }else if (lines.size() < 2) {
      throw FormatError();
   }
   
   QString first(lines[0].replace(QChar(','),QChar(' ')));
   QStringList tokens(first.split(QRegExp("\\s+"), QString::SkipEmptyParts));

   // Need at least the charge/multiplicity line and a single atom
   if (tokens.count() != 2) throw FormatError();

   bool ok1, ok2;
   int charge = tokens[0].toInt(&ok1);
   unsigned int multiplicity = tokens[1].toUInt(&ok2);
   if (!ok1 || !ok2) throw FormatError();

   // We take a peek at the first line of coordinates.  If there is only one
   // token then we assume a z-matrix format, otherwise we assume an XYZ.
   tokens.clear();
   tokens = lines[1].split(QRegExp("\\s+"), QString::SkipEmptyParts);

   OBConversion conv;
   QString s;

   if (tokens.count() == 1) {
      conv.SetInFormat("gzmat");
      // create dummy z-matrix input
      s = "#\n\nzmat\n\n";
      s += lines.join("\n");
   }else {
      conv.SetInFormat("xyz");
      // create dummy xyz input
      lines.pop_front();  // remove the charge/multiplicity line
      int nAtoms = lines.count();
      s = QString::number(nAtoms);
      s += "\n\n" + lines.join("\n");
   }

   OBMol mol;
   std::istringstream iss(std::string(s.toLatin1()));
   conv.Read(&mol, &iss);

   mol.SetTotalCharge(charge);
   mol.SetTotalSpinMultiplicity(multiplicity);

   OpenBabel obParser;
   return obParser.extractData(mol);
}



DataList QChem::RemSection::parse(QTextStream&)
{
   // NYI  where should this go? we don't really want it visible
   // until the user fires up the QUI
   return DataList();
}


} } // end namespace IQmol::Parser
