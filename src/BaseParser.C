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
#include "FormattedCheckpointParser.h"
#include "FileLayer.h"
#include "QMsgBox.h"
#include <QFile>
#include <QTextStream>


namespace IQmol {
namespace Parser {

bool Base::s_displayErrors = false;

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
          // First, check for a QChem input/output file.  For input files 
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

          // Second, check for external charges, which are not recognized by Open
   	      // Babel.  Check the ExternalChargesParser.h file for a description of
          // the external charges format.
          if (dataList.isEmpty()) {
             ExternalCharges chargeParser;
             dataList += chargeParser.parseFile(fileName);
          }

          // Now let Open Babel have a crack.
          OpenBabel obParser;
          dataList += obParser.parseFile(fileName);

	      // OB will pull the geometry from a FChk file, but not the MO 
          // information so we must pass this information explicitly.
          if (fileName.endsWith(".fchk", Qt::CaseInsensitive)) {
             FormattedCheckpoint fchkParser;
             dataList += fchkParser.parseFile(fileName);
          }

          // Finally, if we have found anything we append the file as well.
          if (!dataList.isEmpty()) {
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
