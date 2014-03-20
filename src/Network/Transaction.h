#ifndef IQMOL_NETWORK_TRANSACTION_H
#define IQMOL_NETWORK_TRANSACTION_H
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

#include "Task.h"
#include <QMutexLocker>


namespace IQmol {
namespace Network {

   /// Base class for transfers through a Session.
   class Transaction : public Task {

      Q_OBJECT

      public:
         Transaction(Session* session) : m_session(session) { }

      public Q_SLOTS:
         virtual void start() {
            QMutexLocker locker(m_session->mutex());
            m_thread->start();
         }

      private:
         Session* m_session;
   };


} } // end namespace IQmol::Network

#endif
