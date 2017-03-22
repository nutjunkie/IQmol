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

#include "QueueResources.h"
#include "QsLog.h"
#include "TextStream.h"


namespace IQmol {
namespace Process {

QVariant QueueResources::toQVariant() const
{
   QVariantList list;
   list << m_name
        << m_maxWallTime
        << m_defaultWallTime

        << m_maxMemory
        << m_minMemory
        << m_defaultMemory

        << m_maxScratch
        << m_minScratch
        << m_defaultScratch

        << m_maxCpus
        << m_minCpus
        << m_defaultCpus;

   return QVariant(list);
}


bool QueueResources::fromQVariant(QVariant const& qvar)
{
   QVariantList list(qvar.toList());

   if (list.size() != 12) {
      QLOG_WARN() << "Invalid resource list passed to QueueResources::fromQVariant()";
      return false;
   }

   bool allOk(true), ok(true);

   m_name            = list.at( 0).toString();
   m_maxWallTime     = list.at( 1).toString();
   m_defaultWallTime = list.at( 2).toString();
   m_maxMemory       = list.at( 3).toInt(&ok);     allOk = allOk && ok;
   m_minMemory       = list.at( 4).toInt(&ok);     allOk = allOk && ok;
   m_defaultMemory   = list.at( 5).toInt(&ok);     allOk = allOk && ok;
   m_maxScratch      = list.at( 6).toInt(&ok);     allOk = allOk && ok;
   m_minScratch      = list.at( 7).toInt(&ok);     allOk = allOk && ok;
   m_defaultScratch  = list.at( 8).toInt(&ok);     allOk = allOk && ok;
   m_maxCpus         = list.at( 9).toInt(&ok);     allOk = allOk && ok;
   m_minCpus         = list.at(10).toInt(&ok);     allOk = allOk && ok;
   m_defaultCpus     = list.at(11).toInt(&ok);     allOk = allOk && ok;

   return allOk;
}


} } // end namespace IQmol::Process

