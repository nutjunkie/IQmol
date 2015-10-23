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

#include "Connection.h"
#include "Reply.h"
#include <QDebug>


namespace IQmol {
namespace Network {

QString Connection::toString(Connection::Status const status)
{
   QString s;
   switch (status) {
      case Closed:         s = "Closed";         break;
      case Opened:         s = "Opened";         break;
      case Authenticated:  s = "Authenticated";  break;
   }
   return s;
}


QString Connection::toString(Connection::AuthenticationT const authentication)
{
   QString s;
   switch (authentication) {
      case None:                 s = "None";                  break;
      case Agent:                s = "Agent";                 break;
      case HostBased:            s = "Host Based";            break;
      case KeyboardInteractive:  s = "Keyboard Interactive";  break;
      case Password:             s = "Password";              break;
      case PublicKey:            s = "Public Key";            break;
   }
   return s;
}

void Connection::thread(Reply* reply)
{
   reply->moveToThread(&m_thread);
   connect(&m_thread, SIGNAL(finished()), reply, SLOT(interrupt()));
   connect(&m_thread, SIGNAL(finished()), reply, SLOT(deleteLater()));
   if (!m_thread.isRunning()) m_thread.start();
}


Connection::~Connection()
{
   killThread();
}


void Connection::killThread()
{
   if (m_thread.isRunning()) {
      //m_thread.quit();
      qDebug() << "Terminating thread for connection" << m_hostname;
      m_thread.terminate();
      qDebug() << "Waiting...";
      m_thread.wait();
      qDebug() << "...thread killed";
   }
}

} } // end namespace IQmol::Network
