#ifndef IQMOL_PROCESS_QUEUERESOURCES_H
#define IQMOL_PROCESS_QUEUERESOURCES_H
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

#include <QString>
#include <QList>
#include <QVariant>


namespace IQmol {
namespace Process {

   class QueueResourcesList;
   class QueueResourcesDialog;

   /// Class containing the details of a queue on a server.  
   /// Note all default values are in Mb
   class QueueResources {

      public:
         friend class QueueResourcesList;
         friend class QueueResourcesDialog;

         // This sets the default values for a new queue
         explicit QueueResources(QString const& name = QString()) : m_name(name),
            m_maxWallTime("72:00:00"), 
            m_defaultWallTime("1:00:00"), 
            m_maxMemory(99999), 
            m_minMemory(100), 
            m_defaultMemory(500), 
            m_maxScratch(99999), 
            m_minScratch(100), 
            m_defaultScratch(500), 
            m_maxCpus(999), 
            m_minCpus(1), 
            m_defaultCpus(1)  { }

         QVariant toQVariant() const;
         bool fromQVariant(QVariant const&);
 
      protected:
         QString m_name;
         QString m_maxWallTime;
         QString m_defaultWallTime;

         int m_maxMemory;
         int m_minMemory;
         int m_defaultMemory;

         int m_maxScratch;
         int m_minScratch;
         int m_defaultScratch;

         int m_maxCpus;
         int m_minCpus;
         int m_defaultCpus;
   };

} } // end namespace IQmol::Process

#endif
