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

#include "QChemJobInfo.h"
#include "QsLog.h"


namespace IQmol {
namespace Process {


QVariantList QChemJobInfo::toQVariantList() const
{
   QVariantList list;
   list << QVariant(baseName());
   list << QVariant(serverName());
   list << QVariant(m_data[LocalWorkingDirectory]);
   list << QVariant(m_data[RemoteWorkingDirectory]);
   list << QVariant(m_data[InputString]);
   list << QVariant(m_charge);
   list << QVariant(m_multiplicity);
   list << QVariant(m_localFilesExist);

   return list;
}


bool QChemJobInfo::fromQVariantList(QVariantList const& list)
{
   bool ok = (list.size() == 8)            &&
             list[0].canConvert<QString>() &&
             list[1].canConvert<QString>() &&
             list[2].canConvert<QString>() &&
             list[3].canConvert<QString>() &&
             list[4].canConvert<QString>() &&
             list[5].canConvert<int>()     &&
             list[5].canConvert<int>()     &&
             list[7].canConvert<bool>();

   if (ok) {
      setBaseName(list[0].toString());
      setServerName(list[1].toString());
      m_data.insert(LocalWorkingDirectory, list[2].toString());
      m_data.insert(RemoteWorkingDirectory,list[3].toString());
      m_data.insert(InputString,           list[4].toString());
      m_charge          = list[5].toInt();
      m_multiplicity    = list[6].toInt();
      m_localFilesExist = list[7].toBool();
   }

   return ok;
}


void QChemJobInfo::set(Field const field, QString const& value) 
{
   qDebug() << "Setting QChemJobInfo field" << field << "to" << value;
   switch (field) {
      case LocalWorkingDirectory:
      case RemoteWorkingDirectory:
      case InputString:
      case Coordinates:
      case Constraints:
      case ScanCoordinates:
      case ExternalCharges:
      case EfpFragments:
      case EfpParameters:
         m_data[field] = value;
         break;
      default:
         QLOG_DEBUG() << "Attempt to set invalid field in QChemJobInfo::set()";
      break;
   }
   //updated();
}


void QChemJobInfo::set(Field const field, int const& value) 
{
   switch (field) {
      case Charge:       
         m_charge = value;        
         break;
      case Multiplicity:  
         m_multiplicity = value;  
         break;
      default: 
         m_data[field] = QString::number(value);
         break;
   }            
   //updated();
}


// Obviously this will return rubbish if a non-file field is specified
QString QChemJobInfo::getLocalFilePath(Field const field) const
{
   QString path(get(LocalWorkingDirectory));
   if (!path.endsWith("/")) path += "/";
   path += get(field);
   return path;
}


// Obviously this will return rubbish if a non-file field is specified
QString QChemJobInfo::getRemoteFilePath(Field const field) const
{
   QString path(get(RemoteWorkingDirectory));
   if (!path.endsWith("/")) path += "/";
   path += get(field);
   return path;
}


QString QChemJobInfo::get(Field const field) const
{ 
   QString value(baseName());

   switch (field) {
      case InputFileName:
         value += ".inp";
         break;
      case OutputFileName:
         value += ".out";
         break;
      case AuxFileName:
         value += ".FChk";
         break;
      case EspFileName:
         value += ".esp";
         break;
      case MoFileName:
         value += ".mo";
         break;
      case DensityFileName:
         value += ".hf";
         break;
      case RunFileName:
         value += ".run";
         break;
      case BatchFileName:
         value += ".bat";
         break;
      case ErrorFileName:
         value += ".err";
         break;

      default:
         value = m_data[field];
         break;
   }

   return value;
}


QStringList QChemJobInfo::outputFiles() const
{
   QStringList files;
   files.append(get(LocalWorkingDirectory) + "/" + get(OutputFileName));
   files.append(get(LocalWorkingDirectory) + "/" + get(AuxFileName));
   return files;
}


void QChemJobInfo::dump() const
{
   QLOG_DEBUG() << "QChemJobInfo info:";
   QLOG_DEBUG() << "   BaseName              " << baseName();
   QLOG_DEBUG() << "   InputFileName         " << get(InputFileName);
   QLOG_DEBUG() << "   OutputFileName        " << get(OutputFileName);
   QLOG_DEBUG() << "   AuxFileName           " << get(AuxFileName);
   QLOG_DEBUG() << "   RunFileName           " << get(RunFileName);
   QLOG_DEBUG() << "   ServerName            " << serverName();
   QLOG_DEBUG() << "   LocalWorkingDirectory " << get(LocalWorkingDirectory);
   QLOG_DEBUG() << "   RemoteWorkingDirectory" << get(RemoteWorkingDirectory);
   QLOG_DEBUG() << "   Molecule pointer      " << m_moleculePointer;
   JobInfo::dump();
}


void QChemJobInfo::copy(QChemJobInfo const& that)
{
   JobInfo::copy(that);
   qDebug() << "QChemJobInfo copy constuctor called" << m_data;
   m_data              = that.m_data;
   m_charge            = that.m_charge;
   m_multiplicity      = that.m_multiplicity;
   m_localFilesExist   = that.m_localFilesExist;
   m_promptOnOverwrite = that.m_promptOnOverwrite;
   m_efpOnlyJob        = that.m_efpOnlyJob;
   m_moleculePointer   = that.m_moleculePointer;
}

} } // end namespace IQmol::Process

