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

#include "ParseFile.h"
#include "Bank.h"
#include "File.h"
#include "XyzParser.h"
#include "IQmolParser.h"
#include "QChemInputParser.h"
#include "QChemOutputParser.h"
#include "QChemPlotParser.h"
#include "EfpFragmentParser2.h"
#include "ExternalChargesParser2.h"
/*
#include "OpenBabelParser.h"
#include "FormattedCheckpointParser.h"
*/

#include "openbabel/plugin.h"
#include <QFileInfo>
#include <vector>

#include <QDebug>

namespace IQmol {
namespace Parser2 {

QStringList ParseFile::s_obFormats = QStringList();

ParseFile::ParseFile(QStringList filePaths) : m_dataBank(0), m_filePaths(filePaths)
{
}


ParseFile::ParseFile(QString filePath) : m_dataBank(0)
{
   m_filePaths << filePath;
}


Data::Bank* ParseFile::takeData()
{
   Data::Bank* tmp(m_dataBank);
   m_dataBank = 0;
   return tmp;
}


void ParseFile::run()
{
   m_dataBank = new Data::Bank;
   Data::FileList* fileList = new Data::FileList();

   QStringList::const_iterator file;
   for (file = m_filePaths.begin(); file != m_filePaths.end(); ++file) {
       if (parse(*file)) fileList->append(new Data::File(*file));
   }

   if (fileList->isEmpty()) {
      delete fileList;
   }else {
      m_dataBank->append(fileList);
   }
}


bool ParseFile::parse(QString const& filePath)
{
   QFileInfo fileInfo(filePath);
   
   if (!fileInfo.exists()) {
      QString msg("File not found: ");
      msg += fileInfo.filePath();
      m_filePaths.removeAll(filePath);
      m_errorList.append(msg);
      return false;
   }

   QString extension(fileInfo.suffix().toLower());
   Base* parser(0);

   if (extension == "xyz") {
      parser = new Xyz;
   }

   if (extension == "efp") {
      parser = new EfpFragment;
   }

   if (extension == "esp" || extension == "mo" || extension == "hf") {
      parser = new QChemPlot;
   }

   if (extension == "in"  || extension == "qcin"  || extension == "inp") {
      parser = new QChemInput;
   }

   if (extension == "out"  || extension == "qcout") {
      parser = new QChemOutput;
   }

   if (extension == "iqmol") {
      parser = new IQmol;
   }

   if (extension == "chg") {
      parser = new ExternalCharges;
   }

/*
   if (extension == "fchk") {
      parser = new FormattedCheckpoint;
   }

   // Only if we do not have a custom parser do we let Open Babel have a craic.
   if (dataBank.isEmpty() && obSupported(extension)) {
      parser = new OpenBabel;
   }
*/

   if (parser) {
      runParser(parser, filePath);
      delete parser;
   }

   return true;
}


void ParseFile::runParser(Base* parser, QString const& filePath)
{
   Data::Bank& bank(parser->parseFile(filePath));
   m_dataBank->merge(&bank);
   QStringList errors(parser->errors());

   if (!errors.isEmpty()) {
      QFileInfo info(filePath);
      QString msg("Error parsing file: ");
      msg += info.fileName();
      m_errorList.append(msg);
      m_errorList.append(errors);
   }
}


bool ParseFile::obSupported(QString const& extension)
{
   // Check out extension list has been initialized.
   if (s_obFormats.isEmpty()) {
      std::vector<std::string> formats;
      if (::OpenBabel::OBPlugin::ListAsVector("formats", 0, formats)) {
          std::vector<std::string>::iterator iter;
          for (iter = formats.begin(); iter != formats.end(); ++iter) {
              QString fmt(QString::fromStdString(*iter));
              fmt = fmt.split(QRegExp("\\s+"), QString::SkipEmptyParts).first();
              s_obFormats.append(fmt.toLower());
          }
      }
   }

   return s_obFormats.contains(extension);
}

} } // end namespace IQmol::Parser
