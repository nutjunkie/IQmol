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

#include "Connection.h"
#include "Reply.h"
#include <QDebug>
#include <QInputDialog>


namespace IQmol {
namespace Network {


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


QString Connection::getPasswordFromUser(QString const& message)
{
   bool okPushed(true);
   QString password(QInputDialog::getText(0, "IQmol", message, QLineEdit::Password,
       QString(), &okPushed));
   if (!okPushed) password.clear();
   return password;
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
