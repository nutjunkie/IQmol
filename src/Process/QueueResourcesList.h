#ifndef IQMOL_PROCESS_QUEUERESOURCESLIST_H
#define IQMOL_PROCESS_QUEUERESOURCESLIST_H
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


namespace IQmol {
namespace Process {

   class QueueResourcesList : public QList<QueueResources*> {
   
      public:
         QueueResourcesList() : QList<QueueResources*>() { }

         QueueResourcesList(QueueResourcesList const& list) 
          : QList<QueueResources*>() { copy(list); }

         explicit QueueResourcesList(QVariant const& qvar)
          : QList<QueueResources*>() { fromQVariantList(qvar.toList()); }

         explicit QueueResourcesList(QVariantList const& qvar)
          : QList<QueueResources*>() { fromQVariantList(qvar); }

         ~QueueResourcesList();

         QVariantList toQVariantList() const;
         void fromQVariantList(QVariantList const&);

         void fromPbsQueueInfoString(QString const&);
         void fromSgeQueueInfoString(QString const&);
         void fromSlurmQueueInfoString(QString const&);

         QueueResourcesList& operator=(QueueResourcesList const& that) {
            if (this != &that) copy(that);  return *this;
         }

      private:
         // Parses a resource limit and ensures it is in megabytes
         int parseResource(QString&);

         void copy(QueueResourcesList const&);
   };


} } // end namespace IQmol::Process

#endif
