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


// ---------- QueueResourcesList ----------


QueueResourcesList::~QueueResourcesList()
{
   QueueResourcesList::const_iterator iter;
   for (iter = begin(); iter != end(); ++iter) {
       delete (*iter);
   }
}


QVariantList QueueResourcesList::toQVariantList() const
{
   QVariantList list;
   QueueResourcesList::const_iterator iter;
   for (iter = begin(); iter != end(); ++iter) {
       list.append((*iter)->toQVariant());
   }
   return list;
}


void QueueResourcesList::fromQVariantList(QVariantList const& list)
{
   QVariantList::const_iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       QueueResources* queue(new QueueResources());
       if (queue->fromQVariant(*iter)) {
          append(queue);
       } else {
          delete queue;
       }
   }
}


void QueueResourcesList::fromPbsQueueInfoString(QString const& queueInfo)
{
   QueueResources* queue(0);

   QStringList lines(queueInfo.split(QRegExp("\\n"), QString::SkipEmptyParts));
   QStringList tokens;
   QString line;
   bool ok;
   int n;

   QStringList::iterator iter;
   for (iter = lines.begin(); iter != lines.end(); ++iter) {
       line = *iter; 
       tokens = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
   
       if (line.contains("Queue: ")) {
          queue = new QueueResources(tokens[1]);
          append(queue);
       }   

       if (!queue) break;
   
       if (line.contains("resources_max.walltime")) {
           queue->m_maxWallTime = tokens[2];
       }else if (line.contains("resources_default.walltime")) {
           queue->m_defaultWallTime = tokens[2];
       }else if (line.contains("resources_max.vmem")) {
           queue->m_maxMemory = parseResource(tokens[2]);
       }else if (line.contains("resources_min.vmem")) {
           queue->m_minMemory = parseResource(tokens[2]);
       }else if (line.contains("resources_default.vmem")) {
           queue->m_defaultMemory = parseResource(tokens[2]);
       }else if (line.contains("resources_max.jobfs")) {
           queue->m_maxScratch = parseResource(tokens[2]);
       }else if (line.contains("resources_min.jobfs")) {
           queue->m_minScratch = parseResource(tokens[2]);
       }else if (line.contains("resources_default.jobfs")) {
           queue->m_defaultScratch = parseResource(tokens[2]);
       }else if (line.contains("resources_max.ncpus")) {
          n = tokens[2].toInt(&ok);
          if (ok) queue->m_maxCpus = n;
       }else if (line.contains("resources_min.ncpus")) {
          n = tokens[2].toInt(&ok); 
          if (ok) queue->m_minCpus = n;
       }else if (line.contains("resources_default.ncpus")) {
          n = tokens[2].toInt(&ok);
          if (ok) queue->m_defaultCpus = n;
       }   
   }       
}


// I can't figure out how to get much information from the SGE qstat, so we
// just settle on a list of queue names and rely on the default limits given
// in ServerQueue.h
void QueueResourcesList::fromSgeQueueInfoString(QString const& queueInfo)
{  
   QString info(queueInfo);
   QStringList tokens;
   Parser::TextStream textStream(&info);
   
   // The list of queues comes after the header line
   textStream.seek("--------------------");
      
   while (!textStream.atEnd()) {
      tokens = textStream.nextLineAsTokens();
      if (tokens.size() > 1) append(new QueueResources(tokens.first()));
   }     
}


void QueueResourcesList::copy(QueueResourcesList const& list) 
{ 
   QueueResourcesList::const_iterator iter;
   for (iter = list.begin(); iter != list.end(); ++iter) {
       append(new QueueResources(*(*iter)));
   }
}


int QueueResourcesList::parseResource(QString& r)
{
   int  Mb(0);
   bool ok(false);

   if (r.contains("gb", Qt::CaseInsensitive)) {
      Mb = r.remove("gb").toInt(&ok);
      Mb *= 1024;
   }else if (r.contains("mb", Qt::CaseInsensitive)) {
      Mb = r.remove("mb").toInt(&ok);
   }

   if (!ok) { QLOG_DEBUG() << "Failed to parse:" << r; }

   return Mb;
}



} } // end namespace IQmol::Process

