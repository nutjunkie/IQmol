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
#include "QsLog.h"
#include "XyzParser.h"
#include "CubeParser.h"
#include "IQmolParser.h"
#include "MeshParser.h"
#include "QChemInputParser.h"
#include "QChemOutputParser.h"
#include "QChemPlotParser.h"
#include "EfpFragmentParser.h"
#include "ExternalChargesParser.h"
#include "FormattedCheckpointParser.h"
#include "OpenBabelParser.h"

#include <QFileInfo>
#include <QDir>


namespace IQmol {
namespace Parser {


ParseFile::ParseFile(QString const& filePath)
{
   setFilePaths(filePath);
}


void ParseFile::setFilePaths(QString const& filePath)
{
   QFileInfo info(filePath);
   m_filePath = filePath;

   if (info.isDir()) {
      QDir dir(filePath);

      QStringList list;
      list << dir.dirName() + ".*";
      dir.setNameFilters(list);

      QDir::Filters filters(QDir::Files | QDir::Readable);
      m_filePaths << dir.entryList(filters);

      QStringList::iterator iter;
      for (iter = m_filePaths.begin(); iter != m_filePaths.end(); ++iter) {
          (*iter).prepend(m_filePath + "/");
      }
   }else {
      m_filePaths.append(filePath);
   }
}


void ParseFile::run()
{
   Data::FileList* fileList = new Data::FileList();

   QStringList::const_iterator file;
   for (file = m_filePaths.begin(); file != m_filePaths.end(); ++file) {
       if (parse(*file)) fileList->append(new Data::File(*file));
   }

   if (fileList->isEmpty()) {
      delete fileList;
   }else {
      m_dataBank.append(fileList);
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

   if (extension == "run" || extension == "err") {
      return false;
   }

   if (extension == "xyz") {
      QLOG_INFO() << "Using Xyz parser";
      parser = new Xyz;
   }

   if (extension == "efp") {
      parser = new EfpFragment;
   }

   if (extension == "esp" || extension == "mo" || extension == "hf") {
      parser = new QChemPlot;
   }

   if (extension == "in"  || extension == "qcin"  || extension == "inp") {
      QLOG_INFO() << "Using QChemInput parser";
      parser = new QChemInput;
   }

   if (extension == "out"  || extension == "qcout") {
      QLOG_INFO() << "Using QChemOutput parser";
      parser = new QChemOutput;
   }

   if (extension == "iqmol" || extension == "iqm") {
      QLOG_DEBUG() << "Using IQmol parser";
      parser = new IQmol;
   }

   if (extension == "cube" || extension == "cub") {
      parser = new Cube;
   }

   if (extension == "chg") {
      parser = new ExternalCharges;
   }

   if (extension == "fchk" || extension == "fck" || extension == "fch") {
      QLOG_DEBUG() << "Using FormattedCheckpoint parser";
      parser = new FormattedCheckpoint;
   }

   if (extension == "ply" || extension == "obj" || 
       extension == "stl" || extension == "off" ) {
      QLOG_DEBUG() << "Using Mesh parser";
      parser = new Mesh;
   }
   
   if (!parser && OpenBabel::formatSupported(extension)) {
      // Only if we do not have a custom parser do we let Open Babel at it
      QLOG_DEBUG() << "Using OpenBabel parser";
      parser = new OpenBabel;
   }

   if (!parser) {
      QLOG_WARN() << "Failed to find parser for file:" << filePath << extension;
      return false;
   }

   runParser(parser, filePath);
   delete parser;
   return m_errorList.isEmpty();
}


void ParseFile::runParser(Base* parser, QString const& filePath)
{
   if (parser->parseFile(filePath)) {
      QLOG_INFO() << "File parsed successfully: " << filePath;
      Data::Bank& bank(parser->data());
      m_dataBank.merge(bank);
   }else {
      QStringList errors(parser->errors());
      QFileInfo info(filePath);
      QString msg("Error parsing file: ");
      msg += info.fileName();
      m_errorList.append(msg);
      m_errorList << errors;
   }
}

} } // end namespace IQmol::Parser
