/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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
namespace Process2 {


QVariant QChemJobInfo::serialize()
{
   QList<QVariant> list;
   list << QVariant(m_data[BaseName]);
   list << QVariant(m_data[ServerName]);
   list << QVariant(m_data[LocalWorkingDirectory]);
   list << QVariant(m_data[RemoteWorkingDirectory]);
   list << QVariant(m_data[InputString]);
   list << QVariant(m_charge);
   list << QVariant(m_multiplicity);
   list << QVariant(m_localFilesExist);

   return QVariant(list);
}


QChemJobInfo* QChemJobInfo::deserialize(QVariant const& qvariant)
{

   QList<QVariant> list(qvariant.toList());
   bool ok = (list.size() == 8) &&
             list[0].canConvert<QString>() &&
             list[1].canConvert<QString>() &&
             list[2].canConvert<QString>() &&
             list[3].canConvert<QString>() &&
             list[4].canConvert<QString>() &&
             list[5].canConvert<int>()     &&
             list[5].canConvert<int>()     &&
             list[7].canConvert<bool>();

   if (!ok) {
      QLOG_ERROR() << "Failed to deserialize cached QChemJobInfo";
      return 0;
   }

   QChemJobInfo* jobInfo = new QChemJobInfo();

   jobInfo->m_data[BaseName]               = list[0].toString();
   jobInfo->m_data[ServerName]             = list[1].toString();
   jobInfo->m_data[LocalWorkingDirectory]  = list[2].toString();
   jobInfo->m_data[RemoteWorkingDirectory] = list[3].toString();
   jobInfo->m_data[InputString]            = list[4].toString();
   jobInfo->m_charge                       = list[5].toInt(&ok);
   jobInfo->m_multiplicity                 = list[6].toInt(&ok);
   jobInfo->m_localFilesExist              = list[7].toBool();

   return jobInfo;
}


void QChemJobInfo::set(Field const field, QString const& value) 
{
   switch (field) {
      case LocalWorkingDirectory:
      case RemoteWorkingDirectory:
      case BaseName:
      case ServerName:
      case InputString:
      case Coordinates:
      case Constraints:
      case ScanCoordinates:
      case EfpFragments:
      case EfpParameters:
      case Queue:
      case Walltime:
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


QString QChemJobInfo::get(Field const field) const
{ 
   QString value;
            
   switch (field) {
      case InputFileName:
         value = m_data[BaseName] + ".inp";
         break;
      case OutputFileName:
         value = m_data[BaseName] + ".out";
         break;
      case AuxFileName:
         value = m_data[BaseName] + ".FChk";
         break;
      case EspFileName:
         value = m_data[BaseName] + ".esp";
         break;
      case MoFileName:
         value = m_data[BaseName] + ".mo";
         break;
      case DensityFileName:
         value = m_data[BaseName] + ".hf";
         break;
      case RunFileName:
         value = m_data[BaseName] + ".run";
         break;
      case ErrorFileName:
         value = m_data[BaseName] + ".err";
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
   QLOG_DEBUG() << "   BaseName              " << get(QChemJobInfo::BaseName);
   QLOG_DEBUG() << "   InputFileName         " << get(QChemJobInfo::InputFileName);
   QLOG_DEBUG() << "   OutputFileName        " << get(QChemJobInfo::OutputFileName);
   QLOG_DEBUG() << "   AuxFileName           " << get(QChemJobInfo::AuxFileName);
   QLOG_DEBUG() << "   RunFileName           " << get(QChemJobInfo::RunFileName);
   QLOG_DEBUG() << "   ServerNmae            " << get(QChemJobInfo::ServerName);
   QLOG_DEBUG() << "   LocalWorkingDirectory " << get(QChemJobInfo::LocalWorkingDirectory);
   QLOG_DEBUG() << "   RemoteWorkingDirectory" << get(QChemJobInfo::RemoteWorkingDirectory);
}

void QChemJobInfo::copy(QChemJobInfo const& that)
{
   m_data              = that.m_data;
   m_charge            = that.m_charge;
   m_multiplicity      = that.m_multiplicity;
   m_localFilesExist   = that.m_localFilesExist;
   m_promptOnOverwrite = that.m_promptOnOverwrite;
   m_efpOnlyJob        = that.m_efpOnlyJob;
}

} } // end namespace IQmol::Process

