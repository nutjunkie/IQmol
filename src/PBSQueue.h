#ifndef IQMOL_PBSQUEUE_H
#define IQMOL_PBSQUEUE_H
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

#include <QString>
#include <QList>


namespace IQmol {

   class PBSConfigurator;

   class PBSQueue {

      // Container class with PBS queue limits.  Note all default values are in Mb
      public:
         friend class PBSConfigurator;

         PBSQueue(QString const& name) : m_name(name),
            m_maxWallTime("24:00:00"), 
            m_defaultWallTime("1:00:00"), 
            m_maxMemory(1000), 
            m_minMemory(1), 
            m_defaultMemory(500), 
            m_maxJobfs(1000), 
            m_minJobfs(1), 
            m_defaultJobfs(500), 
            m_maxCpus(1), 
            m_minCpus(1) { }
 
		 /// Parses the output from the qstat -fQ command and returns a list of
		 /// the currently *enabled* PBS queues.
         static QList<PBSQueue*> fromQstat(QString const&);

      protected:
         QString m_name;
         QString m_maxWallTime;
         QString m_defaultWallTime;

         int m_maxMemory;
         int m_minMemory;
         int m_defaultMemory;

         int m_maxJobfs;
         int m_minJobfs;
         int m_defaultJobfs;

         int m_maxCpus;
         int m_minCpus;
         int m_defaultCpus;

      private:
         // Parses a resource limit and ensures it is in megabytes
         static int parseResource(QString&);
   };

   typedef QList<PBSQueue*> QueueList;

} // end namespace IQmol


#endif
