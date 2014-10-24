#ifndef IQMOL_NETWORK_REPLY_H
#define IQMOL_NETWORK_REPLY_H
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

#include "QsLog.h"
#include "NetworkException.h"
#include <QObject>


namespace IQmol {
namespace Network {

   class Reply : public QObject {

     Q_OBJECT

      public:
         enum Status { Waiting, Running, Finished, Error, Interrupted, TimedOut };

         Reply() : m_status(Waiting), m_interrupt(false) { }
         virtual ~Reply() { }

         QString message() const { return m_message; }
         Status status() const { return m_status; }
         virtual void start() = 0;

      public Q_SLOTS:
         void interrupt() { m_interrupt = true; }

      Q_SIGNALS:
         void updateAvailable();
         void finished();

      protected:
          Status  m_status;
          QString m_message;
          bool    m_interrupt;
   };

} } // end namespace IQmol::Network

#endif
