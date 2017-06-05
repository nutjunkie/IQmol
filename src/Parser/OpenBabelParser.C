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


#include "OpenBabelParser.h"
#include "TextStream.h"
#include "AtomicProperty.h"
#include "Frequencies.h"
#include "Constants.h"
#include "Geometry.h"
#include "GeometryList.h"
#include "QsLog.h"
#include "GridData.h"
#include "Preferences.h"

#include <QFileInfo>
#include <vector>

#include "openbabel/mol.h"
#include "openbabel/plugin.h"
#include "openbabel/builder.h"
#include "openbabel/format.h"
#include "openbabel/obconversion.h"
#include "openbabel/generic.h"
#include "openbabel/forcefield.h"
#include "openbabel/griddata.h"
#include "openbabel/math/vector3.h"



namespace IQmol {
namespace Parser {

QStringList OpenBabel::s_obFormats = QStringList();
bool OpenBabel::s_formatsLoaded = false;


bool OpenBabel::parseFile(QString const& filePath)
{
   QFileInfo info(filePath);
   if (!formatSupported(info.suffix())) return false;

   m_filePath = filePath;

   ::OpenBabel::OBConversion conv;
   ::OpenBabel::OBFormat* inFormat(conv.FormatFromExt(QFile::encodeName(m_filePath).data()));

   if (!inFormat || !conv.SetInFormat(inFormat)) {
      m_errors.append("Failed to determine format from extension");
      return false;
   }

   QString type(inFormat->TypeID());
   QString id(inFormat->GetID());
   qDebug() << "Type:" << type << "ID:" << id;

   std::ifstream ifs;
   ifs.open(QFile::encodeName(m_filePath).data());
   if (!ifs) {
      m_errors.append("Failed to open file for reading");
      return false;
   }

   ::OpenBabel::OBMol* mol(new ::OpenBabel::OBMol());
   if (conv.Read(mol, &ifs)) {
     parse(*mol);
   }else {
      m_errors.append("File format error");
   }

   delete mol;

   ifs.close();
   return m_errors.isEmpty();
}


bool OpenBabel::parse(TextStream& stream)
{
   // This is a bit hacky as we rely on the file 
   // extension to determine if we can parse or not
   QFileInfo info(m_filePath);
   QString extension(info.suffix());

   if (!formatSupported(extension)) {
      QString msg("File format not supported");
      m_errors.append(msg);
      return false;
   }

   ::OpenBabel::OBConversion conv;
   ::OpenBabel::OBFormat* inFormat = conv.FormatFromExt(QFile::encodeName(m_filePath).data());

   if (!inFormat) {
      m_errors.append("Failed to determine format from extension");
      return false;
   }

   if (!conv.SetInFormat(inFormat)) {
      m_errors.append("Failed to set format");
      return false;
   }

   ::OpenBabel::OBMol mol;

   QByteArray byteArray(stream.readAll().toLatin1());
   std::string s(std::string(byteArray.data()));
   std::istringstream iss(std::string(byteArray.data()));
   if (conv.Read(&mol, &iss)) {
      parse(mol);
   }else {
      m_errors.append("File format error");
   }

   // catch?

   return m_errors.isEmpty();
}


// This might be better returning a Data::Geometry object for convenience
bool OpenBabel::parse(QString const& string, QString const& extension)
{
   if (!formatSupported(extension)) {
      QString msg("File format not supported");
      m_errors.append(msg);
      return false;
   }

   std::string str(string.toStdString());
   std::string ext(extension.toStdString());

   std::stringstream ss(str);
   ::OpenBabel::OBConversion conv(&ss);

   if (!conv.SetInFormat(ext.data())) {
      m_errors.append("Failed to set input format from extension " + extension);
      return false;
   }

   ::OpenBabel::OBMol mol;

   if (conv.Read(&mol)) {
      parse(mol);
   }else {
      m_errors.append("File format error");
   }

   // catch?

   return m_errors.isEmpty();
}


void OpenBabel::buildFrom2D(::OpenBabel::OBMol& obMol)
{
qDebug() << "*** Building 3D coordinates ***";
qDebug() << "***     Build    may fail   ***";
   ::OpenBabel::OBBuilder builder;
   builder.Build(obMol);

   ::OpenBabel::OBAtomTyper atomTyper;  
   atomTyper.AssignImplicitValence(obMol);
   atomTyper.AssignHyb(obMol);

   obMol.AddHydrogens(true, true);
   obMol.BeginModify();
   obMol.EndModify();

   const char* obff(Preferences::DefaultForceField().toLatin1().data());
   ::OpenBabel::OBForceField* 
      forceField(::OpenBabel::OBForceField::FindForceField(obff));
   if (!forceField) {
qDebug() << "Force Field not found";
      return;
   }

   if (!forceField->Setup(obMol)) {
qDebug() << "Force Field failed setup";
   }

   forceField->SetLogFile(&std::cout);
   forceField->SetLogLevel(OBFF_LOGLVL_LOW);

   forceField->SetConformers(obMol);

   double convergence(1e-6f);
   int maxSteps(1000);

   // We pre-optimize with conjugate gradient 
   forceField->ConjugateGradientsInitialize(maxSteps, convergence);
   forceField->ConjugateGradientsTakeNSteps(maxSteps);

   // And finish off with steepest descent
   forceField->SteepestDescentInitialize(maxSteps, convergence);
   forceField->SteepestDescentTakeNSteps(maxSteps);
}


bool OpenBabel::parse(::OpenBabel::OBMol& obMol)
{
   qDebug() << "Parsing OBMol";
   int numberOfConformers(obMol.NumConformers());
   if (numberOfConformers < 1) return false;

   QLOG_INFO() << numberOfConformers << "geometries found";
   Data::GeometryList* geometries(new Data::GeometryList);
   m_dataBank.append(geometries);

   if (!obMol.Has3D()) buildFrom2D(obMol);

   int charge(obMol.GetTotalCharge());
   unsigned multiplicity(obMol.GetTotalSpinMultiplicity());

   for (int conformer = 0; conformer < numberOfConformers; ++conformer) {
       obMol.SetConformer(conformer);
       Data::Geometry* geometry(new Data::Geometry());
       geometries->append(geometry);

       QList<double> charges;
       for (::OpenBabel::OBMolAtomIter obAtom(&obMol); obAtom; ++obAtom) {
           qglviewer::Vec pos(obAtom->x(), obAtom->y(), obAtom->z());
           unsigned Z(obAtom->GetAtomicNum());
           geometry->append(Z, pos);
           charges.append(obAtom->GetPartialCharge());
       }
       geometry->setAtomicProperty<Data::AtomicCharge>(charges);
       geometry->setChargeAndMultiplicity(charge, multiplicity);
   }

   ::OpenBabel::OBGenericData* data;

   // Frequencies
   data = obMol.GetData(::OpenBabel::OBGenericDataType::VibrationData);
   if (data) {
      ::OpenBabel::OBVibrationData* vibrationData = 
         static_cast< ::OpenBabel::OBVibrationData* >(data);
      if (vibrationData) {
         QLOG_DEBUG() << "Vibrational data found";
         appendVibrationData(*vibrationData);
      }
   }

   // Cube data
   data = obMol.GetData(::OpenBabel::OBGenericDataType::GridData);
   if (data) {
      ::OpenBabel::OBGridData* gridData = static_cast< ::OpenBabel::OBGridData* >(data);
      if (gridData) {
         QLOG_DEBUG() << "Cube data found";
         appendGridData(*gridData);
      }
   }

   return m_errors.isEmpty();
}


void OpenBabel::appendVibrationData(::OpenBabel::OBVibrationData const& vibrationData)
{
   Data::Frequencies* freq(new Data::Frequencies());
   m_dataBank.append(freq);

   unsigned nModes(vibrationData.GetNumberOfFrequencies());
   std::vector<double> frequencies(vibrationData.GetFrequencies());
   std::vector<double> intensities(vibrationData.GetIntensities());
   std::vector<double> ramanActivities(vibrationData.GetRamanActivities());

   if (intensities.empty()) intensities = std::vector<double>(nModes, 0.0);
   if (ramanActivities.empty()) ramanActivities = std::vector<double>(nModes, 0.0);

   std::vector< ::OpenBabel::vector3 > eigenvectors;
   std::vector< ::OpenBabel::vector3 >::const_iterator iter;
   Data::VibrationalMode* mode;

   for (unsigned i = 0; i < nModes; ++i) {
       bool irActive(intensities[i] > 0.0);
       bool ramanActive(ramanActivities[i] > 0.0);

       mode = new Data::VibrationalMode(frequencies[i], intensities[i], irActive, ramanActive);
       freq->append(mode);
              
       eigenvectors = vibrationData.GetLx()[i];
       for (iter = eigenvectors.begin(); iter != eigenvectors.end(); ++iter) {
           mode->appendDirectionVector(qglviewer::Vec((*iter).x(), (*iter).y(), (*iter).z()));
       }
   }
}


void OpenBabel::appendGridData(::OpenBabel::OBGridData const& gridData) 
{
   int n[3];
   gridData.GetNumberOfPoints(n[0], n[1], n[2]);
   ::OpenBabel::vector3 v(gridData.GetOriginVector());
   qglviewer::Vec origin(v.x(), v.y(), v.z());

   double thresh(1e-6);
   double e0[3], e1[3], e2[3];
   gridData.GetAxes(e0, e1, e2);
   qglviewer::Vec delta(e0[0], e1[1], e2[2]);
   
   if ( std::abs(e0[1]) > thresh || std::abs(e0[2]) > thresh ||
        std::abs(e1[0]) > thresh || std::abs(e1[2]) > thresh ||
        std::abs(e2[0]) > thresh || std::abs(e2[1]) > thresh ) { 
        m_errors.append("Warning: Non-axial grid found in data file");
   }

   if (std::abs(delta.x-delta.y) > thresh ||
       std::abs(delta.x-delta.z) > thresh || 
       std::abs(delta.y-delta.z) > thresh) {
      m_errors.append("Warning: Non-regular grid found in data file");
   }   

   if (gridData.GetUnit() == ::OpenBabel::OBGridData::BOHR) {
      QLOG_TRACE() << "Scaling grid Bohr -> Angstrom";
      delta  *=  Constants::BohrToAngstrom;
      origin *=  Constants::BohrToAngstrom;
   }else if (gridData.GetUnit() == ::OpenBabel::OBGridData::ANGSTROM) {
      QLOG_TRACE() << "Grid spacings are already in Angstroms";
   }   

   unsigned nx(n[0]), ny(n[1]), nz(n[2]); 
   QLOG_TRACE() << "Grid Size:" << delta.x << nx << ny << nz << "Total points:" <<  nx*ny*nz; 

   try {

      QString text("Cube File Data");
      Data::SurfaceType type(Data::SurfaceType::CubeData);
      Data::GridSize    size(origin, delta, nx, ny, nz);
      Data::GridData*   grid(new Data::GridData(size, type));

      for (unsigned i = 0; i < nx; ++i) {
          for (unsigned j = 0; j < ny; ++j) {
               for (unsigned k = 0; k < nz; ++k) {
                   (*grid)(i,j,k) = gridData.GetValue(i,j,k);
               }   
          }   
      }   

      m_dataBank.append(grid);

   } catch (...) {
      m_errors.append("Invalid grid data in file");
   }
}


bool OpenBabel::formatSupported(QString const& extension)
{
   // Check if the extension list has been initialized.
   if (!s_formatsLoaded) {
      s_formatsLoaded = true;
      std::vector<std::string> formats;
      bool formatsFound(::OpenBabel::OBPlugin::ListAsVector("formats", 0, formats));

      if (!formatsFound) {
         QLOG_WARN() << "No OB file formats found";
         return false;
      }

      QLOG_DEBUG() << "Initializing OB supported format list";
      std::vector<std::string>::iterator iter;
      QString s;
      for (iter = formats.begin(); iter != formats.end(); ++iter) {
          QString fmt(QString::fromStdString(*iter));
          fmt = fmt.split(QRegExp("\\s+"), QString::SkipEmptyParts).first();
          s += fmt + " ";
          s_obFormats.append(fmt.toLower());
      }

      QLOG_DEBUG() << "Formats supported by OB: " << s;
   }

   return s_obFormats.contains(extension);
}

} } // end namespace IQmol::Parser
