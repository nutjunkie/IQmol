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

#include "QChemInputParser.h"
#include "CartesianCoordinatesParser.h"
#include "ZMatrixCoordinatesParser.h"
#include "ExternalChargesParser.h"
#include "EfpFragmentParser.h"
#include "RemSectionData.h"
#include "EfpFragment.h"
#include "TextStream.h"
#include "PointCharge.h"

#include <QtDebug>


namespace IQmol {
namespace Parser {

bool QChemInput::parse(TextStream& textStream)
{
   QString line;
   m_geometryList = new Data::GeometryList;

   while (!textStream.atEnd()) {
      line = textStream.nextLine();

      if (line.contains("@@@", Qt::CaseInsensitive)) {
         // Any need to demarkate jobs?

      }else if (line.contains("$rem", Qt::CaseInsensitive)) {
         readRemSection(textStream);

      }else if (line.contains("$molecule", Qt::CaseInsensitive)) {
         readMoleculeSection(textStream);

      }else if (line.contains("$efp_params", Qt::CaseInsensitive)) {
         //readEfpParamsSection(textStream);

      }else if (line.contains("$efp_fragments", Qt::CaseInsensitive)) {
         readEfpFragmentSection(textStream);

      }else if (line.contains("$external_charges", Qt::CaseInsensitive)) {
         readExternalChargesSection(textStream);

      }else if (line.contains("------------------------")) {
          // Assume this is the end of the input echoed in an output file.
          break;
      }
   }

   if (m_geometryList->isEmpty()) {
      delete m_geometryList;
   }else {
      m_dataBank.append(m_geometryList);
   }

   return m_errors.isEmpty();
}


void QChemInput::readMoleculeSection(TextStream& textStream)
{
   QString msg("Invalid $molecule format on line ");
   int charge(0);
   unsigned multiplicity(1);
   bool ok;

   // First line can only contain the charge and multiplicity or 'read'
   QString line(textStream.nextLine());
   line = line.replace(',', ' ');
   QStringList tokens(TextStream::tokenize(line));

   switch (tokens.size()) {
      case 0:  // Error
         m_errors.append(msg + QString::number(textStream.lineNumber()));
         return;
      break;

      case 1:  // Could be reading in previous geometry
         if (tokens[0].contains("read", Qt::CaseInsensitive)) {
            if (m_geometryList->size() > 0) {
                // copy previous geometry
                Data::Geometry* geometry(new Data::Geometry(*(m_geometryList->last()))); 
                m_geometryList->append(geometry);
             }else {
				// We assume we are reading an input section from
				// an output file, so there is no new geometry.
             }
          }else {
             m_errors.append(msg + QString::number(textStream.lineNumber()));
          }
          return;
      break;

      default:
         charge = tokens[0].toInt(&ok);
         if (ok) multiplicity = tokens[1].toUInt(&ok);
         if (!ok) {
            m_errors.append(msg + QString::number(textStream.lineNumber()));
            return;
         }
      break;
   }

   // Second line could be a 'read' token or the first atom
   line = textStream.nextLine();
   // offset is passed to the CartesianCoordinates parser to give
   // an accurate line number if an error occurs.
   int offset(textStream.lineNumber()-1);

   if (line.isEmpty()) {
      m_errors.append(msg + QString::number(textStream.lineNumber()));
      return;
   }

   // Special case: read previous geometry
   if (line.contains("read", Qt::CaseInsensitive)) {
      if (m_geometryList->size() > 0) {
         // copy last geometry
         Data::Geometry* geometry(new Data::Geometry(*(m_geometryList->last()))); 
         geometry->setChargeAndMultiplicity(charge, multiplicity);
         m_geometryList->append(geometry);
      }else {
         m_errors.append(msg + QString::number(textStream.lineNumber()));
      }
      return;
   }

   // Special case: EFP only fragments
   if (line.contains("--")) {
	  // Check for an existing list which may have been created if the
	  // $efp_fragments section was parsed before $molecule.
      Data::EfpFragmentList* efps(0);
      QList<Data::EfpFragmentList*> lists = m_dataBank.findData<Data::EfpFragmentList>();
      if (lists.isEmpty()) {
         efps = new Data::EfpFragmentList;
         m_dataBank.append(efps);
      }else {
         efps = lists.last();
      }

      int count(1);
      while (!textStream.atEnd()) {
         tokens = textStream.nextLineAsTokens();

         if (tokens.size() == 1 && tokens[0].contains("$end", Qt::CaseInsensitive)) {
            break;
         }else if (tokens.size() == 6) {
            bool allOk(true);
            double x, y, z, a, b, c;
            if (allOk) x = tokens[0].toDouble(&ok);  allOk = allOk && ok;
            if (allOk) y = tokens[1].toDouble(&ok);  allOk = allOk && ok;
            if (allOk) z = tokens[2].toDouble(&ok);  allOk = allOk && ok;
            if (allOk) a = tokens[3].toDouble(&ok);  allOk = allOk && ok;
            if (allOk) b = tokens[4].toDouble(&ok);  allOk = allOk && ok;
            if (allOk) c = tokens[5].toDouble(&ok);  allOk = allOk && ok;

            if (allOk) {
               Data::EfpFragment* efp(0);
               if (count <= efps->size()) {
                  efp = efps->at(count-1);
               }else {
                  efp = new Data::EfpFragment;
                  efps->append(efp);
               }
               efp->setPosition(Vec(x,y,z));
               efp->setEulerAngles(a,b,c);
            }
            ++count;
         }
      }
      return;
   }

   tokens = TextStream::tokenize(line);
   bool zmat(tokens.size() == 1);

   QStringList lines;
   while (!textStream.atEnd() && !line.contains("$end", Qt::CaseInsensitive)) {
      // Account for the possibility of a FSM job, which has two geometries
      // separated by the **** token.
      if (line.contains("****")) {
         parseGeometry(lines, offset, zmat);
         offset = textStream.lineNumber();
         lines.clear();
      }else {
         lines.append(line); 
      }
      line = textStream.nextLine();
   }
   
   parseGeometry(lines, offset, zmat);
}


void QChemInput::parseGeometry(QStringList const& lines, int offset, bool zmat)
{
   Data::Geometry* geom(0);
   QString line(lines.join("\n"));

   if (zmat) {
     ZMatrixCoordinates parser;
     geom = parser.parse(line);
     if (parser.error().isEmpty()) {
        m_geometryList->append(geom);
      }else {
        m_errors.append(parser.error());
      }
   }else {
     TextStream molecule(&line);
     molecule.setOffset(offset);
     CartesianCoordinates parser(lines.size());
     geom = parser.parse(molecule);
     if (parser.error().isEmpty()) {
        m_geometryList->append(geom);
      }else {
        m_errors.append(parser.error());
      }
   }
}

 
void QChemInput::readRemSection(TextStream& textStream)
{  
   QString line;
   QStringList tokens;
   bool finished(false);
   Data::RemSection* rem = new Data::RemSection;

   while (!textStream.atEnd() && !finished) {
      line = textStream.nextLine();
      line = line.replace('=', ' ');
      tokens = TextStream::tokenize(line);
      
      switch (tokens.size()) {
         case 0:
            // Ignore empty lines
            break;
         case 1:
            if (tokens[0].contains("$end", Qt::CaseInsensitive)) {
               finished = true;
            }else if (tokens[0].contains("$rem", Qt::CaseInsensitive)) {
               // carry on
            }else {
               m_errors.append("Invalid $rem format, line " + 
                 QString::number(textStream.lineNumber()));
               delete rem;
               rem = 0;
               finished = true;
            }  
            break;
         default:
            rem->insert(tokens[0].toLower(), tokens[1].toLower());
            break;
      }     
   }  
   
   if (rem) m_dataBank.append(rem);
} 


void QChemInput::readEfpFragmentSection(TextStream& textStream)
{
   // Check for an existing list from the $molecule section
   Data::EfpFragmentList* efps(0);
   QList<Data::EfpFragmentList*> lists = m_dataBank.findData<Data::EfpFragmentList>();
   if (lists.isEmpty()) {
      efps = new Data::EfpFragmentList;
      m_dataBank.append(efps);
   }else {
      efps = lists.last();
   }

   QString msg("Invalid $efp_fragments format on line ");
   QStringList tokens;
   bool ok, allOk;
   int count(1);

   while (!textStream.atEnd()) {
      tokens = textStream.nextLineAsTokens();

      if (tokens.size() == 1 && tokens[0].contains("$end", Qt::CaseInsensitive)) {
         break;

      }else if (tokens.size() < 7) {
         Data::EfpFragment* efp(0);
         if (count <= efps->size()) {
            efp = efps->at(count-1);
         }else {
            efp = new Data::EfpFragment;
            efps->append(efp);
         }
         efp->setName(tokens[0]);
 
      }else {
         allOk = true;
         QString name(tokens[0]);
         double x, y, z, a, b, c;
         if (allOk) x = tokens[1].toDouble(&ok);  allOk = allOk && ok;
         if (allOk) y = tokens[2].toDouble(&ok);  allOk = allOk && ok;
         if (allOk) z = tokens[3].toDouble(&ok);  allOk = allOk && ok;
         if (allOk) a = tokens[4].toDouble(&ok);  allOk = allOk && ok;
         if (allOk) b = tokens[5].toDouble(&ok);  allOk = allOk && ok;
         if (allOk) c = tokens[6].toDouble(&ok);  allOk = allOk && ok;

         if (allOk) {
            efps->append(new Data::EfpFragment(name, qglviewer::Vec(x,y,z), a, b, c));
         }else {
            m_errors.append(msg += QString::number(textStream.lineNumber()));
         }
      }
      ++count;
   }
}


void QChemInput::readEfpParamsSection(TextStream& textStream)
{
   EfpFragment parser;
   while (!textStream.atEnd()) {
      parser.loadNextFragment(textStream);
      if (textStream.previousLine().contains("$end", Qt::CaseInsensitive)) break;
   }
}


void QChemInput::readExternalChargesSection(TextStream& textStream)
{
   ExternalCharges parser;

   if (parser.parse(textStream)) {
      Data::Bank& bank(parser.data());
      if (!bank.isEmpty()) {
         Data::Base* base(bank.takeFirst());
         Data::PointChargeList* charges(dynamic_cast<Data::PointChargeList*>(base));
         if (charges) m_dataBank.append(charges);
      }
   }else {
      m_errors << parser.errors();
   }
}
   

} } // end namespace IQmol::Parser
