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

#include "ExternalChargesParser.h"
#include "OpenBabelParser.h"
#include "QChemParser.h"
#include "EFPFragmentParser.h"
#include "EFPFragmentLayer.h"
#include "FormattedCheckpointParser.h"
#include "FileLayer.h"
#include "QMsgBox.h"
#include "openbabel/plugin.h"
#include <QFile>
#include <QTextStream>

#include "QChemOutputParser.h"
#include "QChemInputParser.h"
#include "QChemPlotParser.h"
#include "EfpFragment.h"
#include "EfpFragmentLibrary.h"
#include "MultipoleExpansion.h"
#include "Geometry.h"
#include "Atom.h"

namespace IQmol {
namespace Parser {

bool Base::s_displayErrors = true;

DataList ParseFiles(QStringList const& fileNames)
{
  DataList dataList;
  QStringList::const_iterator file;
  for (file = fileNames.begin(); file != fileNames.end(); ++file) {
      dataList += ParseFile(*file);
  }

  return dataList;
}


DataList ParseFile(QString const& fileName) 
{
   DataList dataList;

   QFileInfo info(fileName);
   if (info.exists()) {

      try {
		  // Check for EFP file.
          if (fileName.endsWith(".efp", Qt::CaseInsensitive)) {
             EFPFragment efpParser;
             dataList += efpParser.parseFile(fileName);
          }

/*
          // Check for a QChem input/output file.  For input files 
          // OB returns an empty molecule, for output files the charge and 
          // multiplicity don't seem to be parsed correctly.
          if (fileName.endsWith(".in",    Qt::CaseInsensitive) ||
              fileName.endsWith(".inp",   Qt::CaseInsensitive) ||
              fileName.endsWith(".qcin",  Qt::CaseInsensitive) ||
              fileName.endsWith(".qcinp", Qt::CaseInsensitive) ||
              fileName.endsWith(".out",   Qt::CaseInsensitive) ||
              fileName.endsWith(".qcout", Qt::CaseInsensitive) ) {
             QChem qchemParser;
             dataList += qchemParser.parseFile(fileName);
          }
*/

 // --------------------------------------------------------------------
 //  Parser2 temp Temporary hack until the Parser2 stuff is implemented
 // --------------------------------------------------------------------

/*
We need to extract any EFPs from the qchem input and output files.  In addition
we need to avoid OpenBabel parsing an EFP input file or fchk file as it will
barf at the efp stuff

Note the new parsers need to add an Layer::Info and 
the repercieve bonds function needs to be re-written so as not to create a new undo action
bonds need to be determined without using 

*/

          if (fileName.endsWith(".in",    Qt::CaseInsensitive) ||
              fileName.endsWith(".inp",   Qt::CaseInsensitive) ||
              fileName.endsWith(".qcin",  Qt::CaseInsensitive) ||
              fileName.endsWith(".qcinp", Qt::CaseInsensitive) ) {

             // need to get molecule and 
             // $efp_fragments
             Parser2::QChemInput parser;
             Data::Bank& bank(parser.parseFile(fileName));

             QStringList errors(parser.errors());
             if (!errors.isEmpty()) {
                QMsgBox::warning(0, "IQmol", errors.join("\n"));
             }
qDebug() << "Data found in Parser::QChemInput";
bank.dump();

             // Atom list
             QList<Data::GeometryList*> geoms = bank.findData<Data::GeometryList>();
             if (!geoms.isEmpty()) {
                Data::GeometryList* geometryList(geoms.first());
                if (!geometryList->isEmpty()) {
                   Data::Geometry* geometry(geometryList->first());
                   Layer::Atoms* atoms = new Layer::Atoms();
                   Layer::Atom*  atom;

                   for (int i = 0; i < geometry->nAtoms(); ++i) {
                       atom = new Layer::Atom(geometry->atomicNumber(i));
                       atom->setPosition(geometry->position(i));
                       atoms->appendLayer(atom);
                   }
                   dataList += atoms;
                }
             }

             // EFP fragments
             QList<Data::EfpFragmentList*> frags = bank.findData<Data::EfpFragmentList>();
             if (!frags.isEmpty()) {
 qDebug() << "Found some EFP fragments in output file";
                Data::EfpFragmentList* fragments(frags.first());
                fragments->dump(); 
                Data::EfpFragmentLibrary& library(Data::EfpFragmentLibrary::instance());
                Layer::EFPFragments* efps = new Layer::EFPFragments();

                Data::EfpFragmentList::const_iterator iter;

                for (iter = fragments->begin(); iter != fragments->end(); ++iter) {
                    QString filePath(library.getFilePath((*iter)->name()));
                    qDebug() << "Loading fragment from" << filePath;
                    Layer::EFPFragment* efp = new Layer::EFPFragment(filePath);
                    efp->setPosition((*iter)->position());
                    efp->setOrientation((*iter)->orientation());
                    //efp->setRotation((*iter)->orientation());
                    efps->appendLayer(efp);
                }   
                dataList.prepend(efps);
             }
          }


          if (fileName.endsWith(".out",   Qt::CaseInsensitive) ||
              fileName.endsWith(".qcout", Qt::CaseInsensitive) ) {

             // run our old parser as frequencies are not dealt with yet.
             QChem qchemParser;
             dataList += qchemParser.parseFile(fileName);

             Parser2::QChemOutput parser;
             Data::Bank& bank(parser.parseFile(fileName));

             QStringList errors(parser.errors());
             if (!errors.isEmpty()) {
                QMsgBox::warning(0, "IQmol", errors.join("\n"));
             }

			 // This should really take the DMA for the reference geometry
			 // only, but for the time being we assume only the first DMA is of
			 // interest.
             QList<Data::MultipoleExpansionList*> dmas = 
                bank.findData<Data::MultipoleExpansionList>();

             if (!dmas.isEmpty()) {
qDebug() << "Found DMA in output file";
                bank.removeAll(dmas.first()); // very important
                Layer::Data* data = new Layer::Data("DMA Data");
                data->setData(dmas.first());
dmas.first()->dump();
                dataList.append(data);
             }

             QList<Data::EfpFragmentList*> frags = bank.findData<Data::EfpFragmentList>();

             if (!frags.isEmpty()) {
                Data::EfpFragmentList* fragments(frags.first());
 qDebug() << "Found EFP fragments in output file";
                fragments->dump(); 

                Data::EfpFragmentLibrary& library(Data::EfpFragmentLibrary::instance());
                Layer::EFPFragments* efps = new Layer::EFPFragments();

                Data::EfpFragmentList::const_iterator iter;

                for (iter = fragments->begin(); iter != fragments->end(); ++iter) {
                    QString filePath(library.getFilePath((*iter)->name()));
                    qDebug() << "Loading fragment from" << filePath;
                    Layer::EFPFragment* efp = new Layer::EFPFragment(filePath);
                    efp->setPosition((*iter)->position());
                    efp->setOrientation((*iter)->orientation());
                    efps->appendLayer(efp);
                }   

                dataList.prepend(efps);
             }
          }


          if (fileName.endsWith(".esp", Qt::CaseInsensitive) ||
              fileName.endsWith(".mo", Qt::CaseInsensitive) ||
              fileName.endsWith(".hf", Qt::CaseInsensitive) ) {
qDebug() << "Parsing QChemPlot file";

             Parser2::QChemPlot parser;
             Data::Bank& bank(parser.parseFile(fileName));

             QStringList errors(parser.errors());
             if (!errors.isEmpty()) {
                QMsgBox::warning(0, "IQmol", errors.join("\n"));
             }

             QList<Data::GridList*> grids = bank.findData<Data::GridList>();
             if (!grids.isEmpty()) {
                Data::GridList* gridList = grids.first();
                bank.removeAll(gridList); // very important
                for (int i = 0; i < gridList->size(); ++i) {
                    Layer::Data* data = new Layer::Data("Grid Data");
                    data->setData((*gridList)[i]);
                    dataList.append(data);
                }
             }
          }
 // --------------------------------------------------------------------
 //  Parser2 temp Temporary hack until the Parser2 stuff is implemented
 // --------------------------------------------------------------------


          // Check for external charges, which are not recognized by Open
   	      // Babel.  Check the ExternalChargesParser.h file for a description of
          // the external charges format.
          if (dataList.isEmpty()) {
             ExternalCharges chargeParser;
             dataList += chargeParser.parseFile(fileName);
          }

		  // Now let Open Babel have a crack.  This is needed not only for the
		  // geometry, but it also detects the bonds.

// Parser2 temp  Avoid reading fchk files with Open Babel as efp jobs cause problems
if (!fileName.endsWith(".fchk", Qt::CaseInsensitive) &&
    !fileName.endsWith(".esp",  Qt::CaseInsensitive) &&
    !fileName.endsWith(".hf",   Qt::CaseInsensitive) &&
    !fileName.endsWith(".mp",   Qt::CaseInsensitive) &&
    !fileName.endsWith(".inp",  Qt::CaseInsensitive)) {
          std::vector<std::string> formats;
          if (::OpenBabel::OBPlugin::ListAsVector("formats", 0, formats)) {
             QFileInfo info(fileName);
             QString ext(info.suffix());
             std::vector<std::string>::iterator iter;
             for (iter = formats.begin(); iter != formats.end(); ++iter) {
                 QString fmt(QString::fromStdString(*iter));
                 fmt = fmt.split(QRegExp("\\s+"), QString::SkipEmptyParts).first();
                 if (ext.contains(fmt, Qt::CaseInsensitive)) {
                    OpenBabel obParser;
QLOG_DEBUG() << "Running OBparser on file " << fileName;
                    dataList += obParser.parseFile(fileName);
                    break;
                 }
             }
          }
}


	      // OB will pull the geometry from a FChk file, but not the MO 
          // information so we must pass this information explicitly.
          if (fileName.endsWith(".fchk", Qt::CaseInsensitive)) {
             FormattedCheckpoint fchkParser;
             dataList += fchkParser.parseFile(fileName);
          }

          // Finally, if we have found anything we append the file as well.
          if (dataList.isEmpty()) {
             throw ExtensionError();
          }else {
             Layer::Files* files(new Layer::Files());
             files->appendLayer(new Layer::File(fileName));
             dataList.append(files);
          }

      } catch (IOError& ioerr) {
         if (Base::s_displayErrors) QMsgBox::warning(0, "IQmol", ioerr.what());
      } catch (std::exception& e) {
         if (Base::s_displayErrors) QMsgBox::warning(0, "IQmol", e.what());
      }


   }

   return dataList;
}


DataList Base::parseFile(QString const& fileName)
{
   QFile file(fileName);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) throw ReadError();
   QTextStream textStream(&file);
   parse(textStream);
   file.close();
   return m_dataList;
}

} } // end namespace IQmol::Parser
