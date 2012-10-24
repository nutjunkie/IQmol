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

#include "PBSQueue.h"
#include <QStringList>
#include <QtDebug>


namespace IQmol {

QueueList PBSQueue::fromQstat(QString const& qstat)
{
   QueueList queues;
   PBSQueue* queue(0);
   QStringList lines(qstat.split(QRegExp("\\n"), QString::SkipEmptyParts));
   QStringList tokens;
   QString line;
   bool ok;
   int n;

   QStringList::iterator iter;
   for (iter = lines.begin(); iter != lines.end(); ++iter) {
       line = *iter; 
       tokens = line.split(" ", QString::SkipEmptyParts);

       if (line.contains("Queue: ")) {
          queue = new PBSQueue(tokens[1]);
          queues.append(queue);
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
           queue->m_maxJobfs= parseResource(tokens[2]);
       }else if (line.contains("resources_min.jobfs")) {
           queue->m_minJobfs= parseResource(tokens[2]);
       }else if (line.contains("resources_default.jobfs")) {
           queue->m_defaultJobfs= parseResource(tokens[2]);
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

   return queues;
}



int PBSQueue::parseResource(QString& r)
{
   int  Mb(0);
   bool ok(false);

   if (r.contains("gb", Qt::CaseInsensitive)) {
      Mb = r.remove("gb").toInt(&ok);
      Mb *= 1024;
   }else if (r.contains("mb", Qt::CaseInsensitive)) {
      Mb = r.remove("mb").toInt(&ok);
   }

   if (!ok) qDebug() << "PBSQueue::parseResource failed to parse:" << r;

   return Mb;
}

} // end namespace IQmol

